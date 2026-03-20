#ifndef MYTCPSOCKET_H
#define MYTCPSOCKET_H
#include <QTcpSocket>
#include "protocol.h"
#include <QDir>     // 专门用来操作文件目录的

#define ROOT_PATH   "F:/C++/myNetworkDisk_file"

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

    bool m_isTransferData;  // 是否是传输数据模式

    qint64 m_fileSize;      // 要上传的文件大小

    QByteArray m_totalData;    // 暂存数据，等数据完全接收

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
    void handleSelectFriendRequest();

    // 处理删除好友的消息
    void handleDeleteFriendRequest(PDU* pdu);

    // 处理私聊请求
    void handlePrivateChatRequest(PDU* pdu);

    // 处理群发请求
    void handleGroupChatRequest(PDU* pdu);

    // 处理创建文件夹请求
    void handleCreateFolderRequest(PDU* pdu);

    // 处理刷新文件请求
    void handleFlushFileRequest(PDU* pdu);

    // 处理删除文件的请求
    void handleDeleteFileRequest(PDU* pdu);

    // 处理重命名文件的请求
    void handleRenameFileRequest(PDU* pdu);

    // 处理进入文件夹的请求
    void handleEnterFolderRequest(PDU* pdu);

    // 处理返回上一级的请求
    void handleReturnPreFolderRequest(PDU* pdu);

    // 处理上传文件请求
    void handleUploadFileRequest(PDU* pdu);

    // 处理传输数据请求
    void handleTransferDataRequest(PDU* pdu);
};

#endif // MYTCPSOCKET_H
