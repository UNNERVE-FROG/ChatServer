#include "chatserver.hpp"
#include "json.hpp"
#include <functional>
#include "chatservice.hpp"
using namespace std;
using namespace placeholders;
using namespace nlohmann;
ChatServer::ChatServer(muduo::net::EventLoop *loop,
            const muduo::net::InetAddress& listenAddr,
            const std::string& nameArg):_server(loop,listenAddr,nameArg),_loop(loop)
            {
                _server.setConnectionCallback(bind(&ChatServer::onConnection,this,_1));

                _server.setMessageCallback(bind(&ChatServer::onMessage,this,_1,_2,_3));

                _server.setThreadNum(4);
            }

void ChatServer::start() 
{
    _server.start();
}

void ChatServer::onConnection(const muduo::net::TcpConnectionPtr& conn)
{   
    //客户端断开连接
    if (!conn->connected()) {
        ChatService::instance()->clientCloseException(conn);
        conn->shutdown();
    }
}
void ChatServer::onMessage(const muduo::net::TcpConnectionPtr& conn,muduo::net::Buffer* buffer,muduo::Timestamp time)
{
    string buf = buffer->retrieveAllAsString();
    json js = json::parse(buf);
    //获取ChatService实例
    auto msgHandler = ChatService::instance()->gethandler(js["msgid"].get<int>());
    //回调消息绑定好的消息处理器
    msgHandler(conn,js,time);
}
