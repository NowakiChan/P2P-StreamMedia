#ifndef MYSQL_CONNECTOR
#define MYSQL_CONNECTOR
#include<mysql/mysql.h>
#include<jsoncpp/json/json.h>
#include<cstring>
#include<string>
#include<mutex>

class DBConnectionConfig{
public:
    std::string host;
    std::string user;
    std::string pwd;
    std::string db_name;
    unsigned int port;
    DBConnectionConfig() = default;

    DBConnectionConfig(const char* _host,const char* _user,const char* _pwd,
                       const char* _db_name,const unsigned int _port = 3306) :
                       host(_host) , user(_user) , pwd(_pwd) , db_name(_db_name) , port(_port)
    {}
};

class Connector{
private:
    MYSQL mysql_initializer;
private:
    /*这个函数无论如何都会返回一个Json数组或者空值，即便结果只有一行*/
    Json::Value ProcessResult(){
        Json::Value res = Json::nullValue,row_res = Json::nullValue;

        /*在mysql_real_query到mysql_store_res之间线程不安全*/
        MYSQL_RES* mysql_res = mysql_store_result(&mysql_initializer);
        const int field_number = mysql_num_fields(mysql_res),row_number = mysql_num_rows(mysql_res);
        std::string field_name[field_number];

        //读取列内容
        MYSQL_FIELD* result_field;
        for(int i = 0;(result_field = mysql_fetch_field(mysql_res));i++){
            field_name[i] = result_field->name;
        }

        // 读取行内容
        MYSQL_ROW current_row = mysql_fetch_row(mysql_res);
        for(int j = 0;j < row_number;j++){
            for(int i = 0;i < field_number;i++){
                row_res[field_name[i]] = (current_row[i] == NULL) ? "NULL" : current_row[i];
            }

            res.append(row_res);

            mysql_fetch_row(mysql_res);
        }

        mysql_free_result(mysql_res);
        return res;
    }

    Json::Value GetExcuteInfo(){
        Json::Value error_info;
        error_info["errorid"] = mysql_errno(&mysql_initializer);
        error_info["details"] = mysql_error(&mysql_initializer);
        error_info["change_rows"] = (long)mysql_affected_rows(&mysql_initializer);

        return error_info;
    } 
public:
    /* 不允许默认构造，不允许拷贝赋值和拷贝构造 */
    Connector() = delete;
    Connector operator = (const Connector& _conn) = delete;
    Connector(const Connector& _conn) = delete;
 

    Connector(const DBConnectionConfig& config) {
        mysql_library_init(0,NULL,NULL);
        mysql_init(&mysql_initializer);
        mysql_real_connect(&mysql_initializer,config.host.c_str(),config.user.c_str(),
                           config.pwd.c_str(),config.db_name.c_str(),config.port,NULL,0);
    }

    Connector(const char* username,const char* pwd,const char* host,const char* db_name,const unsigned int port = 3306){
        mysql_library_init(0,NULL,NULL);
        mysql_init(&mysql_initializer);
        mysql_real_connect(&mysql_initializer,host,username,
                           pwd,db_name,port,NULL,0);
    }

    Json::Value Query(const char* sql_str){
        Json::Value excute_result;
        mysql_real_query(&mysql_initializer,sql_str,strlen(sql_str));
    
        excute_result = GetExcuteInfo();
        if(excute_result["errorid"].asInt() == 0)
            excute_result["data"] = ProcessResult();
        else
            excute_result["data"] = Json::nullValue;
        return excute_result;
    }

    Json::Value Execute(const char* sql_str){
        mysql_real_query(&mysql_initializer,sql_str,strlen(sql_str));
        Json::Value excute_result = GetExcuteInfo(); 

        return excute_result;
    }

    bool Error(){
        return mysql_errno(&mysql_initializer) == 0;
    }

    ~Connector() {
        mysql_close(&mysql_initializer);
        mysql_library_end();
    }
};


#endif