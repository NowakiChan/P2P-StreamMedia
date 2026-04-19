#ifndef WEBSOCKET_SERVER
#define WEBSOCKET_SERVER
// #define ASIO_STANDALONE
#include<websocketpp/config/asio_no_tls.hpp>
#include<websocketpp/server.hpp>
#include<unordered_map>
#include"../SQLConnector/ThreadPool.h"
#include"../SQLConnector/RedisConnector.h"

typedef websocketpp::server<websocketpp::config::asio> websocketsvr;
typedef websocketpp::connection_hdl hdl;

class WebSocketSvr{
private:
    websocketsvr svr_core;
    std::mutex conn_lock;
    std::unordered_map<std::string,hdl> conn_list;
    std::map<websocketpp::connection_hdl, std::string,std::owner_less<websocketpp::connection_hdl>> conn_map;

    ThreadPool<RedisConnector>* redis_pool; 
    bool running;
    unsigned int port;

public:
    WebSocketSvr(const unsigned int _port) 
    : port(_port), running(false), redis_pool(nullptr)
    {
        svr_core.set_error_channels(websocketpp::log::elevel::all);
        svr_core.set_access_channels(websocketpp::log::alevel::all ^ websocketpp::log::alevel::frame_payload);

        svr_core.init_asio();
    }

    void Run(){
        if(!redis_pool){
            std::cout<<"WebSocket Server : No active redis connection pool,now exit\n";
            return;
        }

        svr_core.listen(port);
        svr_core.start_accept();

        running = true;
        svr_core.run();
    }

    void Stop(){
        svr_core.stop_listening();
        for(auto& this_conn : conn_list){
            svr_core.close(this_conn.second,websocketpp::close::status::going_away,"shutdown");
        }
        running = false;
        svr_core.stop();
    }

    void SetConnectionPool(ThreadPool<RedisConnector>* pool){
        this->redis_pool = pool;
    }

    ~WebSocketSvr(){
        if(running) Stop();
    }
private:
    void Open(hdl);
    void Close(hdl);
    void Update(const Json::Value,const std::string);
    void Signal(const Json::Value,const std::string);
    void ConnUpdate(const Json::Value,const std::string);

    void MsgHandler(hdl client,websocketpp::server<websocketpp::config::asio>::message_ptr msg){
        Json::Value req = GetJsonFromStr(msg->get_payload());
        if(req.isMember("pid") && req.isMember("req_type") && req.isMember("req_data")){
            const std::string type = req["req_type"].asString();
            const std::string pid = req["pid"].asString();
            if(type == "update") Update(req["req_data"],pid);
            else if(type == "signal") Signal(req["req_data"],pid);
            else if(type == "connection") ConnUpdate(req["req_data"],pid);
        }
        else{
            Json::Value res;
            res["type"] = "handler";
            res["status"] = DATA_FORMAT_ERR;
            svr_core.send(client,res.toStyledString(),websocketpp::frame::opcode::text);
        }

    }

    void SetHandler(){
        svr_core.set_open_handler(
            [this](hdl new_hdl){ Open(new_hdl); }
        );

        svr_core.set_close_handler(
            [this](hdl close_hdl){ Close(close_hdl); }
        );

        svr_core.set_message_handler(
            [this](hdl client,websocketpp::server<websocketpp::config::asio>::message_ptr msg){
                MsgHandler(client,msg);
            }
        );
    }
};

void WebSocketSvr::Open(hdl new_conn_hdl){
    auto conn = svr_core.get_con_from_hdl(new_conn_hdl);
    const std::string pid = conn->get_request().get_header("Device-ID");
    {
        std::unique_lock<std::mutex> lock(conn_lock);
        conn_list.insert({pid,new_conn_hdl});
        conn_map.insert({new_conn_hdl,pid});
    }
    // 将节点数据写入Redis
    this->redis_pool->GetConnection()->HSet(pid.c_str(),"weight","0.5","last_update",std::to_string(std::time(nullptr)).c_str(),
                                            "upload","0","download","0","conn","0","request","0");

    Json::Value res;
    res["type"] = "open";
    res["status"] = OK;
    svr_core.send(new_conn_hdl,res.toStyledString(),websocketpp::frame::opcode::text);
}

void WebSocketSvr::Close(hdl close_hdl){
    const std::string pid = conn_map[close_hdl];
    Json::Value res_list = GetJsonFromStr(redis_pool->GetConnection()->HGet(pid.c_str())["resource_list"].asString());
    for(auto& res : res_list){
        redis_pool->GetConnection()->SRem(res.asString(),pid);
    }
    redis_pool->GetConnection()->Del(pid.c_str());
    conn_list.erase(pid);
    conn_map.erase(close_hdl);
}

void WebSocketSvr::Update(const Json::Value update_args,const std::string client_id){
    Json::Value res;
    res["type"] = "update";

    if(update_args.isMember("upload") && update_args.isMember("download") && 
       update_args.isMember("conn")){
        // 更新节点权重列表
        Json::Value node_data = this->redis_pool->GetConnection()->HGet(client_id.c_str());
        double old_upload = node_data["upload"].asDouble(),
               old_download = node_data["download"].asDouble(),
               old_conn = node_data["conn"].asDouble(),
               new_upload = update_args["upload"].asDouble(),
               new_download = update_args["download"].asDouble(),
               new_conn = update_args["conn"].asDouble();
        const double new_weight = ResourceAlgo::GetInstance().CalculateWeight(new_upload,new_download,new_conn,
                                                                              old_upload,old_download,old_conn);

        this->redis_pool->GetConnection()->HSet(client_id.c_str(),
                                                "weight",std::to_string(new_weight).c_str(),"last_update",std::to_string(std::time(nullptr)).c_str(),
                                                "upload",update_args["upload"].asCString(),"download",update_args["download"].asCString(),
                                                "conn",update_args["conn"].asCString());
        res["status"] = OK;
    }
    else res["status"] = DATA_FORMAT_ERR;

    svr_core.send(conn_list[client_id],res.toStyledString(),websocketpp::frame::opcode::text);
}

void WebSocketSvr::Signal(const Json::Value msg,const std::string from_client){
    Json::Value res,transfer_msg = msg;
    res["type"] = "signal";
    
    if(msg.isMember("to")){
        if(conn_list.find(msg["to"].asString()) != conn_list.end()){
            transfer_msg["from"] = from_client;
            svr_core.send(conn_list[msg["to"].asString()],transfer_msg.toStyledString(),
                          websocketpp::frame::opcode::text);
            res["status"] = OK;
        }
        else res["status"] = NO_SUCH_PEER;
    }
    else res["status"] = DATA_FORMAT_ERR;

    svr_core.send(conn_list[from_client],res.toStyledString(),websocketpp::frame::opcode::text);
}

void WebSocketSvr::ConnUpdate(const Json::Value msg,const std::string client_id){
    Json::Value res;
    res["type"] = "connection";
    if(msg.isMember("type") && msg.isMember("peers") && msg["peers"].isArray()){
        const std::string type = msg["type"].asString();
        for(auto& peer : msg["peers"]){
            Json::Value peer_info = redis_pool->GetConnection()->HGet(peer.asCString());
            int request_number = 0; double weight = 0.0;
            if(type == "fail" || type == "unuse"){
                request_number = peer_info["request"].asInt() - 1;
                weight = std::clamp(peer_info["weight"].asDouble() * ((type == "unuse") ? 1.2 : 1),0.0,1.0);
            }
            else if(type == "connect"){
                request_number = peer_info["request"].asInt() - 1;
                weight = std::clamp(peer_info["weight"].asDouble() * 1.1,0.0,1.0);
            }
            redis_pool->GetConnection()->HSet(peer.asString().c_str(),
                                              "request",std::to_string(request_number).c_str(),
                                              "weight",std::to_string(weight).c_str());
        }
        res["status"] = OK;
    }
    else res["status"] = DATA_FORMAT_ERR;

    svr_core.send(conn_list[client_id],res.toStyledString(),websocketpp::frame::opcode::text);
}
#endif