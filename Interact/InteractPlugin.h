#ifndef INTERACT_PLUGIN
#define INTERACT_PLUGIN
#include"./InteractDB.h"
#include"../Plugins/ResponseCode.h"
#include<httplib.h>

class InteractPlugin{
private:
    InteractDB db_interface;
public:
    InteractPlugin(ThreadPool<Connector>* pool) : db_interface(InteractDB(pool)) {}

    void AddPath(httplib::Server* svr){
        svr->Post("/interact/publish",[&](const httplib::Request& req, httplib::Response& res){
            res.set_content(PublishComment(req.body),JSON_HTML_TYPE);
        });

        svr->Get("/interact/comment",[&](const httplib::Request& req, httplib::Response& res){
            res.set_content(GetComment(req),JSON_HTML_TYPE);
        });

        svr->Get("/interact/like",[&](const httplib::Request& req, httplib::Response& res){
            res.set_content(LikeResource(req),JSON_HTML_TYPE);
        });

        svr->Get("/interact/likecomment",[&](const httplib::Request& req, httplib::Response& res){
            res.set_content(LikeComment(req),JSON_HTML_TYPE);
        });

        svr->Get("/interact/history",[&](const httplib::Request& req, httplib::Response& res){
            res.set_content(GetLikeHistory(req),JSON_HTML_TYPE);
        });
    }

    std::string PublishComment(const std::string req_body){
        Json::Value req = GetJsonFromStr(req_body),res;
        if(req.isMember("rid") && req.isMember("content") && req.isMember("userid")){
            if(!IsDigitStr(req["userid"].asString())){
                res["status"] = DATA_FORMAT_ERR;
                return res.toStyledString();
            }
            Json::Value db_res = db_interface.AddComment(req["rid"].asString(),req["content"].asString(),
                                                         req["userid"].asString(),
                                                         (req.isMember("reply_to")) ? req["reply_to"].asString() : "");
            res["status"] = (db_res["errorid"].asInt() == 0 && db_res["change_rows"].asInt() > 0) ? OK : INTERNAL_ERR;
        }
        else{
            res["status"] = DATA_FORMAT_ERR;
        }

        return res.toStyledString();
    }

    std::string GetComment(const httplib::Request& req){
        Json::Value res;
        const std::string type = req.get_param_value("type");
        const std::string rid = req.get_param_value("rid");
        const std::string cid = req.get_param_value("cid");

        if(type.size() == 0 || rid.size() == 0){
            res["status"] = INVAILD_PARAM;
            res["comment"] = Json::nullValue;
            return res.toStyledString();
        }

        if(type != "1" && type != "2"){
            res["status"] = INVAILD_PARAM;
            res["comment"] = Json::nullValue;
            return res.toStyledString();
        }

        if(type == "2" && cid.size() == 0){
            res["status"] = INVAILD_PARAM;
            res["comment"] = Json::nullValue;
            return res.toStyledString();
        }

        Json::Value db_res = db_interface.SelectComment(rid,std::atoi(type.c_str()),cid);
        if(db_res["errorid"].asInt() == 0){
            res["status"] = OK;
            res["comment"] = (db_res["data"] == Json::nullValue) ? Json::arrayValue : db_res["data"];
            if(type == "2"){
                for(auto& item : res["comment"]){
                    item["replies"] = 0;
                }
            }
        }
        else{
            res["status"] = INTERNAL_ERR;
            res["comment"] = Json::nullValue;
        }
        return res.toStyledString();
    }

    std::string LikeResource(const httplib::Request& req){
        Json::Value res;
        const std::string rid = (req.has_param("rid")) ? req.get_param_value("rid") : "";
        const std::string uid = (req.has_param("uid")) ? req.get_param_value("uid") : "";
        if(rid.size() == 0 || uid.size() == 0 || !IsDigitStr(uid)){
            res["status"] = INVAILD_PARAM;
            return res.toStyledString();
        }

        Json::Value check_res = db_interface.CheckLike(rid,uid);
        if(check_res["errorid"].asInt() != 0){
            res["status"] = INTERNAL_ERR;
            return res.toStyledString();
        }
        if(check_res["data"] != Json::nullValue){
            res["status"] = VERIFY_ERR;
            return res.toStyledString();
        }

        Json::Value like_res = db_interface.AddLikeRecord(rid,uid);
        if(like_res["errorid"].asInt() != 0 || like_res["change_rows"].asInt() <= 0){
            res["status"] = INTERNAL_ERR;
            return res.toStyledString();
        }

        Json::Value update_res = db_interface.AddResourceLike(rid);
        res["status"] = (update_res["errorid"].asInt() == 0 && update_res["change_rows"].asInt() > 0) ? OK : INTERNAL_ERR;
        return res.toStyledString();
    }

    std::string LikeComment(const httplib::Request& req){
        Json::Value res;
        const std::string cid = (req.has_param("cid")) ? req.get_param_value("cid") : "";
        const std::string uid = (req.has_param("uid")) ? req.get_param_value("uid") : "";
        if(cid.size() == 0 || uid.size() == 0 || !IsDigitStr(uid)){
            res["status"] = INVAILD_PARAM;
            return res.toStyledString();
        }

        Json::Value check_res = db_interface.CheckCommentLike(cid,uid);
        if(check_res["errorid"].asInt() != 0){
            res["status"] = INTERNAL_ERR;
            return res.toStyledString();
        }
        if(check_res["data"] != Json::nullValue){
            res["status"] = VERIFY_ERR;
            return res.toStyledString();
        }

        Json::Value like_res = db_interface.AddCommentLikeRecord(cid,uid);
        res["status"] = (like_res["errorid"].asInt() == 0 && like_res["change_rows"].asInt() > 0) ? OK : INTERNAL_ERR;
        return res.toStyledString();
    }

    std::string GetLikeHistory(const httplib::Request& req){
        Json::Value res;
        const std::string uid = (req.has_param("uid")) ? req.get_param_value("uid") : "";
        const std::string type = (req.has_param("type")) ? req.get_param_value("type") : "";
        if(uid.size() == 0 || !IsDigitStr(uid) || (type != "comment" && type != "resource")){
            res["status"] = INVAILD_PARAM;
            res["history"] = Json::nullValue;
            return res.toStyledString();
        }

        Json::Value db_res = (type == "comment") ? db_interface.SelectCommentLikeHistory(uid)
                                                  : db_interface.SelectLikeHistory(uid);
        if(db_res["errorid"].asInt() == 0){
            Json::Value history = Json::arrayValue;
            if(db_res["data"] != Json::nullValue){
                for(auto& it : db_res["data"]){
                    if(type == "comment"){
                        history.append(it["cid"]);
                    }
                    else{
                        history.append(it["rid"]);
                    }
                }
            }
            res["status"] = OK;
            res["history"] = history;
        }
        else{
            res["status"] = INTERNAL_ERR;
            res["history"] = Json::nullValue;
        }
        return res.toStyledString();
    }
};

#endif
