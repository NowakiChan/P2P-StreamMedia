#ifndef MYSQL_THREAD_POOL
#define MYSQL_THREAD_POOL
#include"./Connector.h"
#include<queue>
#include<mutex>
#include<condition_variable>
#include<thread>
#include<vector>
#include<memory>
#include<functional>

template<class ConnectorType>
class ThreadPool{
private:
    std::mutex monitor_lock;
    std::vector<ConnectorType* > conn_pool;
    unsigned int max_connect_num;
    unsigned int mininum_connect_num;
    unsigned int monitor_sleep_time;
    bool open;
    std::thread monitor_thread;
    DBConnectionConfig config;
public:
    std::mutex pool_lock;
    std::condition_variable monitor_var;
private:
    void MonitorThread();
public:
    ThreadPool(const DBConnectionConfig& conn_config)
               : max_connect_num(conn_config.conn_pool_max_size) , 
                 mininum_connect_num(conn_config.conn_pool_min_size) , open(true) ,
                 monitor_sleep_time(conn_config.conn_pool_refresh_time) , config(conn_config) ,
                 monitor_thread(std::thread(std::bind(&ThreadPool::MonitorThread,this)))
    {
        monitor_thread.detach();
    }

    std::shared_ptr<ConnectorType> GetConnection();

    bool IsOpen() const { return open; }

    void ClosePool();

    ~ThreadPool(){
        if(open) ClosePool();
    }
};

template<typename T>
void ThreadPool<T>::MonitorThread(){
    while(this->open){
        {
            /* 测试代码 */
            std::cout<<"Adjust pool size to "<<max_connect_num<<"\n";
            std::cout<<"Storage : "<<conn_pool.size()<<"/"<<max_connect_num<<"\n";
        }
        std::unique_lock<std::mutex> lock(monitor_lock);
        std::cv_status return_status = monitor_var.wait_for(lock,std::chrono::seconds(monitor_sleep_time));
        if(!this->open) break;

        std::unique_lock<std::mutex> scan_lock(pool_lock);
        if(conn_pool.size() > max_connect_num){
            while(conn_pool.size() > max_connect_num){
                conn_pool.pop_back();
            }
            continue;
        }
        else if(conn_pool.size() > max_connect_num - 2){
            max_connect_num++;
            continue;
        }

        if(return_status == std::cv_status::timeout && max_connect_num > mininum_connect_num){
            max_connect_num--;
        }
    }
    std::cout<<"Monitor Exit\n";
}

template<typename T>
void ThreadPool<T>::ClosePool(){
    open = false;
    monitor_var.notify_all();
    if(monitor_thread.joinable()) monitor_thread.join();

    // mininum_connect_num = max_connect_num = 0; // 设置容量为0

    while(conn_pool.size() > 0) conn_pool.pop_back();
}

template<typename T>
std::shared_ptr<T> ThreadPool<T>::GetConnection(){
    std::unique_lock<std::mutex> lock(pool_lock);

    auto PtrRecollection = [this](T* ptr){
        if(this->conn_pool.size() < this->max_connect_num && this->open){
            std::unique_lock<std::mutex> lock(this->pool_lock);
            this->conn_pool.emplace_back(ptr);
            this->monitor_var.notify_all();
        }
        else delete ptr;   
    };

    if(!conn_pool.empty()){
        std::shared_ptr<T> return_conn(conn_pool.back(),PtrRecollection);
        conn_pool.pop_back();
        std::cout<<"Return a current ptr\n";
        return return_conn;
    }
    
    std::shared_ptr<T> new_ptr(new T(config),PtrRecollection);
    std::cout<<"Create new ptr\n";
    return new_ptr;
}


#endif