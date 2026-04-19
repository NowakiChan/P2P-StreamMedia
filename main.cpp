#include"Account/AccountPlugin.h"
#include"Resource/ResourcePlugin.h"
#include"SQLConnector/RedisConnector.h"
#include"Server/P2PServer.h"
#include<iostream>
#include<filesystem>
#include<csignal>

P2PServer* svr_ptr = nullptr; // 全局指针，用于处理Ctrl c

int main(int arg,char* args[]){
    std::string config_path = "";
    if(arg > 1){
        config_path = args[1];
    }
    else{
        std::cout<<"No specific config file,use default\n";
        config_path = std::filesystem::current_path().string() + "/config.json";
    }
    try{
        ServerConfig config(config_path);
        P2PServer svr(config);
        svr_ptr = &svr;
        svr.JoinPath(AccountPlugin(svr.GetMysqlPool(),config.user_file_storage_path),
                     ResourcePlugin(svr.GetMysqlPool(),svr.GetRedisPool()));
        // std::cout<<"Server now start running at port : "<<config.port<<"\n";
        signal(SIGINT,[](int sig){ 
            if(svr_ptr) svr_ptr->Stop();
        });
        svr.Start();
    }
    catch(const char* e){
        std::cout<<"Error happen while starting the server\n"<<e<<"\n";
        std::cout<<"Server now exit...\n";
    }

    return 0;
}