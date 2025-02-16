#include "chatservice.hpp"
#include "public.hpp"
#include <muduo/base/Logging.h>
#include <string>
#include <vector>
#include <map>
using namespace std;
using namespace nlohmann;

//单例
ChatService* ChatService::instance()
{
    static ChatService service;
    return &service;
}

//注册消息与回调
ChatService::ChatService()
{
    _msgHandlerMap.insert({LOGIN_MSG,std::bind(&ChatService::login,this,placeholders::_1,placeholders::_2,placeholders::_3)});
    _msgHandlerMap.insert({REG,std::bind(&ChatService::reg,this,placeholders::_1,placeholders::_2,placeholders::_3)});
    _msgHandlerMap.insert({ONE_CHAT_MSG,std::bind(&ChatService::oneChat,this,placeholders::_1,placeholders::_2,placeholders::_3)});
    _msgHandlerMap.insert({ADD_FRIEND_MSG,bind(&ChatService::addFriend,this,placeholders::_1,placeholders::_2,placeholders::_3)});
    _msgHandlerMap.insert({LOGINOUT_MSG,bind(&ChatService::loginout,this,placeholders::_1,placeholders::_2,placeholders::_3)});

    if (_redis.connect())
    {
        //注册回调
        _redis.init_notify_handler(bind(&ChatService::handleRedisSubscribeMessage,this,placeholders::_1,placeholders::_2));
    }
}

//处理登录业务
void ChatService::login(const muduo::net::TcpConnectionPtr &conn,nlohmann::json &js, muduo::Timestamp time)
{
    LOG_INFO << "do login service!";
    int id = js["id"].get<int>();
    string password = js["password"];
    User user = _userModel.query(id);
    if (user.getId()!= -1 && user.getPwd() == password) {
        if (user.getState() == "online")
        {
            json response;
            response["msgid"] = LOG_MSG_ACK;
            response["errno"] = 1;
            response["errmsg"] = "该账号已登陆，请重新输入新账号";
            conn->send(response.dump());
        }
        else 
        {
            LOG_INFO << "登录成功！";
            //更新用户状态信息
            {
                lock_guard<mutex> lock(_connMutex);
                _userConnMap.insert({id,conn});   
            }
            _redis.subscribe(id);
            user.setState("online");
            _userModel.updateState(user);

            json response;
            response["msgid"] = LOG_MSG_ACK;
            response["errno"] = 0;
            response["id"] = user.getId();
            response["name"] = user.getName();
            //查询用户是否有离线数据
            vector<string> vec = _offlineMsgModel.query(id);
            if (!vec.empty())
            {
                response["offlinemsg"] = vec;
                //清楚离线消息
                _offlineMsgModel.remove(user.getId());
            }
            vector<User> userVec = _friendModel.query(id);
            LOG_INFO << userVec.size();
            if (!userVec.empty())
            {   
                vector<string> vec2;
                for (auto &user : userVec)
                {
                    json js;
                    js["id"] = user.getId();
                    js["name"] = user.getName();
                    js["state"] = user.getState();
                    vec2.push_back(js.dump());
                }
                response["friends"] = vec2;
            }
            conn->send(response.dump());
        }
    }
    else
    {
        LOG_INFO << "登录失败！";
        json response;
        response["msgid"] = LOG_MSG_ACK;
        response["errno"] = 1;
        response["errmsg"] = "用户名或密码不存在";
        conn->send(response.dump());
    }
}
//处理注册业务
void ChatService::reg(const muduo::net::TcpConnectionPtr &conn,nlohmann::json &js, muduo::Timestamp time)
{
    LOG_INFO << "do reg service!";
    string name = js["name"];
    string pwd = js["password"];
    User user;
    user.setName(name);
    user.setPwd(pwd);
    bool state = _userModel.insert(user);
    if (state) 
    {
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["errno"] = 0;
        response["id"] = user.getId();
        conn->send(response.dump());

    }
    else 
    {
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["errno"] = 1;
        conn->send(response.dump());
    }
}


MsgHandler ChatService::gethandler(int msgid) 
{
    auto it = _msgHandlerMap.find(msgid);
    if (it == _msgHandlerMap.end()) 
    {
        //返回一个默认的处理器
        return [=](const muduo::net::TcpConnectionPtr &conn,nlohmann::json &js, muduo::Timestamp time) {
            LOG_ERROR << "msgid:" << msgid << "can not find handler!";
        };
    } 
    else 
    {
        return _msgHandlerMap[msgid]; 
    }
}

void ChatService::clientCloseException(const muduo::net::TcpConnectionPtr &conn) 
{
    User user;
    {
        lock_guard<mutex> lock(_connMutex);
        for (auto it = _userConnMap.begin();it != _userConnMap.end(); it++) 
        {
            if (it->second == conn)
            {   
                user.setId(it->first);
                _userConnMap.erase(it);
                break;
            }
        }
    }
    _redis.unsubscribe(user.getId());
    if (user.getId() != -1) 
    {
        user.setState("offline");
        _userModel.updateState(user);
    }
}


void ChatService::oneChat(const muduo::net::TcpConnectionPtr &conn,nlohmann::json &js, muduo::Timestamp time)
{
    int toid = js["to"].get<int>();
    {
        lock_guard<mutex> lock(_connMutex);
        auto it = _userConnMap.find(toid);
        if (it != _userConnMap.end())
        {
            //在线消息   服务器推送消息
            it->second->send(js.dump());
            return;
        }
    }
    //查询toid是否在线
    User user = _userModel.query(toid);
    if (user.getState() == "online")
    {
        _redis.publish(toid,js.dump());
        return;
    }
    //离线消息
    _offlineMsgModel.insert(toid,js.dump());
}
//重置用户状态信息
void ChatService::reset()
{
    _userModel.resetState();
}


void ChatService::addFriend(const muduo::net::TcpConnectionPtr &conn,nlohmann::json &js, muduo::Timestamp time)
{
    int userid = js["id"].get<int>();
    int friendid = js["friendid"].get<int>();
    //存储好友信息
    _friendModel.insert(userid,friendid);

}

//创建群组
bool ChatService::createGroup(const muduo::net::TcpConnectionPtr &conn,nlohmann::json &js, muduo::Timestamp time)
{
    int userid = js["id"].get<int>();
    string name = js["groupname"];
    string desc = js["groupdesc"];
    Group group(-1,name,desc);
    if (_groupModel.createGroup(group))
    {
        _groupModel.addGroup(userid,group.getId(),"creator");
        return true;
    }
    return false;
}


//加入群组业务
void ChatService::addGroup(const muduo::net::TcpConnectionPtr &conn,nlohmann::json &js, muduo::Timestamp time)
{
    int userid = js["id"].get<int>();
    int groupid = js["groupid"].get<int>();
    _groupModel.addGroup(userid,groupid,"normal");
}


void ChatService::chatGroup(const muduo::net::TcpConnectionPtr &conn,nlohmann::json &js, muduo::Timestamp time)
{
    int userid = js["id"].get<int>();
    int groupid = js["groupid"].get<int>();
    vector<int> useridVec = _groupModel.queryGroupUsers(userid,groupid);
    lock_guard<mutex> lock(_connMutex);
    for (auto id : useridVec)
    {
        auto it = _userConnMap.find(id);
        if (it != _userConnMap.end())
        {
            it->second->send(js.dump());
        }
        else 
        {  
            User user = _userModel.query(id);
            if (user.getState() == "online")
            {
                _redis.publish(id,js.dump());
            } 
            else
            {
                _offlineMsgModel.insert(id,js.dump());
            }
        }
    }
}

void ChatService::loginout(const muduo::net::TcpConnectionPtr &conn,nlohmann::json &js, muduo::Timestamp time)
{
    int userid = js["id"].get<int>();
    {
        lock_guard<mutex> lock(_connMutex);
        auto it = _userConnMap.find(userid);
        if (it != _userConnMap.end())
        {
            _userConnMap.erase(it);
        }
    }
    _redis.unsubscribe(userid);
    User user(userid,"","","offline");
    _userModel.updateState(user);

}

void ChatService::handleRedisSubscribeMessage(int userid,string msg)
{
    lock_guard<mutex> lock(_connMutex);
    auto it = _userConnMap.find(userid);
    if (it != _userConnMap.end())
    {
        it->second->send(msg);
        return;
    }
    //存储离线消息
    _offlineMsgModel.insert(userid,msg);
}