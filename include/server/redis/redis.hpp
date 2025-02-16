#ifndef REDIS_H
#define REDIS_H

#include <hiredis/hiredis.h>
#include <thread>
#include <functional>

class Redis
{
public:
    Redis();
    ~Redis();

    //连接redis服务器
    bool connect();
    //向redis指定通道channel发布消息
    bool publish(int channel,std::string message);
    //向redis指定通道subcribe消息
    bool subscribe(int channel);
    //取消订阅
    bool unsubscribe(int channel);
    //在独立线程接收消息
    void observer_channel_message();
    //初始化回调
    void init_notify_handler(std::function<void(int,std::string)> fn);
    
private:
    //redis同步上下文对象，负责publish消息
    redisContext* _publish_context;
    //redis同步上下文对象,负责subcribe消息
    redisContext* _subcribe_context;
    //回调，收到订阅消息，给service上报 
    std::function<void(int,std::string)> _notify_message_handler;
};

#endif