#ifndef P2P_SERVER
#define P2P_SERVER
#include"../SQLConnector/ThreadPool.h"
#include"../SQLConnector/RedisConnector.h"
#include<httplib.h>
class ServerConfig{
private:
    void LoadConfigFromFile(const std::string&);
    void LoadServerConfig(const Json::Value& config){
        if(config.isMember("address")) address = config["address"].asString();
        if(config.isMember("port")) port = config["port"].asUInt();
        if(config.isMember("file_size_limit")) upload_size_limit = config["file_size_limit"].asUInt();
        if(config.isMember("file_path")){
            if(config["file_path"].isMember("user_file"))
                user_file_storage_path = config["file_path"]["user_file"].asString();
            if(config["file_path"].isMember("tmp_file"))
                tmp_file_storage_path = config["file_path"]["tmp_file"].asString();
            if(config["file_path"].isMember("log_file"))
                log_file_storage_path = config["file_path"]["log_file"].asString();
        }
    }
public:
    ServerConfig() : port(80) , address("localhost") , user_file_storage_path(".") 
                   , tmp_file_storage_path(".") , log_file_storage_path(".")
    {}
    ServerConfig(const std::string& file) : port(80) , address("localhost")
                                          , user_file_storage_path(".") 
                                          , tmp_file_storage_path(".") 
                                          , log_file_storage_path(".")
    {
        LoadConfigFromFile(file);
    }

    void LoadConfig(const std::string& file) { LoadConfigFromFile(file); }

    DBConnectionConfig mysql_config;
    DBConnectionConfig redis_config;
    std::string address;
    std::string user_file_storage_path;
    std::string tmp_file_storage_path;
    std::string log_file_storage_path;
    unsigned int port;
    unsigned int upload_size_limit = 10 * 1024 * 1024;
};

void ServerConfig::LoadConfigFromFile(const std::string& file_path){
    Json::Value config;
    Json::Reader parser;
    std::ifstream fin(file_path);

    if(fin.is_open()){
        parser.parse(fin,config,false);
        if(config.isMember("redis")) redis_config = DBConnectionConfig(config["redis"]);
        if(config.isMember("mysql")) mysql_config = DBConnectionConfig(config["mysql"]);
        if(config.isMember("server")) LoadServerConfig(config["server"]);
    }
    else throw "Serverconfig : Unable to open file";
}

class P2PServer{
private:
    httplib::Server server_core;
    ThreadPool<Connector> mysql_conn_pool;
    ThreadPool<RedisConnector> redis_conn_pool;
    unsigned int port;
    bool running_flag;
    ServerConfig svr_config;
public:
    P2PServer(const ServerConfig& config) : svr_config(config) , port(config.port) ,
                                            mysql_conn_pool(ThreadPool<Connector>(config.mysql_config)) ,
                                            redis_conn_pool(ThreadPool<RedisConnector>(config.redis_config))
    {
        server_core.set_payload_max_length(config.upload_size_limit);
    }

    P2PServer(const DBConnectionConfig& mysql_config,const DBConnectionConfig& redis_config,
              const unsigned int running_port = 80)
             : mysql_conn_pool(ThreadPool<Connector>(mysql_config)) ,
               redis_conn_pool(ThreadPool<RedisConnector>(redis_config)) ,
               port(running_port) , running_flag(false)
    {}

    template<typename T>
    void JoinPath(T &&plugin){
        plugin.AddPath(&server_core);
    }

    template<typename T,typename... Args>
    void JoinPath(T &&plugin,Args&&... args){
        plugin.AddPath(&server_core);
        JoinPath(args...);
    }

    void Start(){
        running_flag = true;
        server_core.listen("localhost",port);
    }

    void Stop(){
        running_flag = false;
        server_core.stop();
        mysql_conn_pool.ClosePool();
        redis_conn_pool.ClosePool();
    }

    ThreadPool<Connector>* GetMysqlPool() { return &mysql_conn_pool; }
    ThreadPool<RedisConnector>* GetRedisPool() { return &redis_conn_pool; }

    ~P2PServer(){
        if(running_flag) Stop();
    }

};

#endif