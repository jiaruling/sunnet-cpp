# pragma once
#include <vector>
#include "Worker.h"
#include "SocketWorker.h"
#include "Service.h"
#include <unordered_map>

class Worker;

class Sunnet {
    public:
        // 单例
        static Sunnet* inst;
    public:
        // 构造函数
        Sunnet();
        // 初始化并开始
        void Start();
        // 等待运行
        void Wait();
    public:
        // 工作线程
        int WORKER_NUM = 3; // 工作线程数(配置)
        vector<Worker*> workers;       //  worker对象
        vector<thread*> workerThreads; // 线程
    private:
        // 开启工作线程
        void StartWorker();
    public:
        // 服务列表
        unordered_map<uint32_t, shared_ptr<Service>> services;
        uint32_t maxId = 0; // 最大Id
        pthread_rwlock_t servicesLock; // 读写锁
    public:
        // 增删服务
        uint32_t NewService(shared_ptr<string> type);
        void KillService(uint32_t id); //仅限服务自己调用
    private:
        // 获取服务
        shared_ptr<Service> GetService(uint32_t id);
    private:
        // 全局队列
        queue<shared_ptr<Service>> globalQueue;
        int globalLen = 0;  // 队列长度
        pthread_spinlock_t globalLock; // 锁
    public:
        // 发送消息
        void Send(uint32_t toId, shared_ptr<BaseMsg> msg);
        // 全局队列操作
        shared_ptr<Service> PopGlobalQueue();
        void PushGlobalQueue(shared_ptr<Service> srv);
    public:
        shared_ptr<BaseMsg> MakeMsg(uint32_t source, char* buff, int len);
    private:
        // 休眠和唤醒
        pthread_mutex_t sleepMtx;   // 互斥锁
        pthread_cond_t sleepCond;  // 条件变量
        int sleepCount = 0; // 休眠工作线程数
    public:
        // 唤醒工作线程
        void CheckAndWeakUp();
        // 让工作线程等待(仅工作线程调用)
        void WorkerWait();
    private:
        // socket线程
        SocketWorker* socketWorker;
        thread* socketThread;
    private:
        // 开启socket线程
        void StartSocket();
    public:
        // 增删查Conn
        int AddConn(int fd, uint32_t id, Conn::TYPE type);
        shared_ptr<Conn> GetConn(int fd);
        bool RemoveConn(int fd);
    private:
        // Conn列表
        unordered_map<uint32_t, shared_ptr<Conn>> conns;
        pthread_rwlock_t connsLock; //读写锁
    public:
        // 网络连接操作接口
        int Listen(uint32_t port,uint32_t serviceId);
        void CloseConn(uint32_t fd);
};