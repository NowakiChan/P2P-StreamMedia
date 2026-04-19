#ifndef REDIS_CONN
#define REDIS_CONN
#include<sw/redis++/redis++.h>
#include<jsoncpp/json/json.h>
#include"Connector.h"
#include"../Plugins/DataPlugins.h"
#include<string>
#include<functional>
#include<memory>
#include<vector>
#include<algorithm>
using namespace sw;

class RedisConnector{
private:
    redis::Redis conn;

public:
    template<typename M,typename W,typename... Args>
    void ZAdd(const char* key,M&& member,W&& weight,Args&&... args){
        conn.zadd(key,member,weight);
        ZAdd(key,args...);
    }

    template<typename M,typename W>
    void ZAdd(const char* key,M&& member,W&& weight){
        conn.zadd(key,member,weight);
    }

    template<typename F,typename V,typename... Args>
    void HSet(const char* key,F&& field,V&& value,Args&&... args){
        conn.hset(key,field,value);
        HSet(key,args...);
    }

    template<typename F,typename V>
    void HSet(const char* key,F&& field,V&& value){
        conn.hset(key,field,value);
    }

    template<typename M,typename... Args>
    void SRem(const std::string key,M&& member,Args&&...args){
        conn.srem(key,member);
        SRem(key,args...);
    }

    template<typename M>
    void SRem(const std::string key,M&& member){
        conn.srem(key,member);
    }
public:
    RedisConnector(const DBConnectionConfig& config)
                  : conn(redis::Redis("tcp://localhost:" + std::to_string(config.port)))
    {}

    std::string Get(const char* key){
        auto res = conn.get(key);
        if(res) return *res;

        return std::string("");
    }

    void Set(const char* key,const char* value){
        conn.set(key,value); // 返回bool型，往后考虑作为操作成功与否的标志
    }

    // template<typename T,typename... Args>
    // void HSet(T&& key,Args&&... args){
    //     conn.hset(key,args...); // 返回1表示成功设立新哈希，0表示覆写了旧哈希
    // }

    Json::Value HGet(const char* key){
        Json::Value res = Json::nullValue;
        std::unordered_map<std::string,std::string> redis_output;
        conn.hgetall(key,std::inserter(redis_output,redis_output.begin()));
        for(auto& it: redis_output){
            res[it.first] = it.second;
        }

        return res;
    }

    // 用于对哈希表内的浮点数进行操作
    void HIncre(const char* key,const char* field,const double increment){
        conn.hincrbyfloat(key,field,increment);
    }


    void ZIncre(const char* key,const char* member,const double increment){
        conn.zincrby(key,increment,member);
    }

    Json::Value ZSort(const char* key,bool ascending = true){
        Json::Value res = Json::arrayValue;
        std::unordered_map<std::string,double> redis_output;
        if(ascending){
            conn.zrange(key,0,-1,std::inserter(redis_output,redis_output.begin()));
        }
        else{
            conn.zrevrange(key,0,-1,std::inserter(redis_output,redis_output.begin()));
        }
 
        // append每个元素值
        for(auto& it : redis_output){
            Json::Value v;
            v["id"] = it.first;
            v["weight"] = it.second;
            res.append(v);
        }
        return res;
    }

    void ZRemove(const char* key,const char* member){
        conn.zrem(key,member);
    }

    bool Del(const char* key){
        return conn.del(key);
    }

    void SAdd(const std::string key,const std::string member){
        conn.sadd(key,member);
    }

    std::unordered_set<std::string> SMembers(const std::string key){
        std::unordered_set<std::string> result;
        conn.smembers(key,std::inserter(result,result.begin()));

        return result;
    }
};




#endif