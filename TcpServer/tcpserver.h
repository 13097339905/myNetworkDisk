#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class TcpServer; }
QT_END_NAMESPACE

class TcpServer : public QWidget
{
    Q_OBJECT

public:
    TcpServer(QWidget *parent = nullptr);
    ~TcpServer();
    void loadConfig();

private:
    Ui::TcpServer *ui;
    QString m_strIP;        // 读取到的ip，存储到m_strIP
    quint16 m_usPort;       // 读取的端口号，存储到m_usPort，quint16，无符号16位，2个字节，就是unsigned short
};
#endif // TCPSERVER_H
