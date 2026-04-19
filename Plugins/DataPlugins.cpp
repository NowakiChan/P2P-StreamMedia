#include "DataPlugins.h"

std::mutex ResourceAlgo::timestamp_lock;
time_t ResourceAlgo::last_timestamp = 0;
unsigned int ResourceAlgo::current_pertime_number = 0;

std::string GetSqlStr(const std::string str){
    return std::string("'" + str + "'");
}

bool IsDigitStr(const std::string str){
    for(int i = 0;i < str.size();i++){
        if(!isdigit(str.at(i)))
            return false;
    }

    return true;
}

Json::Value GetJsonFromStr(const std::string jstr){
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

std::string GetRedisStr(int&& value){
    return std::to_string(value);
}

std::string GetRedisStr(double&& value){
    return std::to_string(value);
}

std::string GetRedisStr(float&& value){
    return std::to_string(value);
}

std::string GetRedisStr(std::string&& str){
    return str;
}

std::string GetRedisStr(const char* str){
    return str;
}

std::string FillString(const char letter,const int length,const std::string origin){
    if(length >= origin.size()){
        std::string res;
        for(int i = origin.size();i <= length;i++)
            res += letter;
        return res + origin;
    }

    return origin;
}