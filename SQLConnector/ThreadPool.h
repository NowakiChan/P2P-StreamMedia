#ifndef MYSQL_THREAD_POOL
#define MYSQL_THREAD_POOL
#include"./Connector.h"
#include<algorithm>
#include<chrono>
#include<condition_variable>
#include<cstddef>
#include<memory>
#include<mutex>
#include<thread>
#include<vector>

template<class ConnectorType>
class ThreadPool{
private:
    struct Slot{
        ConnectorType* ptr;
        std::chrono::steady_clock::time_point last_idle_since;
    };

    static constexpr std::chrono::minutes IDLE_TIMEOUT{5};
    static constexpr unsigned int EVICT_PER_CYCLE_MAX = 2;
    static constexpr unsigned int MONITOR_WAKE_SECONDS_MIN = 5;

    std::vector<Slot> conn_pool;
    unsigned int max_connect_num;
    unsigned int mininum_connect_num;
    unsigned int monitor_sleep_time;
    bool open;
    std::thread monitor_thread;
    DBConnectionConfig config;

    void MonitorThread();
    void EvictIdleConnectionsLocked();

public:
    std::mutex pool_lock;
    std::condition_variable monitor_var;

    ThreadPool(const DBConnectionConfig& conn_config)
        : max_connect_num(conn_config.conn_pool_max_size),
          mininum_connect_num(conn_config.conn_pool_min_size),
          open(true),
          monitor_sleep_time(std::max(conn_config.conn_pool_refresh_time,MONITOR_WAKE_SECONDS_MIN)),
          config(conn_config),
          monitor_thread(&ThreadPool::MonitorThread,this)
    {}

    std::shared_ptr<ConnectorType> GetConnection();

    bool IsOpen() const { return open; }

    void ClosePool();

    ~ThreadPool(){
        if(open)
            ClosePool();
    }
};

template<typename T>
void ThreadPool<T>::EvictIdleConnectionsLocked(){
    const auto now = std::chrono::steady_clock::now();
    unsigned evicted = 0;
    while(evicted < EVICT_PER_CYCLE_MAX && conn_pool.size() > mininum_connect_num){
        std::size_t best_idx = SIZE_MAX;
        auto oldest_idle_begin = std::chrono::steady_clock::time_point::max();
        for(std::size_t i = 0;i < conn_pool.size();++i){
            const auto idle_for = now - conn_pool[i].last_idle_since;
            if(idle_for >= IDLE_TIMEOUT && conn_pool[i].last_idle_since < oldest_idle_begin){
                oldest_idle_begin = conn_pool[i].last_idle_since;
                best_idx = i;
            }
        }
        if(best_idx == SIZE_MAX)
            break;
        delete conn_pool[best_idx].ptr;
        conn_pool.erase(conn_pool.begin() + static_cast<std::ptrdiff_t>(best_idx));
        ++evicted;
    }
}

template<typename T>
void ThreadPool<T>::MonitorThread(){
    std::unique_lock<std::mutex> lock(pool_lock);
    while(open){
        /* wait_for：阻塞等待时会释放 pool_lock，唤醒返回前重新加锁，因此不会在睡眠期间一直占用锁 */
        monitor_var.wait_for(lock,std::chrono::seconds(monitor_sleep_time),[this]{ return !open; });
        if(!open)
            break;
        EvictIdleConnectionsLocked();
    }
}

template<typename T>
void ThreadPool<T>::ClosePool(){
    {
        std::unique_lock<std::mutex> lock(pool_lock);
        open = false;
    }
    monitor_var.notify_all();
    if(monitor_thread.joinable())
        monitor_thread.join();

    std::unique_lock<std::mutex> lock(pool_lock);
    while(!conn_pool.empty()){
        delete conn_pool.back().ptr;
        conn_pool.pop_back();
    }
}

template<typename T>
std::shared_ptr<T> ThreadPool<T>::GetConnection(){
    std::unique_lock<std::mutex> lock(pool_lock);

    auto PtrRecollection = [this](T* ptr){
        std::unique_lock<std::mutex> lk(this->pool_lock);
        if(!this->open){
            delete ptr;
            return;
        }
        if(this->conn_pool.size() < this->max_connect_num){
            this->conn_pool.push_back({ptr,std::chrono::steady_clock::now()});
            this->monitor_var.notify_one();
        }
        else{
            delete ptr;
        }
    };

    if(!conn_pool.empty()){
        T* raw = conn_pool.back().ptr;
        conn_pool.pop_back();
        return std::shared_ptr<T>(raw,PtrRecollection);
    }

    return std::shared_ptr<T>(new T(config),PtrRecollection);
}

#endif
