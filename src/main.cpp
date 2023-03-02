#include "Sunnet.h"
#include <unistd.h>

int test() {
    auto pingType = make_shared<string>("ping");
    uint32_t ping1 = Sunnet::inst->NewService(pingType);
    uint32_t ping2 = Sunnet::inst->NewService(pingType);
    uint32_t pong = Sunnet::inst->NewService(pingType);

    auto msg1 = Sunnet::inst->MakeMsg(ping1, new char[3] {'h','i','\0'}, 3);
    auto msg2 = Sunnet::inst->MakeMsg(ping2, new char[6] {'h','e','l','l','o','\0'}, 6);
    Sunnet::inst->Send(pong, msg1);
    Sunnet::inst->Send(pong, msg2);
}

int TestSocketCtrl() {
    int fd = Sunnet::inst->Listen(9999, 1);
    usleep(300*100000);
    Sunnet::inst->CloseConn(fd);
}

int TestEcho() {
    auto t = make_shared<string>("gateway");
    uint32_t gateway = Sunnet::inst->NewService(t);
}

int main() {
    new Sunnet();
    Sunnet::inst->Start();
    // 开启系统后的一些逻辑
    // test();
    TestEcho();
    Sunnet::inst->Wait();
    return 0;
}