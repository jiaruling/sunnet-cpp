#include <iostream>
#include <unistd.h>
#include "Worker.h"
#include "Service.h"
using  namespace std;

// 线程函数
void Worker::operator()() {
    while(true) {
        shared_ptr<Service> srv = Sunnet::inst->PopGlobalQueue();
        if (!srv) {
            Sunnet::inst->WorkerWait();
        } else {
            srv->ProcessMsgs(eachNum);
            CheckAndPutGlobal(srv);
        }
    }
}

// 那些调Sunnet的通过传参数解决
// 状态是不在队列中，global=ture
void Worker::CheckAndPutGlobal(shared_ptr<Service> srv) {
    // 退出中（服务的退出方式只能它自己调用，这样isExiting才不会产生线程冲突）
    if (srv->isExiting){
        return;
    }
    pthread_spin_lock(&srv->queueLock);
    {
        //重新返回全局队列
        if(!srv->msgQueue.empty()) {
            // 此时srv->inGlobal一定是true
            Sunnet::inst -> PushGlobalQueue(srv);
        }
        // 不在队列中，重设inGlobal
        else {
            srv->SetInGlobal(false);
        }
    }
    pthread_spin_unlock(&srv->queueLock);
}