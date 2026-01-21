#ifndef DATA_PLUGIN
#define DATA_PLUGIN
#include<iomanip>
#include<iostream>
#include<sstream>
#include<string>
#include<cstring>
#include<openssl/evp.h>
#include"../SQLConnector/ThreadPool.h"
// static Connector conn("root","","localhost","StreamMedia");
/*数据库连接的全局配置,以后这个配置要从文件读取*/
static const DBConnectionConfig global_mysql_config("localhost","root","","StreamMedia");

inline std::string GetSqlStr(const std::string str){
    return std::string("'" + str + "'");
}

inline bool IsDigitStr(const std::string str){
    for(int i = 0;i < str.size();i++){
        if(!isdigit(str.at(i)))
            return false;
    }

    return true;
}

inline Json::Value GetJsonFromStr(const std::string jstr){
    Json::Reader reader;
    Json::Value res = Json::nullValue;
    reader.parse(jstr,res,false);

    return res;
}

bool ComputeSHA256(const std::string& unhashed, std::string& hashed)
{
    bool success = false;

    EVP_MD_CTX* context = EVP_MD_CTX_new();

    if(context != NULL)
    {
        if(EVP_DigestInit_ex(context, EVP_sha256(), NULL))
        {
            if(EVP_DigestUpdate(context, unhashed.c_str(), unhashed.length()))
            {
                unsigned char hash[EVP_MAX_MD_SIZE];
                unsigned int lengthOfHash = 0;

                if(EVP_DigestFinal_ex(context, hash, &lengthOfHash))
                {
                    std::stringstream ss;
                    for(unsigned int i = 0; i < lengthOfHash; ++i)
                    {
                        ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
                    }

                    hashed = ss.str();
                    success = true;
                }
            }
        }

        EVP_MD_CTX_free(context);
    }

    return success;
}

#endif