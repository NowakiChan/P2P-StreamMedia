#ifndef WEBSOCKET_SERVER
#define WEBSOCKET_SERVER
// #define ASIO_STANDALONE
#include<websocketpp/config/asio_no_tls.hpp>
#include<websocketpp/server.hpp>
#include<unordered_map>
#include<cstring>
#include<exception>
#include<iostream>
#include"../SQLConnector/ThreadPool.h"
#include"../SQLConnector/RedisConnector.h"
#include"../Plugins/ResponseCode.h"

/** Binary relay: first 32 bytes are fixed routing — 16-byte ASCII `from`, 16-byte ASCII `to` (space-padded). */
static constexpr std::size_t RELAY_ROUTE_HEADER = 32;
static constexpr std::size_t RELAY_ID_FIELD = 16;

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
    void BinaryRelayForward(hdl client,msgbody msg);
    static void WriteRelayId16(char* out,const std::string& id);
    static std::string TrimRelayId16(const char* field);
    void SendBinaryRelayNotice(hdl client,const std::string& host_id,int status);
    void ForceDisconnect(hdl c);
    void EraseConnMapsIfHeld(hdl close_hdl);

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
            [this](hdl new_hdl){
                try{
                    Open(new_hdl);
                }
                catch(const std::exception& e){
                    std::cerr<<"[WebSocket] open_handler: "<<e.what()<<'\n';
                    ForceDisconnect(new_hdl);
                }
                catch(...){
                    std::cerr<<"[WebSocket] open_handler: non-std exception\n";
                    ForceDisconnect(new_hdl);
                }
            }
        );

        svr_core.set_close_handler(
            [this](hdl close_hdl){
                try{
                    Close(close_hdl);
                }
                catch(const std::exception& e){
                    std::cerr<<"[WebSocket] close_handler: "<<e.what()<<'\n';
                    EraseConnMapsIfHeld(close_hdl);
                }
                catch(...){
                    std::cerr<<"[WebSocket] close_handler: non-std exception\n";
                    EraseConnMapsIfHeld(close_hdl);
                }
            }
        );

        svr_core.set_message_handler(
            [this](hdl client,msgbody msg){
                try{
                    if(msg->get_opcode() == websocketpp::frame::opcode::binary){
                        BinaryRelayForward(client,msg);
                    }
                    else MsgHandler(client,msg);
                }
                catch(const std::exception& e){
                    std::cerr<<"[WebSocket] message_handler: "<<e.what()<<'\n';
                    ForceDisconnect(client);
                }
                catch(...){
                    std::cerr<<"[WebSocket] message_handler: non-std exception\n";
                    ForceDisconnect(client);
                }
            }
        );
    }
};

void WebSocketSvr::ForceDisconnect(hdl c){
    try{
        svr_core.close(c,websocketpp::close::status::internal_endpoint_error,"server handler error");
    }
    catch(const std::exception& e){
        std::cerr<<"[WebSocket] ForceDisconnect(close): "<<e.what()<<'\n';
    }
    catch(...){
        std::cerr<<"[WebSocket] ForceDisconnect(close): unknown exception\n";
    }
}

void WebSocketSvr::EraseConnMapsIfHeld(hdl close_hdl){
    std::unique_lock<std::mutex> lock(conn_lock);
    auto it = conn_map.find(close_hdl);
    if(it == conn_map.end()) return;
    const std::string pid = it->second;
    conn_list.erase(pid);
    conn_map.erase(it);
}

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
    std::string pid;
    {
        std::unique_lock<std::mutex> lock(conn_lock);
        auto it = conn_map.find(close_hdl);
        if(it == conn_map.end()) return;
        pid = it->second;
        conn_list.erase(pid);
        conn_map.erase(it);
    }
    Json::Value res_list = GetJsonFromStr(redis_pool->GetConnection()->HGet(pid.c_str())["resource_list"].asString());
    for(auto& res : res_list){
        redis_pool->GetConnection()->SRem(res.asString(),pid);
    }
    redis_pool->GetConnection()->Del(pid.c_str());
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

void WebSocketSvr::WriteRelayId16(char* out,const std::string& id){
    const std::size_t n = std::min(id.size(), RELAY_ID_FIELD);
    std::memcpy(out, id.data(), n);
    if(n < RELAY_ID_FIELD)
        std::memset(out + n, ' ', RELAY_ID_FIELD - n);
}

std::string WebSocketSvr::TrimRelayId16(const char* field){
    std::size_t end = RELAY_ID_FIELD;
    while(end > 0 && static_cast<unsigned char>(field[end - 1]) == ' ')
        --end;
    return std::string(field, end);
}

void WebSocketSvr::SendBinaryRelayNotice(hdl client,const std::string& host_id,int status){
    Json::Value res;
    res["action"] = "binary_relay";
    res["host_id"] = host_id.empty() ? Json::Value() : Json::Value(host_id);
    res["data"] = Json::nullValue;
    res["status"] = status;
    svr_core.send(client, res.toStyledString(), websocketpp::frame::opcode::text);
}

void WebSocketSvr::BinaryRelayForward(hdl client,msgbody msg){
    const std::string& pl = msg->get_payload();
    if(pl.size() < RELAY_ROUTE_HEADER){
        std::string hid;
        {
            std::unique_lock<std::mutex> lock(conn_lock);
            auto it = conn_map.find(client);
            if(it != conn_map.end()) hid = it->second;
        }
        SendBinaryRelayNotice(client, hid, DATA_FORMAT_ERR);
        return;
    }

    std::string out = pl;
    hdl target{};
    bool have_target = false;

    {
        std::unique_lock<std::mutex> lock(conn_lock);
        auto from_it = conn_map.find(client);
        if(from_it == conn_map.end()){
            lock.unlock();
            SendBinaryRelayNotice(client, "", VERIFY_ERR);
            return;
        }
        const std::string& from_id = from_it->second;
        const std::string to_id = TrimRelayId16(pl.data() + RELAY_ID_FIELD);
        if(to_id.empty()){
            lock.unlock();
            SendBinaryRelayNotice(client, from_id, DATA_FORMAT_ERR);
            return;
        }
        auto to_it = conn_list.find(to_id);
        if(to_it == conn_list.end()){
            lock.unlock();
            SendBinaryRelayNotice(client, from_id, NO_SUCH_PEER);
            return;
        }
        WriteRelayId16(&out[0], from_id);
        target = to_it->second;
        have_target = true;
    }

    if(have_target)
        svr_core.send(target, out, websocketpp::frame::opcode::binary);
}
#endif
