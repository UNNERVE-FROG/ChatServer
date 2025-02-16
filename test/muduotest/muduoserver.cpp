#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <iostream>
#include <functional>
#include <string>

using namespace std;

/*
1.组合TcpServer对象
2.创建EventLoop指针
3.创建TcpServer需要的参数
4.注册处理连接的回调函数和处理读写事件的回调函数
5.设置服务器线程数量，muduo自己分配io线程和业务线程数
*/
class ChatServer {
public:
    ChatServer(muduo::net::EventLoop* loop,  //事件循环
            const muduo::net::InetAddress& listenAddr,  //ip + port
            const string& nameArg)  //服务器名字
            :_server(loop,listenAddr,nameArg),_loop(loop)
            {
                //给服务器注册创建断开连接的回调
                _server.setConnectionCallback(bind(&ChatServer::onConnection,this,placeholders::_1));
                //给服务器注册用户读写的回调
                _server.setMessageCallback(bind(&ChatServer::onMessage,this,placeholders::_1,placeholders::_2,placeholders::_3));
                //设置服务器端的线程数量
                _server.setThreadNum(4);
            }
    ~ChatServer() {
        
    }
    void start() {
        _server.start();
    }
private:
    void onConnection(const muduo::net::TcpConnectionPtr &conn) {
        if (conn->connected()) {
            cout << conn->peerAddress().toIpPort() << "->" << conn->localAddress().toIpPort()<< " online" << endl;
        } else {
            cout << conn->peerAddress().toIpPort() << "->" << conn->localAddress().toIpPort()<< " offline" << endl;
            conn->shutdown();
        }
        
    }

    void onMessage(const muduo::net::TcpConnectionPtr& conn,
                            muduo::net::Buffer* buffer,
                            muduo::Timestamp time) 
    {
        string buf = buffer->retrieveAllAsString();
        cout << "recv data:" << buf << "time:" << time.toString() <<endl;
        conn->send(buf);
    }
    muduo::net::TcpServer _server;
    muduo::net::EventLoop *_loop;
};


int main() {
    muduo::net::EventLoop loop;
    muduo::net::InetAddress addr("127.0.0.1",6000);
    ChatServer server(&loop,addr,"ChatServer");
    server.start();  //listenfd epoll_ctl
    loop.loop();    //epoll_wait

    return 0;
}