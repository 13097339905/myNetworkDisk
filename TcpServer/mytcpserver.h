#ifndef MYTCPSERVER_H
#define MYTCPSERVER_H
#include <QTcpServer>
#include <QList>
#include "mytcpsocket.h"

// MyTcpServer类继承QTcpServer，并且MyTcpServer使用单例模式
class MyTcpServer : public QTcpServer
{
    Q_OBJECT
private:
    MyTcpServer();
    QList<MyTcpSocket*> m_tcpSocketList;

public:
    static MyTcpServer& getInstance();

    void incomingConnection(qintptr socketDescriptor);
};

#endif // MYTCPSERVER_H
