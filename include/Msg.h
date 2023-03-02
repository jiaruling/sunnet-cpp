#pragma once
#include <memory>
using namespace std;

//消息基类
class BaseMsg {
    public:
        enum TYPE {     // 消息类型
            SERVICE = 1,
            SOCKET_ACCEPT = 2,
            SOCKET_RW = 3,
        };
        uint8_t type;  //消息类型
        char load[999999]{}; // 用于检测内存泄漏, 仅用于测试
        virtual ~BaseMsg(){};
};

// 服务间消息
class ServiceMsg:public BaseMsg {
    public:
        uint32_t source; // 消息发送方
        shared_ptr<char> buff; //消息内容
        size_t size; //消息内容大小
};

// 有新连接
class SocketAcceptMsg:public BaseMsg {
    public:
        int listenFd; // 监听套接字描述符
        int clientFd; // 客户端的套接字描述符
};

// 可读可写
class SocketRWMsg:public BaseMsg {
    public:
        int  fd;  // 发生事件的套接字描述符
        bool isRead=false;  // true 可读
        bool isWrite=false; // true 可写
};