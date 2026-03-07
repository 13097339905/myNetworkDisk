#include "mytcpserver.h"
#include <QDebug>

MyTcpServer::MyTcpServer()
{

}

MyTcpServer &MyTcpServer::getInstance()
{
    static MyTcpServer instance;
    return instance;
}

// 是QTcpServer类中的一个虚函数，当有新的客户端连接请求到达时，会被自动调用。
void MyTcpServer::incomingConnection(qintptr socketDescriptor)
{
    qDebug() << "new client connected";
    MyTcpSocket* pTcpSocket = new MyTcpSocket;
    pTcpSocket->setSocketDescriptor(socketDescriptor);
    m_tcpSocketList.append(pTcpSocket);

    connect(pTcpSocket, &MyTcpSocket::logout, this, &MyTcpServer::deleteSocket);    // 删掉socket
}

void MyTcpServer::deleteSocket(MyTcpSocket *mySocket)
{
    for (QList<MyTcpSocket*>::iterator it = m_tcpSocketList.begin(); it != m_tcpSocketList.end(); it++)
    {
        if (*it == mySocket)
        {
            qDebug() << mySocket->getUsername();
            mySocket -> deleteLater();   // 延迟释放空间，使用delete会报错！！！
            mySocket = nullptr;
            m_tcpSocketList.erase(it);
            break;
        }
    }
}
