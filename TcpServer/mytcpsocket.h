#ifndef MYTCPSOCKET_H
#define MYTCPSOCKET_H
#include <QTcpSocket>

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
};

#endif // MYTCPSOCKET_H
