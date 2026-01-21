#ifndef P2P_SERVER
#define P2P_SERVER
#include"../SQLConnector/ThreadPool.h"
#include"../SQLConnector/RedisConnector.h"
#include<httplib.h>
class P2PServer{
private:
    httplib::Server server_core;
    ThreadPool<Connector> mysql_conn_pool;
    ThreadPool<RedisConnector> redis_conn_pool;
    unsigned int port;
    bool running_flag;
public:
    P2PServer(const DBConnectionConfig& mysql_config,const DBConnectionConfig& redis_config,
              const unsigned int running_port = 80)
             : mysql_conn_pool(ThreadPool<Connector>(mysql_config,4,2,5)) ,
               redis_conn_pool(ThreadPool<RedisConnector>(redis_config)) ,
               port(running_port) , running_flag(false)
    {}

    template<typename T>
    void JoinLastPath(T &&plugin){
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