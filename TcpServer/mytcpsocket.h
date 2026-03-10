#ifndef MYTCPSOCKET_H
#define MYTCPSOCKET_H
#include <QTcpSocket>
#include "protocol.h"

class MyTcpSocket : public QTcpSocket
{
    Q_OBJECT
public:
    MyTcpSocket();

    QString getUsername();

signals:
    void logout(MyTcpSocket* mySocket);


public slots:
    void recvMsg();

private:
    QString m_username;   // 当前socket登录的用户

    // 处理客户端发过来的用户注册请求
    void handleRegisterRequest(PDU* pdu);

    // 处理客户端发过来的用户登录请求
    void handleLoginRequest(PDU* pdu);

    // 处理客户端发过来的查询所有在线用户请求
    void handleSelectOnlineRequest();

    // 处理客户端发过来的查询用户请求
    void handleSearchUserRequest(PDU* pdu);

    // 处理客户端发过来的添加好友请求
    void handleAddFriendRequest(PDU* pdu);

    // 处理客户端发过来的同意添加好友
    void handleAgreeRequest(PDU* pdu);

    // 处理客户端发过来的拒绝添加好友
    void handleRefuseRequest(PDU* pdu);

    // 处理查询当前用户所有好友信息
    void handleSelectFriend();

    // 处理删除好友的消息
    void handleDeleteFriend(PDU* pdu);
};

#endif // MYTCPSOCKET_H
