#ifndef MYTCPSERVER_H
#define MYTCPSERVER_H
#include <QTcpServer>
#include <QList>
#include "mytcpsocket.h"

// MyTcpServer类继承QTcpServer，并且MyTcpServer使用单例模式
// 服务器应该是全局唯一的，整个应用程序只需要一个服务器实例来监听连接，所以需要使用单例模式
class MyTcpServer : public QTcpServer
{
    Q_OBJECT
private:
    MyTcpServer();      // 构造函数私有化
    MyTcpServer(const MyTcpServer&) = delete;    // 拷贝构造函数私有化或者delete掉
    MyTcpServer& operator=(const MyTcpServer&) = delete;   // 赋值重载也私有化或者delete掉
    QList<MyTcpSocket*> m_tcpSocketList;

public:
    static MyTcpServer& getInstance();

    // 是QTcpServer类中的一个虚函数，当有新的客户端连接请求到达时，会被自动调用
    void incomingConnection(qintptr socketDescriptor);
};

#endif // MYTCPSERVER_H
