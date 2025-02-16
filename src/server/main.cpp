#include "chatserver.hpp"
#include "chatservice.hpp"
#include <iostream>
#include <signal.h>
using namespace std;
using namespace muduo;
using namespace muduo::net;


void resetHandler(int)
{
    ChatService::instance()->reset();
    exit(0);
}

int main(int argc,char* argv[])
{
    signal(SIGINT,resetHandler);
    EventLoop loop;
    if (argc <= 2){
        printf("argv invailed!");
    }

    InetAddress addr(argv[1],atoi(argv[2]));
    ChatServer server(&loop,addr,"ChatServer");

    server.start();
    loop.loop();

    return 0;
}