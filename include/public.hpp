#ifndef PUBLIC_H
#define PUBLIC_H

enum EnMsgType{
    LOGIN_MSG = 1,
    REG,
    REG_MSG_ACK, //注册响应消息
    LOG_MSG_ACK,
    ONE_CHAT_MSG,
    ADD_FRIEND_MSG,

    CREATE_GROUP_MSG,
    ADD_GROUP_MSG,
    GROUP_CHAT_MSG,

    LOGINOUT_MSG,
};

#endif