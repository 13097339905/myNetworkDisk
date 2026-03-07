#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <QWidget>
#include <QTcpSocket>     // 用来连接服务器

QT_BEGIN_NAMESPACE
namespace Ui { class TcpClient; }
QT_END_NAMESPACE

class TcpClient : public QWidget
{
    Q_OBJECT

public:
    TcpClient(QWidget *parent = nullptr);
    ~TcpClient();

    // 读取配置文件中的信息
    void loadConfig();

private slots:
//    // 点击发送按钮后进行数据发送
//    void on_sendQPushButton_clicked();

    void on_registerPushButton_clicked();

    void on_cancellationPushButton_clicked();


    void recvMsg();

    void on_loginPushButtone_clicked();

private:
    Ui::TcpClient *ui;
    QString m_strIP;        // 读取到的ip，存储到m_strIP
    quint16 m_usPort;       // 读取的端口号，存储到m_usPort，quint16，无符号16位，2个字节，就是unsigned short
    QTcpSocket m_tcpSocket; // 连接服务器，与服务器进行数据交互

};
#endif // TCPCLIENT_H
