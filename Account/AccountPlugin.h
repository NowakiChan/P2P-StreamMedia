#ifndef ACCOUNT_PLUGIN
#define ACCOUNT_PLUGIN
#include"./AccountDB.h"
#include"../Plugins/ResponseCode.h"
#include<httplib.h>

class AccountPlugin{
private:
    AccountDB db_interface;
    httplib::Server* main_server;
private:
    void AddPath(){
        main_server->Post("/user/login",[&](const httplib::Request& req, httplib::Response& res){
            
            res.set_content(Login(req.body),JSON_HTML_TYPE);
        });
    }
public:
    AccountPlugin(httplib::Server* server){
        db_interface = AccountDB();
        main_server = server;

        AddPath();
    }

    std::string Login(const std::string req_body){
        Json::Value req = GetJsonFromStr(req_body),result;
        
        if(req.isMember("identity") && req.isMember("pwd") && req.isMember("keepAlive")){
            Json::Value db_res = db_interface.SelectUserByPwd(req["identity"].asString(),req["pwd"].asString());
            if(db_res["errorid"].asInt() == 0){
                result["status"] = OK;
                result["userid"] = (db_res["data"] == Json::nullValue) ? Json::nullValue : db_res["data"][0]["userid"];
                result["token"] = Json::nullValue;
            }
            else{
                result["status"] = INTERNAL_ERR;
                result["userid"] = Json::nullValue;
                result["token"] = Json::nullValue;
            }
            return result.toStyledString();
        }

        result["status"] = DATA_FORMAT_ERR;
        result["userid"] = Json::nullValue;
        result["token"] = Json::nullValue;

        return result.toStyledString();
    }
};

#endif