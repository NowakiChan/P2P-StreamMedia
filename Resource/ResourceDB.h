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

    Json::Value SelectResourceByName(const std::string name,const std::string uid = ""){
        const std::string is_like_sql = (IsDigitStr(uid) && uid.size() > 0)
            ? "((SELECT COUNT(*) FROM like_record lr WHERE lr.rid = mr.rid AND lr.user = " + uid +
              ") > 0) AS is_like "
            : "0 AS is_like ";
        const std::string sql = std::string(
                   "SELECT mr.rid,mr.resource_name AS name,mr.resource_description AS desciption,mr.upload_user AS uid,mu.username,"
                   "mr.upload_time AS upload_time,mr.video_length AS duration,mr.resolution,mr.likes,"
                   "(SELECT COUNT(*) FROM media_comment mc WHERE mc.rid = mr.rid AND mc.reply_to IS NULL) AS comments,") +
               is_like_sql +
               "FROM media_resource mr INNER JOIN media_user mu ON mr.upload_user = mu.userid ",
            condition = "WHERE mr.resource_name LIKE " + GetSqlStr("%" + name + "%") + " AND mr.available != -1";
        const std::string query = sql + condition;
        return mysql_pool->GetConnection()->Query(query.c_str());
    }

    Json::Value SelectResourceByID(const std::string rid,const std::string uid = ""){
        const std::string is_like_sql = (IsDigitStr(uid) && uid.size() > 0)
            ? "((SELECT COUNT(*) FROM like_record lr WHERE lr.rid = mr.rid AND lr.user = " + uid +
              ") > 0) AS is_like "
            : "0 AS is_like ";
        const std::string sql = std::string(
                   "SELECT mr.rid,mr.resource_name AS name,mr.resource_description AS desciption,mr.upload_user AS uid,mu.username,"
                   "mr.upload_time AS upload_time,mr.video_length AS duration,mr.resolution,mr.likes,"
                   "(SELECT COUNT(*) FROM media_comment mc WHERE mc.rid = mr.rid AND mc.reply_to IS NULL) AS comments,") +
               is_like_sql +
               "FROM media_resource mr INNER JOIN media_user mu ON mr.upload_user = mu.userid ",
            condition = "WHERE mr.rid = " + GetSqlStr(rid) + " AND mr.available != -1";
        const std::string query = sql + condition;
        return mysql_pool->GetConnection()->Query(query.c_str());
    }

    Json::Value SelectResource(const std::string uid = ""){
        const std::string is_like_sql = (IsDigitStr(uid) && uid.size() > 0)
            ? "((SELECT COUNT(*) FROM like_record lr WHERE lr.rid = mr.rid AND lr.user = " + uid +
              ") > 0) AS is_like "
            : "0 AS is_like ";
        const std::string sql = std::string(
                   "SELECT mr.rid,mr.resource_name AS name,mr.resource_description AS desciption,mr.upload_user AS uid,mu.username,"
                   "mr.upload_time AS upload_time,mr.video_length AS duration,mr.resolution,mr.likes,"
                   "(SELECT COUNT(*) FROM media_comment mc WHERE mc.rid = mr.rid AND mc.reply_to IS NULL) AS comments,") +
               is_like_sql +
               "FROM media_resource mr INNER JOIN media_user mu ON mr.upload_user = mu.userid ",
            condition = "WHERE mr.available != -1";
        const std::string query = sql + condition;
        return mysql_pool->GetConnection()->Query(query.c_str());
    }

    Json::Value SelectResourceByUploader(const std::string uid){
        const std::string sql =
            "SELECT mr.rid,mr.resource_name AS name,mr.resource_description AS desciption,"
            "mr.upload_time AS upload_time,mr.video_length AS duration,mr.resolution,mr.likes,"
            "(SELECT COUNT(*) FROM media_comment mc WHERE mc.rid = mr.rid AND mc.reply_to IS NULL) AS comments "
            "FROM media_resource mr ";
        const std::string condition = "WHERE mr.upload_user = " + GetSqlStr(uid) + " AND mr.available != -1";
        const std::string query = sql + condition;
        return mysql_pool->GetConnection()->Query(query.c_str());
    }

    void SetRIDCache(const std::string rid,const std::string timestamp){
        redis_pool->GetConnection()->Set(rid.c_str(),timestamp.c_str());
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

    Json::Value GetReSourceLabel(const std::string rid){
        const std::string query = "SELECT label_name,label_type FROM label WHERE labelid IN (SELECT labelid FROM resource_label WHERE rid = " 
                                  + GetSqlStr(rid) + ")";
        return mysql_pool->GetConnection()->Query(query.c_str());
    }

    Json::Value GetAllLabel(){
        const std::string query = "SELECT * FROM label WHERE label_type = 0";
        return mysql_pool->GetConnection()->Query(query.c_str());
    }

    Json::Value SetLabelRecord(const std::vector<std::string> labels,const std::string rid){
        std::string value_str;
        for(size_t i = 0;i < labels.size();i++){
            value_str += "(" + GetSqlStr(rid) + "," + GetSqlStr(labels[i]) + ")";
            if(i != labels.size() - 1)
                value_str += ",";
        }
        const std::string query = "INSERT INTO resource_label(rid,labelid) VALUES " + value_str;
        return mysql_pool->GetConnection()->Execute(query.c_str());
    }

    Json::Value AddSecondClassLabel(const std::vector<std::string> name){
        std::string value_str;
        for(size_t i = 0;i < name.size();i++){
            const std::string labelid = std::to_string(ResourceAlgo::GetInstance().SnowflakeID());
            value_str += "(" + GetSqlStr(labelid) + "," + GetSqlStr(name[i]) + ",1)";
            if(i != name.size() - 1)
                value_str += ",";
        }
        const std::string query = "INSERT INTO label(labelid,label_name,label_type) VALUES " + value_str;
        return mysql_pool->GetConnection()->Execute(query.c_str());
    }
};

#endif
