#include"Account/AccountPlugin.h"
#include"SQLConnector/RedisConnector.h"
#include"Server/P2PServer.h"
#include<iostream>
#include<filesystem>

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
        svr.JoinPath(AccountPlugin(svr.GetMysqlPool(),config.user_file_storage_path));
        std::cout<<"Server now start running at port : "<<config.port<<"\n";
        svr.Start();
    }
    catch(const char* e){
        std::cout<<"Error happen while starting the server\n"<<e<<"\n";
        std::cout<<"Server now exit...\n";
    }

    return 0;
}