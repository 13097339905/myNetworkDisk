#ifndef PROTOCOL_H
#define PROTOCOL_H

typedef unsigned int uint;

#define REGISTER_SUCCESSED      "register successed"
#define REGISTER_FAILED         "register failed : username existed"
#define LOGIN_SUCCESSED         "login successed"
#define LOGIN_FAILED            "login failed : username or password is not correct or already login"
#define SEARCH_USER_NOT_EIXST   "search user is not exist"
#define SEARCH_USER_ONLINE      "search user is online"
#define SEARCH_USER_NOT_ONLINE  "search user is not online"
#define ALREADY_IS_FRIEND       "already is friend, can't repeat add"
#define FRIEND_NOT_ONLINE       "friend not online, please try to chat later"
#define CREATE_FOLDER_EXIST     "folder already exist, create failed"
#define CREATE_FOLDER_SUCCESS   "folder create success"


enum class ENUM_MSG_TYPE
{
    ENUM_MSG_TYPE_MIN = 0,             // 最小值，用于边界检查
    ENUM_MSG_TYPE_REGISTER_REQUEST,    // 注册请求
    ENUM_MSG_TYPE_REGISTER_RESPOND,    // 注册回复

    ENUM_MSG_TYPE_LOGIN_REQUEST,       // 登录请求
    ENUM_MSG_TYPE_LOGIN_RESPOND,       // 登录回复

    ENUM_MSG_TYPE_SELECT_ONLINE_USER_REQUEST,  // 查询在线用户请求
    ENUM_MSG_TYPE_SELECT_ONLINE_USER_RESPOND,  // 查询在线用户回复

    ENUM_MSG_TYPE_SEARCH_USER_REQUEST,         // 查询用户请求
    ENUM_MSG_TYPE_SEARCH_USER_RESPOND,         // 查询用户回复

    ENUM_MSG_TYPE_ADD_FRIEND_REQUEST,          // 添加好友请求
    ENUM_MSG_TYPE_ADD_FRIEND_RESPOND,          // 添加好友回复

    ENUM_MSG_TYPE_ADD_FRIEND_AGREE,         // 同意添加好友消息
    ENUM_MSG_TYPE_ADD_FRIEND_REFUSE,        // 拒绝添加好友消息

    ENUM_MSG_TYPE_SELECT_FRIEND_REQUEST,    // 查询所有好友请求
    ENUM_MSG_TYPE_SELECT_FRIEND_RESPOND,    // 查询所有好友回复

    ENUM_MSG_TYPE_DELETE_FRIEND_REQUEST,    // 查询所有好友请求
    ENUM_MSG_TYPE_DELETE_FRIEND_RESPOND,    // 查询所有好友回复

    ENUM_MSG_TYPE_PRIVATE_CHAT_REQUEST,    // 私聊好友请求
    ENUM_MSG_TYPE_PRIVATE_CHAT_RESPOND,    // 私聊好友回复

    ENUM_MSG_TYPE_GROUP_CHAT_REQUEST,      // 群聊好友请求

    ENUM_MSG_TYPE_CREATE_FOLDER_REQUEST,    // 创建新文件夹请求
    ENUM_MSG_TYPE_CREATE_FOLDER_RESPOND,    // 创建新文件夹回复

    ENUM_MSG_TYPE_MAX = 100,           // 选一个uint内的最大值，用于边界检查
};


struct PDU
{
    uint uiPDULen;       // 整个PDU的总长度，包括头部和数据部分
    uint uiMsgType;      // 消息类型，用于标识不同的操作（如登录、注册、文件传输等）
    char caData[64];     // 固定长度的数据区域，可用于存储用户名、文件名等固定长度信息
    uint uiMsgLen;       // 实际消息数据的长度，即caMsg部分的长度
    char caMsg[];         // 弹性数组，用于存储实际的消息数据
};

// 开辟一块PDU内存，大小为uiMsgLen + sizeof(PDU)
PDU* makePDU(uint uiMsgLen);

#endif // PROTOCOL_H
