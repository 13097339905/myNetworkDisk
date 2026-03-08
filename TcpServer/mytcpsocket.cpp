#include "mytcpsocket.h"
#include <QDebug>
#include "operatedb.h"
#include "mytcpserver.h"

MyTcpSocket::MyTcpSocket()
{
    connect(this, &MyTcpSocket::readyRead, this, &MyTcpSocket::recvMsg);

    connect(this, &MyTcpSocket::disconnected, this, [this](){
        OperateDB::getInstance().updateOnline(m_username);    // 改变在线状态
        emit logout(this);
    });
}

QString MyTcpSocket::getUsername()
{
    return m_username;
}

// 处理客户端发过来的用户注册请求
void MyTcpSocket::handleRegisterRequest(PDU* pdu)
{
    char username[32];
    char password[32];
    strncpy(username, pdu->caData, 32);
    strncpy(password, pdu->caData + 32, 32);
    PDU* respondPdu = makePDU(0);
    respondPdu->uiMsgType = static_cast<uint>(ENUM_MSG_TYPE::ENUM_MSG_TYPE_REGISTER_RESPOND);

    if (OperateDB::getInstance().insertUserInfo(username, password))   // 插入成功
    {
        strcpy(respondPdu->caData, REGISTER_SUCCESSED);             // 向客户端发送成功的消息
    }
    else
    {
        strcpy(respondPdu->caData, REGISTER_FAILED);                // 向客户端发送失败的消息
    }
    this->write((char*)respondPdu, respondPdu->uiPDULen);
    free(respondPdu);
    respondPdu = nullptr;
}

// 处理客户端发过来的用户登录请求
void MyTcpSocket::handleLoginRequest(PDU* pdu)
{
    char username[32];
    char password[32];
    strncpy(username, pdu->caData, 32);
    strncpy(password, pdu->caData + 32, 32);
    PDU* loginPDU = makePDU(0);
    loginPDU->uiMsgType = static_cast<uint>(ENUM_MSG_TYPE::ENUM_MSG_TYPE_LOGIN_RESPOND);

    if (OperateDB::getInstance().selectUserInfo(username, password))
    {
        OperateDB::getInstance().updateOnline(username);
        strcpy(loginPDU->caData, LOGIN_SUCCESSED);
        m_username = username;       // 保存当前登录的用户名到socket
    }
    else
    {
        strcpy(loginPDU->caData, LOGIN_FAILED);
    }
    this->write((char*)loginPDU, loginPDU->uiPDULen);
    free(loginPDU);
    loginPDU = nullptr;
}

// 处理客户端发过来的查询所有在线用户请求
void MyTcpSocket::handleSelectOnlineRequest()
{
    QString onlineUser = OperateDB::getInstance().selectOnlineUser();
    PDU* selectOnlineUserPDU = makePDU(onlineUser.size() + 1);   // 初始化查询结果返回去的PDU
    selectOnlineUserPDU->uiMsgType = static_cast<uint>(ENUM_MSG_TYPE::ENUM_MSG_TYPE_SELECT_ONLINE_USER_RESPOND);  // 设置消息类型

    strcpy((char*)selectOnlineUserPDU->caMsg, onlineUser.toStdString().c_str());    // 将查询好的结果存到消息里面
    this->write((char*)selectOnlineUserPDU, selectOnlineUserPDU->uiPDULen);         // 将查询结果封装到PDU后发送回客户端
    free(selectOnlineUserPDU);
    selectOnlineUserPDU = nullptr;
}

// 处理客户端发过来的查询用户请求
void MyTcpSocket::handleSearchUserRequest(PDU* pdu)
{
    QString username;
    username = QString(pdu->caData);      // 获取用户发过来的要查询的用户名
    qDebug() << username;

    int res = OperateDB::getInstance().searchUser(username);    // 到数据库查询该用户得到结果

    PDU* searchUserPDU = makePDU(0);      // 初始化回复的PDU
    searchUserPDU->uiMsgType = static_cast<uint>(ENUM_MSG_TYPE::ENUM_MSG_TYPE_SEARCH_USER_RESPOND);
    if (res == USER_NOT_EXIST)            // 根据查询结果设置回复的PDU
    {
        strcpy(searchUserPDU->caData, SEARCH_USER_NOT_EIXST);
    }
    else if (res == USER_NOT_ONLINE)
    {
        strcpy(searchUserPDU->caData, SEARCH_USER_NOT_ONLINE);
    }
    else if (res == USER_IS_ONLINE)
    {
        strcpy(searchUserPDU->caData, SEARCH_USER_ONLINE);
    }
    this->write((char*)searchUserPDU, searchUserPDU->uiPDULen);    // 向客户端发出回复
    free(searchUserPDU);
    searchUserPDU = nullptr;
}

// 处理客户端发过来的添加好友请求
void MyTcpSocket::handleAddFriendRequest(PDU* pdu)
{
    char myUsername[32];
    char username[32];
    strncpy(myUsername, pdu->caData, 32);      // 获取发送过来的当前用户名
    strncpy(username, pdu->caData + 32, 32);   // 和想要加的用户名

    // 到数据库里面去查询
    int res = OperateDB::getInstance().addFriendSearch(myUsername, username);
    PDU* addFriendPDU = makePDU(0);
    addFriendPDU->uiMsgType = static_cast<uint>(ENUM_MSG_TYPE::ENUM_MSG_TYPE_ADD_FRIEND_RESPOND);

    // 1.已经是好友的情况，客户端直接弹出已经是好友的弹出窗口
    if (res == EXIST_FRIEND)
    {
        strcpy(addFriendPDU->caData, ALREADY_IS_FRIEND);
    }
    // 2.想加的用户不在线，客户端直接弹出相加的好友不在线的窗口
    if (res == USER_NOT_ONLINE)
    {
        strcpy(addFriendPDU->caData, SEARCH_USER_NOT_ONLINE);
    }
    // 3.想加的用户在线，被加的客户端弹出窗口，是否同意加好友
    if (res == USER_IS_ONLINE)
    {
        MyTcpServer::getInstance().forwardPDU(pdu, username);   // 转发给想添加的用户
    }
    this->write((char*)addFriendPDU, addFriendPDU->uiPDULen);
    free(addFriendPDU);
    addFriendPDU = nullptr;
}

// 处理客户端发过来的同意添加好友
void MyTcpSocket::handleAgreeRequest(PDU* pdu)
{
    // 先将数据插进去，再转发消息
    char myUsername[32];
    char username[32];

    strncpy(myUsername, pdu->caData, 32);
    strncpy(username, pdu->caData + 32, 32);
//    qDebug() << 2 << myUsername << username;
    if(!OperateDB::getInstance().insertFriendInfo(myUsername, username))   // 插入friendInfo里面去
        return;    // 插入失败

    // 转发消息
    MyTcpServer::getInstance().forwardPDU(pdu, myUsername);    // 转发给申请加好友的用户申请结果
}

// 处理客户端发过来的拒绝添加好友
void MyTcpSocket::handleRefuseRequest(PDU* pdu)
{
    // 直接转发消息
    char myUsername[32];
    strncpy(myUsername, pdu->caData, 32);
    MyTcpServer::getInstance().forwardPDU(pdu, myUsername);    // 转发给申请加好友的用户申请结果
}


void MyTcpSocket::recvMsg()
{
//    qDebug() << this->bytesAvailable();
    uint uiPDULen = 0;
    this->read((char*)&uiPDULen, sizeof(uint));    // 先读取总长度4个字节出来，到uiPDULen
    uint uiMsglen = uiPDULen - sizeof(PDU);        // 得到消息长度
    PDU* pdu = makePDU(uiMsglen);                  // 根据消息长度构造出pdu

    // (char*)pdu + sizeof(uint):前四个字节之前已经读完了，所以要读到pud偏移之前读的位置上
    // uiPDULen - sizeof(uint):之前读的不用读了，所以要减去
    this->read((char*)pdu + sizeof(uint), uiPDULen - sizeof(uint));
    // qDebug() << pdu->uiMsgType << username << password;

    switch (pdu->uiMsgType)
    {
    case static_cast<uint>(ENUM_MSG_TYPE::ENUM_MSG_TYPE_REGISTER_REQUEST):   // 注册请求
        handleRegisterRequest(pdu);
        break;

    case static_cast<uint>(ENUM_MSG_TYPE::ENUM_MSG_TYPE_LOGIN_REQUEST):     // 登录请求
        handleLoginRequest(pdu);
        break;

    case static_cast<uint>(ENUM_MSG_TYPE::ENUM_MSG_TYPE_SELECT_ONLINE_USER_REQUEST):   // 查询在线用户请求
        handleSelectOnlineRequest();
        break;

    case static_cast<uint>(ENUM_MSG_TYPE::ENUM_MSG_TYPE_SEARCH_USER_REQUEST):    // 查找用户请求
        handleSearchUserRequest(pdu);
        break;

    case static_cast<uint>(ENUM_MSG_TYPE::ENUM_MSG_TYPE_ADD_FRIEND_REQUEST):     // 添加好友请求
        handleAddFriendRequest(pdu);
        break;

    case static_cast<uint>(ENUM_MSG_TYPE::ENUM_MSG_TYPE_ADD_FRIEND_AGREE):      // 同意添加好友
        handleAgreeRequest(pdu);
        break;

    case static_cast<uint>(ENUM_MSG_TYPE::ENUM_MSG_TYPE_ADD_FRIEND_REFUSE):     // 拒绝好友请求
        handleRefuseRequest(pdu);
        break;


    default:
        break;
    }
    free(pdu);
    pdu = nullptr;
}
