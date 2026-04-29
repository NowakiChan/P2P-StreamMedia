#ifndef RESOURCE_PLUGIN
#define RESOURCE_PLUGIN
#include"./ResourceDB.h"
#include"../Plugins/ResponseCode.h"
#include<unordered_map>
#include<websocketpp/config/asio_no_tls.hpp>
#include<mutex>
#include<httplib.h>

class ResourcePlugin
{
private:
    ResourceDB db_interface;
private:
    void AttachLabelsToInfoRows(Json::Value& info){
        if(info == Json::nullValue || !info.isArray()){
            return;
        }
        for(auto& row : info){
            if(!row.isObject() || !row.isMember("rid")){
                continue;
            }
            const Json::Value lbl = db_interface.GetReSourceLabel(row["rid"].asString());
            if(lbl["errorid"].asInt() == 0 && lbl["data"] != Json::nullValue){
                row["label"] = lbl["data"];
            }
            else{
                row["label"] = Json::arrayValue;
            }
        }
    }

    void AddPeerWeightAtRequest(const std::string pid){
        Json::Value node = db_interface.GetHashTableCache(pid);
        const int req_number = node["request"].asInt() + 1;
        const double weight = node["weight"].asDouble(),
                     dynamic_weight = 1.0 / (1.0 + 0.2 * (double)req_number);
        db_interface.SetHashTableCache(pid,"weight",std::to_string(weight * dynamic_weight).c_str(),
                                       "request",std::to_string(req_number).c_str());
    }
public:
    ResourcePlugin(ThreadPool<Connector>* mysql_pool,ThreadPool<RedisConnector>* redis_pool) :
    db_interface(ResourceDB(redis_pool,mysql_pool)) {}

    void AddPath(httplib::Server* svr){
        svr->Get("/resource/getid",[&](const httplib::Request& req, httplib::Response& res){         
            res.set_content(GetResourceID(),JSON_HTML_TYPE);
        });

        svr->Get("/resource/info",[&](const httplib::Request& req, httplib::Response& res){         
            res.set_content(GetResourceInfo(req),JSON_HTML_TYPE);
        });

        svr->Get("/resource/peers",[&](const httplib::Request& req, httplib::Response& res){         
            res.set_content(GetAvailablePeers(req),JSON_HTML_TYPE);
        });

        svr->Get("/resource/label",[&](const httplib::Request& req, httplib::Response& res){         
            res.set_content(GetLabel(),JSON_HTML_TYPE);
        });

        svr->Post("/resource/update",[&](const httplib::Request& req, httplib::Response& res){         
            res.set_content(UpdateResourceList(req.body),JSON_HTML_TYPE);
        });

        svr->Post("/resource/upload",[&](const httplib::Request& req, httplib::Response& res){         
            res.set_content(NewResource(req.body),JSON_HTML_TYPE);
        });
    }

    std::string GetResourceID(){
        Json::Value res;
        const uint64_t new_id = ResourceAlgo::GetInstance().SnowflakeID();
        const uint64_t timestamp = new_id >> 32;
        db_interface.SetRIDCache(std::to_string(new_id),std::to_string(timestamp));
        res["status"] = OK;
        res["rid"] = std::to_string(new_id);
        res["svr_time"] = timestamp;

        return res.toStyledString();
    }

    std::string GetLabel(){
        Json::Value res;
        const Json::Value db_res = db_interface.GetAllLabel();
        if(db_res["errorid"].asInt() == 0){
            res["label"] = db_res["data"];
            res["status"] = OK;
        }
        else{
            res["status"] = INTERNAL_ERR;
            res["label"] = Json::nullValue;
        }

        return res.toStyledString();
    }

    std::string NewResource(const std::string data){
        Json::Value resource = GetJsonFromStr(data),res;
        if(resource.isMember("rid") && resource.isMember("name") && resource.isMember("description") &&
           resource.isMember("uid") && resource.isMember("duration") && resource.isMember("resolution") &&
           resource.isMember("time") && resource.isMember("label") && resource["label"].isArray()){
            const std::string pid_time = db_interface.GetRIDCache(resource["rid"].asString());
            if(pid_time.size() > 0 && pid_time == resource["time"].asString()){
                db_interface.AddNewResource(resource["rid"].asString(),resource["name"].asString(),resource["uid"].asString(),
                                            resource["duration"].asString(),resource["resolution"].asString(),resource["description"].asString());
                db_interface.DelCache(resource["rid"].asString()); // 写入mysql后删除redis中的缓存
                // 写入标签
                std::vector<std::string> labels;
                for(int i = 0;i < resource["label"].size();i++){
                    labels.push_back(resource["label"][i].asString());
                }
                db_interface.SetLabelRecord(labels,resource["rid"].asString());
                res["status"] = OK;
            }
            else res["status"] = DATA_NOT_FOUND;
        }
        else res["status"] = DATA_FORMAT_ERR;
        std::cout<<res["status"].asString()<<"\n"; 
        return res.toStyledString();
    }
    
    std::string UpdateResourceList(const std::string data){
        Json::Value resource = GetJsonFromStr(data),res;
        if(resource.isMember("pid") && resource.isMember("resources") && resource["resources"].isArray()){
            if(db_interface.GetHashTableCache(resource["pid"].asString()) != Json::nullValue){
                for(auto& rid: resource["resources"]){
                    if(rid.isString()) 
                        db_interface.SetResourceCache(rid.asString(),resource["pid"].asString());
                }
                Json::FastWriter writer;
                std::string list_str = writer.write(resource["resources"]);
                db_interface.SetHashTableCache(resource["pid"].asString(),"resource_list",list_str.c_str());

                res["status"] = OK;
            }
            else res["status"] = DATA_NOT_FOUND;
        }
        else res["status"] = DATA_FORMAT_ERR;

        return res.toStyledString();
    }

    std::string GetAvailablePeers(const httplib::Request& req){
        Json::Value res;
        auto rid = req.get_param_value("id");
        res["status"] = INTERNAL_ERR;
        if(rid.size() > 0){
            // auto cmp = [](std::pair<std::string,double>& a,std::pair<std::string,double>& b) { return a.second >= b.second; };
            // std::set<std::pair<std::string,double>,decltype(cmp)> nodes(cmp);

            std::map<double,std::string,std::greater<double>> nodes;
            for(auto& node : db_interface.GetResourceCache(rid)){
                Json::Value node_info = db_interface.GetHashTableCache(node);
                const time_t now = std::time(nullptr);
                if(node_info["last_update"].asInt() - now <= 120){
                    nodes.insert({node_info["weight"].asDouble(),node}); // 根据节点权重排序
                }
            }
            Json::Value select_node = Json::arrayValue;
            for(auto& node : nodes){
                if(select_node.size() <= 4){
                    select_node.append(node.second);
                    AddPeerWeightAtRequest(node.second);
                }
            }
            res["status"] = OK;
            res["peers"] = select_node;
        }
        else res["status"] = INVAILD_PARAM;

        return res.toStyledString();
    }

    std::string GetResourceInfo(const httplib::Request& req){
        const std::string type = req.get_param_value("type");
        const std::string patten = req.get_param_value("patten");
        Json::Value res;
        if(type.empty() || (type != "all" && type != "id" && type != "name")){
            res["status"] = INVAILD_PARAM;
            res["info"] = Json::nullValue;
            return res.toStyledString();
        }
        if((type == "id" || type == "name") && patten.empty()){
            res["status"] = INVAILD_PARAM;
            res["info"] = Json::nullValue;
            return res.toStyledString();
        }

        Json::Value db_res;
        if(type == "all"){
            db_res = db_interface.SelectResource();
        }
        else if(type == "id"){
            db_res = db_interface.SelectResourceByID(patten);
        }
        else{
            db_res = db_interface.SelectResourceByName(patten);
        }

        if(db_res["errorid"].asInt() != 0){
            res["status"] = INTERNAL_ERR;
            res["info"] = Json::nullValue;
            return res.toStyledString();
        }

        Json::Value info = (db_res["data"] == Json::nullValue) ? Json::Value(Json::arrayValue) : db_res["data"];
        AttachLabelsToInfoRows(info);
        res["status"] = OK;
        res["info"] = info;
        return res.toStyledString();
    }
};

#endif
