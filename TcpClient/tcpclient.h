#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <QWidget>
#include <QTcpSocket>     // 用来连接服务器
#include "protocol.h"

QT_BEGIN_NAMESPACE
namespace Ui { class TcpClient; }
QT_END_NAMESPACE

class TcpClient : public QWidget
{
    Q_OBJECT

private:
    TcpClient(QWidget *parent = nullptr);
    ~TcpClient();
    TcpClient(const TcpClient& tc) = delete;
    TcpClient& operator=(const TcpClient& tc) = delete;

    // 读取配置文件中的信息
    void loadConfig();

    // 处理服务器发来的注册回复
    void handleRegisterRespond(PDU* pdu);

    // 处理服务器发来的登录回复
    void handleLoginRespond(PDU* pdu);

    // 处理服务器发来的查询所有在线用户的回复
    void handleSelectOnlineUserRespond(PDU* pdu);

    // 处理服务器发来的搜索用户的回复
    void handleSearchUserRespond(PDU* pdu);

    // 处理服务器发来的加好友的回复
    void handleAddFriendRespond(PDU* pdu);

    // 处理服务器转发的的加好友的请求
    void handleAddFriendRequest(PDU* pdu);

    // 处理同意加好友的情况
    void handleAgreeFriend(PDU* pdu);

    // 处理不同意加好友的情况
    void handleRefuseFriend(PDU* pdu);

    // 处理查询所有好友
    void handleSelectFriend(PDU* pdu);

    // 处理删除好友的回复
    void handleDeleteFriendRespond(PDU* pdu);

    // 处理删除好友的请求
    void handleDeleteFriendRequest(PDU* pdu);

    // 处理私聊好友的请求
    void handlePrivateChatRequest(PDU* pdu);

    // 处理私聊好友的回复
    void handlePrivateChatRespond(PDU* pdu);

public:
    // 由于需要在主菜单页面中也需要socket与服务器进行通信，但是socket之前是TcpClient的私有成员
    // 所以需要改变一下，看怎么能让mainMenu也能拿到socket进行通信
    // 可以把TcpClient也设置成单例模式，只要包含头文件就能拿到进行使用
    static TcpClient& getInstance();

    // 提供public接口给其他类获取socket
    QTcpSocket& getSocket();

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
