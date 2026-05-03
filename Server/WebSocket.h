#ifndef WEBSOCKET_SERVER
#define WEBSOCKET_SERVER
// #define ASIO_STANDALONE
#include<websocketpp/config/asio_no_tls.hpp>
#include<websocketpp/server.hpp>
#include<unordered_map>
#include"../SQLConnector/ThreadPool.h"
#include"../SQLConnector/RedisConnector.h"


class WebSocketSvr{
private:
    typedef websocketpp::server<websocketpp::config::asio> websocketsvr;
    typedef websocketpp::connection_hdl hdl;
    typedef websocketpp::server<websocketpp::config::asio>::message_ptr msgbody;

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
        SetHandler();
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

    void MsgHandler(hdl client,msgbody msg){
        Json::Value req = GetJsonFromStr(msg->get_payload());
        Json::Value res;
        res["data"] = Json::nullValue;
        if(req.isMember("action") && req.isMember("host_id") && req.isMember("data")){
            const std::string type = req["action"].asString();
            const std::string pid = req["host_id"].asString();
            res["action"] = type;
            res["host_id"] = pid;

            // Validate that host_id in payload matches the bound host on this connection.
            {
                std::unique_lock<std::mutex> lock(conn_lock);
                if(conn_map.find(client) == conn_map.end() || conn_map[client] != pid){
                    res["status"] = VERIFY_ERR;
                    svr_core.send(client,res.toStyledString(),websocketpp::frame::opcode::text);
                    return;
                }
            }

            if(type == "update_metrics") Update(req["data"],pid);
            else if(type == "signal_forward") Signal(req["data"],pid);
            else if(type == "connection_feedback") ConnUpdate(req["data"],pid);
            else{
                res["status"] = DATA_FORMAT_ERR;
                svr_core.send(client,res.toStyledString(),websocketpp::frame::opcode::text);
            }
        }
        else{
            res["action"] = "handle_request";
            res["host_id"] = Json::nullValue;
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
            [this](hdl client,msgbody msg){
                MsgHandler(client,msg);
            }
        );
    }
};

void WebSocketSvr::Open(hdl new_conn_hdl){
    auto conn = svr_core.get_con_from_hdl(new_conn_hdl);
    const std::string pid = conn->get_request().get_header("Device-ID");
    Json::Value res;
    res["data"] = Json::nullValue;

    if(pid.size() == 0){
        res["action"] = "connect";
        res["host_id"] = Json::nullValue;
        res["status"] = DATA_FORMAT_ERR;
        svr_core.send(new_conn_hdl,res.toStyledString(),websocketpp::frame::opcode::text);
        return;
    }

    {
        std::unique_lock<std::mutex> lock(conn_lock);
        conn_list.insert({pid,new_conn_hdl});
        conn_map.insert({new_conn_hdl,pid});
    }
    // 将节点数据写入Redis
    this->redis_pool->GetConnection()->HSet(pid.c_str(),"weight","0.5","last_update",std::to_string(std::time(nullptr)).c_str(),
                                            "upload","0","download","0","conn","0","request","0");

    res["action"] = "connect";
    res["host_id"] = pid;
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
    res["action"] = "update_metrics";
    res["host_id"] = client_id;
    res["data"] = Json::nullValue;

    if(update_args.isMember("upload") && update_args.isMember("download") && 
       update_args.isMember("conn")){
        // 更新节点权重列表
        Json::Value node_data = this->redis_pool->GetConnection()->HGet(client_id.c_str());
        const double old_upload = JsonNumAsDouble(node_data["upload"]),
               old_download = JsonNumAsDouble(node_data["download"]),
               old_conn = JsonNumAsDouble(node_data["conn"]),
               new_upload = JsonNumAsDouble(update_args["upload"]),
               new_download = JsonNumAsDouble(update_args["download"]),
               new_conn = JsonNumAsDouble(update_args["conn"]);
        const double new_weight = ResourceAlgo::GetInstance().CalculateWeight(new_upload,new_download,new_conn,
                                                                              old_upload,old_download,old_conn);

        this->redis_pool->GetConnection()->HSet(client_id.c_str(),
                                                "weight",std::to_string(new_weight).c_str(),"last_update",std::to_string(std::time(nullptr)).c_str(),
                                                "upload",std::to_string(new_upload).c_str(),"download",std::to_string(new_download).c_str(),
                                                "conn",std::to_string(new_conn).c_str());
        res["status"] = OK;
    }
    else res["status"] = DATA_FORMAT_ERR;

    svr_core.send(conn_list[client_id],res.toStyledString(),websocketpp::frame::opcode::text);
}

void WebSocketSvr::Signal(const Json::Value msg,const std::string from_client){
    Json::Value res,reply,transfer_msg = msg;
    res["action"] = "signal_forward";
    res["host_id"] = from_client;
    res["data"] = Json::nullValue;
    
    if(msg.isMember("to")){
        if(conn_list.find(msg["to"].asString()) != conn_list.end()){
            reply["action"] = "signal_received";
            reply["host_id"] = msg["to"];
            transfer_msg["from"] = from_client;
	        transfer_msg["to"] = msg["to"];
            reply["data"] = transfer_msg;
            svr_core.send(conn_list[msg["to"].asString()],reply.toStyledString(),
                          websocketpp::frame::opcode::text);
            res["status"] = OK;
            res["data"]["to"] = msg["to"];
        }
        else res["status"] = NO_SUCH_PEER;
    }
    else res["status"] = DATA_FORMAT_ERR;

    svr_core.send(conn_list[from_client],res.toStyledString(),websocketpp::frame::opcode::text);
}

void WebSocketSvr::ConnUpdate(const Json::Value msg,const std::string client_id){
    Json::Value res;
    res["action"] = "connection_feedback";
    res["host_id"] = client_id;
    res["data"] = Json::nullValue;

    if(msg.isMember("type") && msg.isMember("peers") && msg["peers"].isArray()){
        const std::string type = msg["type"].asString();
        if(type != "fail" && type != "unuse" && type != "connect"){
            res["status"] = INVAILD_PARAM;
            svr_core.send(conn_list[client_id],res.toStyledString(),websocketpp::frame::opcode::text);
            return;
        }
        for(const Json::Value& peer : msg["peers"]){
            const std::string peer_id = peer.asString();
            Json::Value peer_info = redis_pool->GetConnection()->HGet(peer_id.c_str());
            int request_number = 0; double weight = 0.0;
            if(type == "fail" || type == "unuse"){
                request_number = JsonNumAsInt(peer_info["request"]) - 1;
                weight = std::clamp(JsonNumAsDouble(peer_info["weight"]) * ((type == "unuse") ? 1.2 : 1),0.0,1.0);
            }
            else if(type == "connect"){
                request_number = JsonNumAsInt(peer_info["request"]) - 1;
                weight = std::clamp(JsonNumAsDouble(peer_info["weight"]) * 1.1,0.0,1.0);
            }
            redis_pool->GetConnection()->HSet(peer_id.c_str(),
                                              "request",std::to_string(request_number).c_str(),
                                              "weight",std::to_string(weight).c_str());
        }
        res["status"] = OK;
    }
    else res["status"] = DATA_FORMAT_ERR;

    svr_core.send(conn_list[client_id],res.toStyledString(),websocketpp::frame::opcode::text);
}
#endif
