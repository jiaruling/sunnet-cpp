#include "SocketWorker.h"
#include <iostream>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <Sunnet.h>
#include <fcntl.h>
#include <sys/socket.h>

// 初始化
void SocketWorker::Init() {
    cout << "SocketWorker Init" << endl;
    // 创建epoll对象
    epollFd = epoll_create(1024); // 返回值：非负数表示成功创建的epoll对象的描述符,-1表表示创建失败
    assert(epollFd > 0);
}

void SocketWorker::operator()() {
    while(true) {
        //阻塞等待
        const int EVENT_SIZE = 64;
        struct epoll_event events[EVENT_SIZE];
        int eventCount = epoll_wait(epollFd, events, EVENT_SIZE, -1);
        //取得事件
        for (int i=0;i<eventCount;i++) {
            epoll_event ev = events[i]; // 当前要处理的事件
            OnEvent(ev);
        }
    }
}

// 注意跨线程调用
void SocketWorker::AddEvent(int fd) {
    cout << "AddEvent fd " << fd << endl;
    // 添加到epoll对象
    struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLET;
    ev.data.fd = fd;
    if (epoll_ctl(epollFd, EPOLL_CTL_ADD, fd, &ev)==-1) {
        cout << "AddEvent epoll_ctl Fail:" << strerror(errno) << endl;
    }
}

// 跨线程调用
void SocketWorker::RemoveEvent(int fd) {
    cout << "RemoveEvent fd" << fd << endl;
    epoll_ctl(epollFd, EPOLL_CTL_DEL, fd, NULL);
}

// 跨线程调用
void SocketWorker::ModifyEvent(int fd, bool epollOut) {
    cout << "ModifyEvent fd " << fd << " " << epollOut << endl;
    struct epoll_event ev;
    ev.data.fd = fd;
    if (epollOut) {
        ev.events = EPOLLIN | EPOLLET | EPOLLOUT;
    } else {
        ev.events = EPOLLIN | EPOLLET;
    }
    epoll_ctl(epollFd, EPOLL_CTL_MOD, fd, &ev);
}

// 处理事件
void SocketWorker::OnEvent(epoll_event ev) {
    // cout << "OnEvent" << endl;
    int fd = ev.data.fd;
    auto conn = Sunnet::inst -> GetConn(fd);
    if (conn == NULL) {
        cout << "OnEvent error, conn == NULL" << endl;
        return;
    }
    // 事件类型
    bool isRead = ev.events & EPOLLIN;
    bool isWrite = ev.events & EPOLLOUT;
    bool isError = ev.events & EPOLLERR;
    // 监听socket
    if (conn -> type == Conn::TYPE::LISTEN) {
        if(isRead) {
            OnAccept(conn);
        }
    }
    // 普通socket
    else {
        if (isRead || isWrite) {
            OnRW(conn, isRead, isWrite);
        }
        if (isError) {
            cout << "OnError fd:" << conn->fd << endl;
        }
    }
    return;
}

void SocketWorker::OnAccept(shared_ptr<Conn> conn) {
    cout << "OnAccept fd:" << conn->fd << endl;
    // 步骤1：accept
    int clientFd = accept(conn->fd, NULL, NULL);
    if (clientFd < 0) {
        cout << "accept error: " << endl;
    }
    // 步骤2：设置非阻塞
    fcntl(clientFd, F_SETFL, O_NONBLOCK);
    // 步骤3：添加连接对象
    Sunnet::inst->AddConn(clientFd, conn->serviceId, Conn::TYPE::CLIENT);
    // 步骤4：添加到epoll监听列表
    AddEvent(clientFd);
    // 步骤5：通知服务
    auto msg = make_shared<SocketAcceptMsg>();
    msg->type = BaseMsg::TYPE::SOCKET_ACCEPT;
    msg->listenFd = conn->fd;
    msg->clientFd = clientFd;
    Sunnet::inst->Send(conn->serviceId, msg);
    return;
}

void SocketWorker::OnRW(shared_ptr<Conn> conn, bool r, bool w) {
    cout << "OnRW " << conn->fd << endl;
    auto msg = make_shared<SocketRWMsg>();
    msg->type = BaseMsg::TYPE::SOCKET_RW;
    msg->fd = conn->fd;
    msg->isRead = r;
    msg->isWrite = w;
    Sunnet::inst->Send(conn->serviceId, msg);
    return;
}