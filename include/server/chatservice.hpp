#ifndef CHATSERVICE_H
#define CHATSERVICE_H
#include <map>
#include <functional>
#include <muduo/net/TcpConnection.h>
#include <json.hpp>
#include <mutex>
#include "usermodel.hpp"
#include "offlinemessagemodel.hpp"
#include "friendmodel.hpp"
#include "group.hpp"
#include "groupmodel.hpp"
#include "redis.hpp"

//处理消息的事件回调
using MsgHandler = std::function<void(const muduo::net::TcpConnectionPtr &conn,nlohmann::json &js, muduo::Timestamp time)>;
class ChatService
{
private:
    ChatService(/* args */);
    //msgid和消息处理方法
    std::unordered_map<int,MsgHandler> _msgHandlerMap; 
    UserModel _userModel;
    //存储在线用户的连接
    std::unordered_map<int,muduo::net::TcpConnectionPtr> _userConnMap;
    //互斥锁，保证_userConnMap线程安全
    std::mutex _connMutex;
    OfflineMessageModel _offlineMsgModel;
    FriendModel _friendModel;
    GroupModel _groupModel;
    Redis _redis;
public:
    //登录
    void login(const muduo::net::TcpConnectionPtr &conn,nlohmann::json &js, muduo::Timestamp time);
    //注册
    void reg(const muduo::net::TcpConnectionPtr &conn,nlohmann::json &js, muduo::Timestamp time);
    static ChatService* instance();
    MsgHandler gethandler(int msgid);
    void clientCloseException(const muduo::net::TcpConnectionPtr &conn);
    void oneChat(const muduo::net::TcpConnectionPtr &conn,nlohmann::json &js, muduo::Timestamp time);
    //重置s
    void reset();

    void addFriend(const muduo::net::TcpConnectionPtr &conn,nlohmann::json &js, muduo::Timestamp time);

    bool createGroup(const muduo::net::TcpConnectionPtr &conn,nlohmann::json &js, muduo::Timestamp time);
    void addGroup(const muduo::net::TcpConnectionPtr &conn,nlohmann::json &js, muduo::Timestamp time);
    void chatGroup(const muduo::net::TcpConnectionPtr &conn,nlohmann::json &js, muduo::Timestamp time);
    void loginout(const muduo::net::TcpConnectionPtr &conn,nlohmann::json &js, muduo::Timestamp time);
    void handleRedisSubscribeMessage(int,std::string);
};



#endif