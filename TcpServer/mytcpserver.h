#ifndef MYTCPSERVER_H
#define MYTCPSERVER_H
#include <QTcpServer>

// MyTcpServer类继承QTcpServer，并且MyTcpServer使用单例模式
class MyTcpServer : public QTcpServer
{
    Q_OBJECT
private:
    MyTcpServer();
public:
    static MyTcpServer& getInstance();

    void incomingConnection(qintptr socketDescriptor);
};

#endif // MYTCPSERVER_H
