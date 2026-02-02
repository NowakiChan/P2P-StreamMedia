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

    int GetCcommandType(const std::string& comm) const{
        std::cout<<comm<<"\n";
        if(comm == "DEL" || comm == "DUMP" || comm == "EXISTS" ||
           comm == "EXPIRE" || comm == "KEYS" || comm == "PERSIST" ||
           comm == "PTTL" || comm == "TTL" || comm == "RENAME" ||
           comm == "TYPE" || comm == "SET" || comm == "GET" || comm == "GETRANGE" ||
           comm == "GETSET" || comm == "GETBIT" || comm == "SETBIT" ||
           comm == "STRLEN" || comm == "MSET" || comm == "INCR" ||
           comm == "INCRBY" || comm == "DECR" || comm == "DECRBY" ||
           comm == "APPEND")
            return 1;
        else if(comm == "HDEL" || comm == "HEXISTS" || comm == "HGET" ||
                comm == "HGETALL" || comm == "HINCRBY" || comm == "HKEYS" ||
                comm == "HLEN" || comm == "HSET" || comm == "HVALS")
            return 2;
        else if(comm == "BLPOP" || comm == "BRPOP" || comm == "BRPOPLPUSH" ||
                comm == "LINDEX" || comm == "LINSERT" || comm == "LLEN" ||
                comm == "LPOP" || comm == "LPUSH" || comm == "LRANGE" ||
                comm == "LERM" || comm == "LSET" || comm == "LTRIM" ||
                comm == "RPOPLPUSH" || comm == "RPUSH")
            return 3;
        else if(comm == "SADD" || comm == "SCARD" || comm == "SDIFF" ||
                comm == "SINTER" || comm == "SISMEMBER" || comm == "SMEMBERS" ||
                comm == "SMOVE" || comm == "SPOP" || comm == "SRANDMEMBER" ||
                comm == "SREM" || comm == "SUNION" || comm == "SSCAN")
            return 4;
        else if(comm == "ZADD" || comm == "ZCARD" || comm == "ZCOUNT" ||
                comm == "ZINCRBY" || comm == "ZINTERSTORE" || comm == "ZLEXCOUNT" ||
                comm == "ZRANGE" || comm == "ZRANGEBYSCORE" || comm == "ZRANGEBYLEX" ||
                comm == "ZRANK" || comm == "ZREM" || comm == "ZREVRANGE" || comm == "ZREVRANGEBYSCORE" ||
                comm == "ZREVRANK" || comm == "ZSCORE" || comm == "ZUNIONSTORE")
            return 5;
        
        return -1;
    }

    template<typename...Args>
    Json::Value Key(const std::string&,Args&&...);
    template<typename...Args>
    Json::Value Hmap(const std::string&,Args&&...) { return Json::nullValue; }
    template<typename...Args>
    Json::Value List(const std::string&,Args&&...) { return Json::nullValue; }
    template<typename...Args>
    Json::Value Set(const std::string&,Args&&...) { return Json::nullValue; }
    template<typename...Args>
    Json::Value SortedSet(const std::string&,Args&&...) { return Json::nullValue; }
public:
    RedisConnector(const DBConnectionConfig& config)
                  : conn(redis::Redis("tcp://localhost:" + std::to_string(config.port)))
    {}

    template<typename...Args>
    Json::Value Execute(const std::string& pre_comm,Args&&...args){
        std::string comm(pre_comm);
        std::transform(pre_comm.begin(),pre_comm.end(),comm.begin(),::toupper);
        switch(GetCcommandType(comm)){
            case 1:
                return Key(comm,args...);
            case 2:
                return Hmap(comm,args...);
            case 3:
                return List(comm,args...);
            case 4:
                return Set(comm,args...);
            case 5:
                return SortedSet(comm,args...);
            default:
                return Json::nullValue;
        }
    }
};

template<typename...Args>
Json::Value RedisConnector::Key(const std::string& comm,Args&&...args){
    Json::Value res;
    if(comm == "SET")
        conn.command<void>(comm,args...);
    else if(comm == "GET"){
        auto db_value = conn.command<redis::OptionalString>(comm,args...);
        res["result"] = *db_value;
    }

    return res;
}




#endif