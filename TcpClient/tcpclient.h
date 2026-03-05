#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <QWidget>
#include <QFile>      // 用来读取配置文件中的连接信息，IP、端口号

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

private:
    Ui::TcpClient *ui;
    QString m_strIP;        // 读取到的ip，存储到m_strIP
    quint16 m_usPort;       // 读取的端口号，存储到m_usPort，quint16，无符号16位，2个字节，就是unsigned short
};
#endif // TCPCLIENT_H
