#ifndef ACCOUNT_DB
#define ACCOUNT_DB
#include"../Plugins/DataPlugins.h"
#include"../SQLConnector/ThreadPool.h"
#include<string>

class AccountDB{
private:
    ThreadPool<Connector>* conn_pool;
public:
    AccountDB(ThreadPool<Connector>* pool) : conn_pool(pool) {}
    Json::Value SelectUserById(const std::string uid){
        // Json::Value res;
        const std::string sql_str = "SELECT userid,username,autograph,last_login_time,register_time FROM media_user WHERE userid=" +
                                    uid;
        // conn.ExcuteSql(sql_str.c_str(),res,true);

        return conn_pool->GetConnection()->Query(sql_str.c_str());
    }

    Json::Value SelectUserByName(const std::string username,bool fuzzy_search = true){
        // Json::Value res;
        const std::string sql_str = "SELECT userid,username,autograph,last_login_time,register_time FROM media_user WHERE username" +
                                    std::string(((fuzzy_search) ? (" like ") : "=")) + ((fuzzy_search) ? GetSqlStr("%" + username + "%") : GetSqlStr(username));
        // conn.ExcuteSql(sql_str.c_str(),res,true);

        return conn_pool->GetConnection()->Query(sql_str.c_str());
    }

    Json::Value SelectUserByPwd(const std::string identity,const std::string pwd){
        // Json::Value res;
        const std::string condition_str = "(userid=" + ((IsDigitStr(identity)) ? identity : GetSqlStr(identity)) + " OR username=" + GetSqlStr(identity) + ")";
        const std::string sql_str = "SELECT * FROM media_user WHERE " + condition_str + " AND pwd=" + GetSqlStr(pwd);
        // conn.ExcuteSql(sql_str.c_str(),res,true);

        return conn_pool->GetConnection()->Query(sql_str.c_str());
    }

    Json::Value AddNewUser(const std::string username,const std::string pwd){
        // Json::Value res;
        const std::string sql_str = "INSERT INTO media_user(username,pwd,register_time) VALUES(" + GetSqlStr(username) + "," + 
                                    GetSqlStr(pwd) + ", NOW() )";
        // conn.ExcuteSql(sql_str.c_str(),res);

        return conn_pool->GetConnection()->Execute(sql_str.c_str());
    }

    Json::Value DeleteUser(const std::string userid){
        // Json::Value res;
        const std::string sql_str = "DELETE FROM media_user WHERE userid=" + userid;
        // conn.ExcuteSql(sql_str.c_str(),res);

        return conn_pool->GetConnection()->Execute(sql_str.c_str());
    }

    Json::Value UpdateInfo(const std::string userid,const std::string new_username,const std::string new_autograph,const std::string new_pwd = "",bool pwd_only = false){
        // Json::Value res;
        std::string sql_str;
        if(pwd_only){
            sql_str = "UPDATE media_user SET pwd=" + GetSqlStr(new_pwd) + " WHERE userid=" + userid;
        }
        else{
            sql_str = "UPDATE media_user SET username=" + GetSqlStr(new_username) + "," + "autograph=" + GetSqlStr(new_autograph) + 
                      " WHERE userid=" + userid;
        }
        // conn.ExcuteSql(sql_str.c_str(),res);

        return conn_pool->GetConnection()->Execute(sql_str.c_str());
    }

    Json::Value UpdateLoginTime(const std::string userid){
        const std::string sql_str = "UPDATE media_user SET last_login_time = NOW() WHERE userid=" + userid;
        
        return conn_pool->GetConnection()->Execute(sql_str.c_str());
    }

    Json::Value UpdateAvatorLink(const std::string userid,const std::string path){
        const std::string sql_str = "UPDATE media_user SET avator_link = " + GetSqlStr(path) +
                                    " WHERE userid=" + userid;
        return conn_pool->GetConnection()->Execute(sql_str.c_str());
    }
};

#endif