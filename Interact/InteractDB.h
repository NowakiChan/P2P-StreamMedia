#ifndef INTERACT_DB
#define INTERACT_DB
#include"../SQLConnector/ThreadPool.h"
#include"../Plugins/DataPlugins.h"

class InteractDB{
private:
    ThreadPool<Connector>* conn_pool;
public:
    InteractDB(ThreadPool<Connector>* pool) : conn_pool(pool) {}

    Json::Value AddComment(const std::string rid,const std::string content,
                           const std::string userid,const std::string reply_to = ""){
        const std::string cid = std::to_string(ResourceAlgo::GetInstance().SnowflakeID());
        const bool is_reply = (reply_to.size() > 0);
        const std::string query = "INSERT INTO media_comment(cid,rid,content,user,publish_time,reply_to) VALUES(" +
                                  GetSqlStr(cid) + "," + GetSqlStr(rid) + "," + GetSqlStr(content) + "," +
                                  ((IsDigitStr(userid)) ? userid : GetSqlStr(userid)) + ",NOW()," +
                                  ((is_reply) ? GetSqlStr(reply_to) : "NULL") + ")";
        return conn_pool->GetConnection()->Execute(query.c_str());
    }

    Json::Value SelectComment(const std::string rid,const int type = 1,const std::string cid = "",
                              const std::string uid = ""){
        const std::string is_like_sql = (IsDigitStr(uid) && uid.size() > 0)
            ? "((SELECT COUNT(*) FROM like_comment lc WHERE lc.cid = media_comment.cid AND lc.user = " + uid +
              ") > 0) AS is_like "
            : "0 AS is_like ";
        const std::string sql = std::string(
                   "SELECT media_comment.cid,media_comment.content,media_comment.user AS userid,media_user.username,media_comment.publish_time,"
                   "(SELECT COUNT(*) FROM media_comment AS reply WHERE reply.reply_to = media_comment.cid) AS replies,"
                   "(SELECT COUNT(*) FROM like_comment WHERE like_comment.cid = media_comment.cid) AS likes,") +
               is_like_sql +
               "FROM media_comment LEFT JOIN media_user ON media_comment.user = media_user.userid ";
        std::string condition = "WHERE media_comment.rid = " + GetSqlStr(rid);
        if(type == 2){
            condition += " AND media_comment.reply_to = " + GetSqlStr(cid);
        }
        else{
            condition += " AND media_comment.reply_to IS NULL";
        }
        const std::string order_by = " ORDER BY media_comment.publish_time DESC";
        const std::string query = sql + condition + order_by;
        return conn_pool->GetConnection()->Query(query.c_str());
    }

    Json::Value CheckLike(const std::string rid,const std::string uid){
        const std::string query = "SELECT rid,user FROM like_record WHERE rid=" + GetSqlStr(rid) +
                                  " AND user=" + ((IsDigitStr(uid)) ? uid : GetSqlStr(uid));
        return conn_pool->GetConnection()->Query(query.c_str());
    }

    Json::Value AddLikeRecord(const std::string rid,const std::string uid){
        const std::string query = "INSERT INTO like_record(rid,user,publish_time) VALUES(" + GetSqlStr(rid) + "," +
                                  ((IsDigitStr(uid)) ? uid : GetSqlStr(uid)) + ",NOW())";
        return conn_pool->GetConnection()->Execute(query.c_str());
    }

    Json::Value AddResourceLike(const std::string rid){
        const std::string query = "UPDATE media_resource SET likes = likes + 1 WHERE rid = " + GetSqlStr(rid);
        return conn_pool->GetConnection()->Execute(query.c_str());
    }

    Json::Value CheckCommentLike(const std::string cid,const std::string uid){
        const std::string query = "SELECT cid,user FROM like_comment WHERE cid=" + GetSqlStr(cid) +
                                  " AND user=" + ((IsDigitStr(uid)) ? uid : GetSqlStr(uid));
        return conn_pool->GetConnection()->Query(query.c_str());
    }

    Json::Value AddCommentLikeRecord(const std::string cid,const std::string uid){
        const std::string query = "INSERT INTO like_comment(cid,user,publish_time) VALUES(" + GetSqlStr(cid) + "," +
                                  ((IsDigitStr(uid)) ? uid : GetSqlStr(uid)) + ",NOW())";
        return conn_pool->GetConnection()->Execute(query.c_str());
    }

    Json::Value SelectLikeHistory(const std::string uid){
        const std::string query = "SELECT rid,publish_time FROM like_record WHERE user = " +
                                  ((IsDigitStr(uid)) ? uid : GetSqlStr(uid)) +
                                  " ORDER BY publish_time DESC";
        return conn_pool->GetConnection()->Query(query.c_str());
    }

    Json::Value SelectCommentLikeHistory(const std::string uid){
        const std::string query = "SELECT cid,publish_time FROM like_comment WHERE user = " +
                                  ((IsDigitStr(uid)) ? uid : GetSqlStr(uid)) +
                                  " ORDER BY publish_time DESC";
        return conn_pool->GetConnection()->Query(query.c_str());
    }
};

#endif
