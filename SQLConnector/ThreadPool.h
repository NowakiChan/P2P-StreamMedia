#ifndef MYSQL_THREAD_POOL
#define MYSQL_THREAD_POOL
#include"./Connector.h"
#include<queue>
#include<mutex>
#include<condition_variable>
#include<tuple>
#include<thread>
#include<vector>
#include<iostream>

class SubmitItem{
public:
    Json::Value result_item = Json::nullValue;
    bool finish_flag = false;
};
typedef std::tuple<std::string,SubmitItem*,bool> WorkingItem;

class SQLThreadPool{
private:
    std::mutex working_lock;
    bool is_running;
    std::queue<WorkingItem> working_resources;
    std::mutex running_flag_lock;
    std::vector<std::thread> thread_pool;
private:
    class WorkingThread{
    private:
        SQLThreadPool* pool;
        Connector* db_conn;
        int thread_id;
    public:
        WorkingThread() = delete;

        WorkingThread(SQLThreadPool* pool_ptr,const DBConnectionConfig& conn_config,const int id)
                     : thread_id(id){
            pool = pool_ptr;
            db_conn = new Connector(conn_config);
            {
                /*测试部分
                std::cout<<"Try excuting SQL query\n";
                Json::Value v;
                db_conn->ExcuteSql("SELECT username FROM media_user",v,true);
                std::cout<<"Excuting Success ->\n"<<v.toStyledString();
                */
            }
        }

        void Start(){
            std::cout<<"Thread "<<thread_id<<" now running\n";
            while(pool->IsRunning()){
                WorkingItem new_work(std::string(""),NULL,false);
                if(pool->GetItem(new_work)){
                    // std::cout<<"Thread "<<thread_id<<" now running new job\n";
                    const std::string sql = std::get<0>(new_work);
                    // Json::Value& result_set = std::get<1>(new_work);
                    db_conn->ExcuteSql(sql.c_str(),std::get<1>(new_work)->result_item,std::get<2>(new_work));
                    std::get<1>(new_work)->finish_flag = true;
                }
                else if(pool->IsRunning()){
                    // std::cout<<"Thread "<<thread_id<<" now waiting\n";
                    pool->working_var.wait(pool->working_unipue_lock);
                }
                else{
                    delete db_conn;
                    break;
                }
            }
            {
                /*测试部分*/
                std::cout<<"Thread "<<thread_id<<" now exit\n";
            }
        }

        void operator()(){
            Start();
        }
    };
public:
    std::unique_lock<std::mutex> working_unipue_lock;
    std::condition_variable working_var; //条件变量，用于任务唤醒
    std::mutex resource_lock; //资源锁，用于互斥访问资源队列
private:
    void CreateThread(const DBConnectionConfig& config,const unsigned int thread_number){
        for(unsigned int i = 0;i < thread_number;i++){
            thread_pool.at(i) = std::thread(WorkingThread(this,config,i));
            // new_working_thread.detach();
        }
    }
public:
    SQLThreadPool(const DBConnectionConfig& config,const unsigned int thread_num = 10){
        working_resources = std::queue<WorkingItem>();
        working_unipue_lock = std::unique_lock<std::mutex>(working_lock);
        thread_pool = std::vector<std::thread>(thread_num);
        // working_var = std::unique_lock<std::mutex>(working_lock);
        is_running = true;

        CreateThread(config,thread_num);
    }

    bool IsRunning() const { return is_running; }

    bool GetItem(WorkingItem& first_item){
        bool flag = false;
        resource_lock.lock();

        if(!working_resources.empty()){
            first_item = working_resources.front();
            working_resources.pop();
            flag = true;
        }

        resource_lock.unlock();

        return flag;
    }

    void Submit(const std::string sql_str,SubmitItem* result_set,bool has_result){
        resource_lock.lock();

        working_resources.push(WorkingItem(sql_str,result_set,has_result));
        resource_lock.unlock();
        
        working_var.notify_one();
    }

    Json::Value WaitFinish(const std::string query,bool result_flag){
        SubmitItem new_submit_item;
        Submit(query,&new_submit_item,result_flag);

        while(!new_submit_item.finish_flag); // 等待完成

        return new_submit_item.result_item;
    }

    void ShutDown(){
        // running_flag_lock.lock();
        is_running = false;
        // running_flag_lock.unlock();

        working_var.notify_all();

        for(int i = 0;i < thread_pool.size();i++){
            if(thread_pool.at(i).joinable()){
                std::cout<<"Exiting thread "<<i<<"\n";
                thread_pool.at(i).detach();
            }
            else std::cout<<"Thread "<<i<<" already exit\n";
        }
    }
    
    ~SQLThreadPool(){
        if(!is_running)
            ShutDown();
    }
};


#endif