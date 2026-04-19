#ifndef DATA_PLUGIN
#define DATA_PLUGIN
#include<iomanip>
#include<iostream>
#include<sstream>
#include<string>
#include<cstring>
#include<openssl/evp.h>
#include<jsoncpp/json/json.h>
#include<ctime>
#include<mutex>
#include <algorithm>
// #include"../SQLConnector/ThreadPool.h"
// static Connector conn("root","","localhost","StreamMedia");
/*数据库连接的全局配置,以后这个配置要从文件读取*/
// static const DBConnectionConfig global_mysql_config("localhost","root","","StreamMedia");

std::string GetSqlStr(const std::string);
bool IsDigitStr(const std::string);
Json::Value GetJsonFromStr(const std::string);
bool ComputeSHA256(const std::string&, std::string&);
std::string GetRedisStr(int&&);
std::string GetRedisStr(double&&);
std::string GetRedisStr(float&&);
std::string GetRedisStr(std::string&&);
std::string GetRedisStr(const char*);
std::string FillString(const char,const int,const std::string);

class ResourceAlgo
{
private:
    static const unsigned int max_history_records = 5; // 最多使用历史记录次数
    static constexpr double cold_active_punishment = 0.7;  // 冷启动时的基数惩罚
    static constexpr double ema_value = 0.65; // ema平滑值
    static constexpr double max_increasement = 0.3; // 最大增长值
    static const unsigned int top_k = 5; // 选取的前top-K个节点
    static const unsigned int max_return_nodes = 3; // 最多返回几个节点

    /*雪花算法相关*/
    static const unsigned int max_pertime_number = 4095; // 每个时间戳允许生成的最大序号数量
    static std::mutex timestamp_lock;
    static time_t last_timestamp;
    static unsigned int current_pertime_number;
    static const unsigned int timestamp_shift_bits = 32;
private:
    ResourceAlgo() { 
        last_timestamp = std::time(nullptr);
        current_pertime_number = 0;
    }
public:
    static ResourceAlgo& GetInstance() {
        static ResourceAlgo singleton;
        return singleton; 
    }

    double CalculateWeight(double new_upload, double new_download, double new_conn,
                       double old_upload = 0, double old_download = 0, double old_conn = 0) {
        const double eps = 1e-6;
        bool cold = (old_upload < eps && old_download < eps && old_conn < eps);
        double upload, download, conn;
        if (cold) {
            upload = new_upload;
            download = new_download;
            conn = new_conn;
        } else {
            // 限幅 + EMA
            auto smooth = [&](double new_v, double old_v) {
                double delta = new_v - old_v;
                delta = std::clamp(delta, -max_increasement, max_increasement);
                return old_v + ema_value * delta;
            };
            upload = smooth(new_upload, old_upload);
            download = smooth(new_download, old_download);
            conn = smooth(new_conn, old_conn);
        }
        // 瓶颈 + 平均
        double avg = (upload + download + conn) / 3.0;
        double bottleneck = std::max({upload, download, conn});
        double weight = 0.6 * bottleneck + 0.4 * avg;
        weight = std::clamp(weight, 0.0, 1.0);
        double score = 1.0 - weight;
        // 冷启动衰减
        if (cold) {
            score *= cold_active_punishment;
        }

        return score;
    }

    uint64_t SnowflakeID(const std::string identity = ""){
        time_t now = std::time(nullptr);
        int sequence_number = 0;
        {
            std::unique_lock<std::mutex> set_timestamp(timestamp_lock);
            if(now > last_timestamp){
                last_timestamp = now;
                current_pertime_number = 0;
            }
            else if(now == last_timestamp){
                sequence_number = ++current_pertime_number;
                if(current_pertime_number > max_pertime_number)
                throw std::runtime_error("Reach max pertime timestamp number");
            }
            else throw std::runtime_error("Wrong timestamp,check machine time");
        }
        const uint64_t id = (now << timestamp_shift_bits) | sequence_number;
        
        return id;
    }
};
#endif