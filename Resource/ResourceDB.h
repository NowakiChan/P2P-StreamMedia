#ifndef RESOURCE_DB
#define RESOURCE_DB
#include"../SQLConnector/ThreadPool.h"
#include"../SQLConnector/RedisConnector.h"

class ResourceDB{
private:
    ThreadPool<RedisConnector>* redis_pool;
    ThreadPool<Connector>* mysql_pool;

public:
    ResourceDB(ThreadPool<RedisConnector>* key_pool,ThreadPool<Connector>* sql_pool) 
    : mysql_pool(sql_pool) , redis_pool(key_pool) {}

    Json::Value AddNewResource(const std::string rid,const std::string name,const std::string uid,
                               const std::string video_length,const std::string resolution,const std::string description = "")
    {
        const std::string sql = "INSERT INTO media_resource(rid,resource_name,upload_user,video_length,resolution,upload_time,resource_description)",
                          value_set = " VALUES(" + GetSqlStr(rid) + "," + GetSqlStr(name) + "," + GetSqlStr(uid) + "," + GetSqlStr(video_length) +
                                      "," + GetSqlStr(resolution) + ",NOW()," + GetSqlStr(description) + ')';
        const std::string query = sql + value_set;
        return mysql_pool->GetConnection()->Execute(query.c_str());
    }

    Json::Value SelectResourceByName(const std::string name){
        const std::string sql = "SELECT rid,resource_name,resource_description,upload_time,upload_user,likes,video_length,resolution,username FROM media_resource,media_user ",
                          condition = " WHERE resource_name LIKE " + GetSqlStr("%" + name + "%") + " AND media_resource.upload_user = media_user.userid AND available != -1";
        return mysql_pool->GetConnection()->Query(sql.c_str());
    }

    Json::Value SelectResourceByID(const std::string rid){
        const std::string sql = "SELECT rid,resource_name,resource_description,upload_time,upload_user,likes,video_length,resolution,username FROM media_resource,media_user ",
                          condition = " WHERE rid = " + GetSqlStr(rid) + " AND media_resource.upload_user = media_user.userid AND available != -1";
        return mysql_pool->GetConnection()->Query(sql.c_str());
    }

    Json::Value SelectResource(){
        const std::string sql = "SELECT rid,resource_name,resource_description,upload_time,upload_user,likes,video_length,resolution,username FROM media_resource,media_user ",
                          condition = " WHERE media_resource.upload_user = media_user.userid AND available != -1";
	const std::string query = sql + condition;
        return mysql_pool->GetConnection()->Query(query.c_str());
    }

    void SetRIDCache(const std::string rid,const std::string timestamp){
	std::cout<<"set "<<rid<<" to "<<timestamp<<"\n";
        redis_pool->GetConnection()->Set(rid.c_str(),timestamp.c_str());
	std::cout<<GetRIDCache(rid)<<"\n";
    }

    std::string GetRIDCache(const std::string rid){
        return redis_pool->GetConnection()->Get(rid.c_str());
    }

    void DelCache(const std::string key){
        redis_pool->GetConnection()->Del(key.c_str());
    }

    void SetResourceCache(const std::string rid,const std::string pid){
        redis_pool->GetConnection()->SAdd(rid,pid);
    }

    std::unordered_set<std::string> GetResourceCache(const std::string rid){
        return redis_pool->GetConnection()->SMembers(rid);
    }

    void SetResourcePointCache(const std::string pid,const std::string brandwith,const std::string connect,std::string capacity){
        redis_pool->GetConnection()->HSet(pid.c_str(),"brandwith",brandwith,"connect",connect,"capacity",capacity);
    }

    void SetSessionCache(const std::string sid,const std::string from,std::string to,std::string status,std::string create_time){
        redis_pool->GetConnection()->HSet(sid.c_str(),"from",from,"to",to,"status",status,"create_time",create_time);
    }

    template<typename... Args>
    void SetHashTableCache(const std::string key,Args&&... args){
        static_assert(sizeof...(Args) % 2 == 0,"Redis hash function needs args appears in pairs");
        redis_pool->GetConnection()->HSet(key.c_str(),args...);
    }

    Json::Value GetHashTableCache(const std::string key){
        return redis_pool->GetConnection()->HGet(key.c_str());
    }

    
};

#endif
