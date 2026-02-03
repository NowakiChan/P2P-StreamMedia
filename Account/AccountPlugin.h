#ifndef ACCOUNT_PLUGIN
#define ACCOUNT_PLUGIN
#include"./AccountDB.h"
#include"../Plugins/ResponseCode.h"
#include<jsoncpp/json/json.h>
#include<httplib.h>
#include<jwt-cpp/jwt.h>
#include<cstdio>
#include<fstream>
#define JWT_ISU "ServerJWT"
#define JWT_SIGN "ServerSign"

class AccountPlugin{
private:
    AccountDB db_interface;
    const char* avator_storge_path;
private:
    Json::Value GetToken(std::string);
    int SetPassword(const Json::Value&);
    int UpdateAvator(const std::string,const std::string,const std::string,const std::string);
public:
    AccountPlugin(ThreadPool<Connector>* pool,const std::string& storge_path)
                 : db_interface(AccountDB(pool)) , avator_storge_path(storge_path.c_str())
    {}

    virtual void AddPath(httplib::Server* server){
        server->Post("/user/login",[&](const httplib::Request& req, httplib::Response& res){         
            res.set_content(Login(req.body),JSON_HTML_TYPE);
        });

        server->Post("/user/tokenverify",[&](const httplib::Request& req, httplib::Response& res){        
            res.set_content(VerifyToken(req.body),JSON_HTML_TYPE);
        });

        server->Post("/user/infocheck",[&](const httplib::Request& req, httplib::Response& res){        
            res.set_content(CheckName(req.body),JSON_HTML_TYPE);
        });

        server->Post("/user/register",[&](const httplib::Request& req, httplib::Response& res){        
            res.set_content(Register(req.body),JSON_HTML_TYPE);
        });

        server->Post("/user/pwdmodify",[&](const httplib::Request& req, httplib::Response& res){        
            res.set_content(ModifyPassword(req.body),JSON_HTML_TYPE);
        });

        server->Post("/user/changeinfo",[&](const httplib::Request& req, httplib::Response& res){        
            res.set_content(InfoModify(req.body),JSON_HTML_TYPE);
        });

        server->Post("/user/avator",[&](const httplib::Request& req, httplib::Response& res){        
            UploadAvator(req,res);
        });

        server->Get("/user/avator",[&](const httplib::Request& req, httplib::Response& res){
            FetchAvator(req,res);
        });

        server->Get("/user/info",[&](const httplib::Request& req, httplib::Response& res){   
            res.set_content(SearchInfo(req),JSON_HTML_TYPE);
        });
    }

    std::string CheckName(const std::string req_body){
        Json::Value req = GetJsonFromStr(req_body),result;

        if(req.isMember("name")){
            result["duplicate"] = (db_interface.SelectUserByName(req["name"].asString(),false)["data"] != Json::nullValue);
        }
        else result["duplicate"] = false;

        return result.toStyledString();
    }

    std::string Login(const std::string req_body){
        Json::Value req = GetJsonFromStr(req_body),result;
        
        if(req.isMember("identity") && req.isMember("pwd") && req.isMember("keepAlive")){
            Json::Value db_res = db_interface.SelectUserByPwd(req["identity"].asString(),req["pwd"].asString());
            if(db_res["errorid"].asInt() == 0){
                result["status"] = (db_res["data"] == Json::nullValue) ? VERIFY_ERR : OK;
                result["userid"] = (db_res["data"] == Json::nullValue) ? Json::nullValue : db_res["data"][0]["userid"];
                result["token"] = (req["keepAlive"].asBool()) 
                                  ? GetToken(db_res["data"][0]["userid"].asString()) 
                                  : Json::nullValue;
                
                /* 更新登录时间 */
                std::cout<<db_res["data"].toStyledString()<<"\n";
                if(db_res["data"] != Json::nullValue)
                    db_res = db_interface.UpdateLoginTime(db_res["data"][0]["userid"].asString());
                if(db_res["errorid"].asInt() != 0 || db_res["change_rows"].asInt() == 0){
                    result["status"] = INTERNAL_ERR;
                    result["userid"] = Json::nullValue;
                    result["token"] = Json::nullValue;
                }
                
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

    std::string VerifyToken(const std::string req_body){
        Json::Value res,req = GetJsonFromStr(req_body);
        auto token_verifier = jwt::verify()
                              .allow_algorithm(jwt::algorithm::hs256{JWT_SIGN})
                              .with_issuer(JWT_ISU);
        if(req.isMember("token")){
            auto decoded_token = jwt::decode(req["token"].asString());
            try{
                token_verifier.verify(decoded_token);
                const std::string id = decoded_token.get_payload_claim("userid").as_string();
                
                if(db_interface.SelectUserById(id)["data"] != Json::nullValue){
                    res["status"] = OK;
                    db_interface.UpdateLoginTime(id);
                }
                else res["status"] = VERIFY_ERR;
                // res["status"] = (db_interface.SelectUserById(id)["data"] != Json::nullValue) ?
                //                 OK : VERIFY_ERR;

            }
            catch(jwt::error::token_verification_exception){
                res["status"] = VERIFY_ERR;
            }
        }
        else res["status"] = DATA_FORMAT_ERR;

        return res.toStyledString();
    }

    std::string Register(const std::string req_body){
        Json::Value res,req = GetJsonFromStr(req_body);

        if(req.isMember("username") && req.isMember("pwd")){
            Json::Value db_res = db_interface.AddNewUser(req["username"].asString(),
                                                         req["pwd"].asString());
            res["status"] = (db_res["errorid"].asInt() == 0 && db_res["change_rows"].asInt() > 0)
                            ? OK : INTERNAL_ERR;
        }
        else res["status"] = DATA_FORMAT_ERR;

        return res.toStyledString();
    }

    std::string ModifyPassword(const std::string req_body){
        Json::Value res,req = GetJsonFromStr(req_body);
        if(req.isMember("oldPwd") && req.isMember("newPwd") && req.isMember("userid")){
            switch (SetPassword(req))
            {
            case -1:
                res["status"] = VERIFY_ERR;
                break;
            case 1:
                res["status"] = OK;
                break;
            default:
                res["status"] = INTERNAL_ERR;
                break;
            }
        }
        else res["status"] = DATA_FORMAT_ERR;

        return res.toStyledString();
    }

    std::string InfoModify(const std::string req_body){
        Json::Value res,req = GetJsonFromStr(req_body);
        if(req.isMember("username") && req.isMember("autograph") && req.isMember("userid")){
            Json::Value db_res = db_interface.UpdateInfo(req["userid"].asString(),req["username"].asString(),
                                                         req["autograph"].asString());
            res["status"] = (db_res["errorid"].asInt() == 0 && db_res["change_rows"].asInt() > 0)
                            ? OK : INTERNAL_ERR;
        }
        return res.toStyledString();
    }

    void UploadAvator(const httplib::Request& req,httplib::Response& res){
        Json::Value result,db_res;
        if(!req.is_multipart_form_data()){
            result["status"] = DATA_FORMAT_ERR;
            res.set_content(result.toStyledString(),JSON_HTML_TYPE);
            return;
        }
        // auto file = req.get_file_value("avator_file");
        auto data = req.form.files.begin();
        auto file = data->second;

        auto param = req.get_param_value("userid");
        // 清除文件名里的空格
        std::string filename = file.filename;
        filename.erase(std::remove(filename.begin(),filename.end(),' '),filename.end());
        
        if(param.size() > 0){
            db_res = db_interface.SelectUserById(param);
            if(db_res["data"] != Json::nullValue){
                const std::string file_name = std::string(avator_storge_path) + "/avator_" + 
                                              db_res["data"][0]["userid"].asString() + "_" + filename;
                const std::string old_file_path = (db_res["data"][0]["avator_link"].asString() == "NULL")
                                                  ? "" : db_res["data"][0]["avator_link"].asString();
                result["status"] = (UpdateAvator(db_res["data"][0]["userid"].asString(),file_name,
                                                 file.content,old_file_path) == 1) ? OK : INTERNAL_ERR;
            }
        }
        else result["status"] = DATA_FORMAT_ERR;

        res.set_content(result.toStyledString(),JSON_HTML_TYPE);
    }

    std::string SearchInfo(const httplib::Request& req){
        auto id = req.get_param_value("id"),
             name = req.get_param_value("name");
        Json::Value res,db_res = Json::nullValue;
        if(id.size() > 0){
            db_res = db_interface.SelectUserById(id);
        }
        else if(name.size() > 0){
            db_res = db_interface.SelectUserByName(name);
        }

        
        if(db_res != Json::nullValue && db_res["errorid"].asInt() == 0){
            res["status"] = OK;
            res["multiple"] = (db_res["data"].size() > 1);
            res["user_info"] = (db_res["data"].size() > 1)
                               ? db_res["data"] : db_res["data"][0];
        }
        else{
            res["status"] = (db_res == Json::nullValue) ? DATA_FORMAT_ERR : INTERNAL_ERR;
            res["multiple"] = false;
            res["user_info"] = Json::nullValue;
        }
        return res.toStyledString();
    }

    void FetchAvator(const httplib::Request& req,httplib::Response& res){
        auto userid = req.get_param_value("userid");
        auto SendFileData = [&](const std::string& path,httplib::Response& r){
            std::ifstream fin(path,std::ios::in | std::ios::binary);
            std::string type = path.substr(path.find_last_of('.') + 1);
            if(type == "jpg") type = "jpeg";

            if(fin.is_open()){
                std::ostringstream stream;
                stream << fin.rdbuf();
                fin.close();

                res.set_content(stream.str(),"image/" + type);
            }
            else res.set_content("","text/plain");
        };
        if(userid.size() > 0){
            Json::Value db_res = db_interface.SelectUserById(userid);
            if(db_res["data"] != Json::nullValue){
                SendFileData(db_res["data"][0]["avator_link"].asString(),res);
                return;
            }
        }
    
        res.set_content("","text/plain");
    }
};

Json::Value AccountPlugin::GetToken(std::string user_identity){
    auto token = jwt::create()
                .set_type("JWT")
                .set_issuer(JWT_ISU)
                .set_payload_claim("userid",jwt::claim(user_identity))
                .set_expires_at(std::chrono::system_clock::now() + std::chrono::hours{24 * 15})
                .sign(jwt::algorithm::hs256{JWT_SIGN});
    Json::Value res = token;
    return res;
}

int AccountPlugin::SetPassword(const Json::Value& info){
    if(db_interface.SelectUserByPwd(info["userid"].asString(),info["oldPwd"].asString())["data"] != Json::nullValue){
        Json::Value execute_res = db_interface.UpdateInfo("","","",info["newPwd"].asString(),true);
        return (execute_res["errorid"].asInt() == 0 && execute_res["change_rows"].asInt() > 0);
    }

    return -1;
}

int AccountPlugin::UpdateAvator(const std::string userid,const std::string new_filename,\
                                const std::string content,const std::string old_path = ""){
    auto write_file = [this](const std::string id,const std::string filename,
                             const std::string content)
    {
        Json::Value res = db_interface.UpdateAvatorLink(id,filename);
        if(res["errorid"].asInt() == 0){
            std::ofstream fout(filename,std::ios::out | std::ios::binary);
            if(fout.is_open()){
                fout << content;
                fout.close();
                return 1;
            }
        }
        return 0;
    };

    if(old_path.size() == 0){
        return write_file(userid,new_filename,content);
    }
    else if(std::remove(old_path.c_str()) == 0){
        return write_file(userid,new_filename,content);
    }

    return 0;
}


#endif