#include "mytcpsocket.h"
#include <QDebug>
#include "operatedb.h"
#include "mytcpserver.h"
#include <QDir>      // 用来操作文件目录的

MyTcpSocket::MyTcpSocket()
{
    connect(this, &MyTcpSocket::readyRead, this, &MyTcpSocket::recvMsg);

    connect(this, &MyTcpSocket::disconnected, this, [this](){
        OperateDB::getInstance().updateOnline(m_username);       // socket断开连接了，改变在线状态，发出退出登录的信号，服务器删除掉之前连接存的socket
        emit logout(this);
    });
}

QString MyTcpSocket::getUsername()
{
    return m_username;
}

// 处理客户端发过来的用户注册请求
void MyTcpSocket::handleRegisterRequest(PDU* pdu)
{
    char username[32];
    char password[32];
    strncpy(username, pdu->caData, 32);
    strncpy(password, pdu->caData + 32, 32);
    PDU* respondPdu = makePDU(0);
    respondPdu->uiMsgType = static_cast<uint>(ENUM_MSG_TYPE::ENUM_MSG_TYPE_REGISTER_RESPOND);

    if (OperateDB::getInstance().insertUserInfo(username, password))   // 插入成功
    {
        strcpy(respondPdu->caData, REGISTER_SUCCESSED);             // 向客户端发送成功的消息
        // 注册成功后为，利用改用户的username专门创建一个文件夹，用来存储该用户的东西
        QDir dir;
        if (dir.mkdir(ROOT_PATH + QString("/%1").arg(username)))
            qDebug() << "create file folder success:" + QString(ROOT_PATH) + QString("/%1").arg(username);
    }
    else
    {
        strcpy(respondPdu->caData, REGISTER_FAILED);                // 向客户端发送失败的消息
    }
    this->write((char*)respondPdu, respondPdu->uiPDULen);
    free(respondPdu);
    respondPdu = nullptr;
}

// 处理客户端发过来的用户登录请求
void MyTcpSocket::handleLoginRequest(PDU* pdu)
{
    char username[32];
    char password[32];
    strncpy(username, pdu->caData, 32);
    strncpy(password, pdu->caData + 32, 32);

    PDU* loginPDU = nullptr;    // 先声明，因为不知道登录是否成功，不知道要make一个多大的PDU

    if (OperateDB::getInstance().selectUserInfo(username, password))    // 登录成功
    {
        // 登录成功后将该用户的目录发送给用户
        QString path = ROOT_PATH + QString("/%1").arg(username);
        QByteArray ba = path.toUtf8();

        loginPDU = makePDU(ba.size());                     // 根据消息大小开辟PDU的大小
        memcpy(loginPDU->caMsg, ba.data(), ba.size());     // 设置目录消息
        OperateDB::getInstance().updateOnline(username);   // 登录成功后将用户设置为在线
        strcpy(loginPDU->caData, LOGIN_SUCCESSED);
        m_username = username;       // 保存当前登录的用户名到socket
    }
    else    // 登录失败
    {
        loginPDU = makePDU(0);
        strcpy(loginPDU->caData, LOGIN_FAILED);
    }
    loginPDU->uiMsgType = static_cast<uint>(ENUM_MSG_TYPE::ENUM_MSG_TYPE_LOGIN_RESPOND);

    this->write((char*)loginPDU, loginPDU->uiPDULen);
    free(loginPDU);
    loginPDU = nullptr;
}

// 处理客户端发过来的查询所有在线用户请求
void MyTcpSocket::handleSelectOnlineRequest()
{
    QStringList res = OperateDB::getInstance().selectOnlineUser();
    PDU* selectOnlineUserPDU = makePDU(res.size() * 32);   // 初始化查询结果返回去的PDU

    for (int i = 0; i < res.size(); i++)
    {
        strncpy(selectOnlineUserPDU->caMsg + i * 32, res[i].toStdString().c_str(), 32);    // 将查询结果放入PDU中
    }
    selectOnlineUserPDU->uiMsgType = static_cast<uint>(ENUM_MSG_TYPE::ENUM_MSG_TYPE_SELECT_ONLINE_USER_RESPOND);  // 设置消息类型

    this->write((char*)selectOnlineUserPDU, selectOnlineUserPDU->uiPDULen);         // 将查询结果封装到PDU后发送回客户端
    free(selectOnlineUserPDU);
    selectOnlineUserPDU = nullptr;
}

// 处理客户端发过来的查询用户请求
void MyTcpSocket::handleSearchUserRequest(PDU* pdu)
{
    QString username;
    username = QString(pdu->caData);      // 获取用户发过来的要查询的用户名
    //qDebug() << username;

    int res = OperateDB::getInstance().searchUser(username);    // 到数据库查询该用户得到结果

    PDU* searchUserPDU = makePDU(0);      // 初始化回复的PDU
    searchUserPDU->uiMsgType = static_cast<uint>(ENUM_MSG_TYPE::ENUM_MSG_TYPE_SEARCH_USER_RESPOND);
    if (res == USER_NOT_EXIST)            // 根据查询结果设置回复的PDU
    {
        strcpy(searchUserPDU->caData, SEARCH_USER_NOT_EIXST);
    }
    else if (res == USER_NOT_ONLINE)
    {
        strcpy(searchUserPDU->caData, SEARCH_USER_NOT_ONLINE);
    }
    else if (res == USER_IS_ONLINE)
    {
        strcpy(searchUserPDU->caData, username.toStdString().c_str());
    }
    this->write((char*)searchUserPDU, searchUserPDU->uiPDULen);    // 向客户端发出回复
    free(searchUserPDU);
    searchUserPDU = nullptr;
}

// 处理客户端发过来的添加好友请求
void MyTcpSocket::handleAddFriendRequest(PDU* pdu)
{
    char myUsername[32];
    char username[32];
    strncpy(myUsername, pdu->caData, 32);      // 获取发送过来的当前用户名
    strncpy(username, pdu->caData + 32, 32);   // 和想要加的用户名

    // 到数据库里面去查询
    int res = OperateDB::getInstance().addFriendSearch(myUsername, username);
    PDU* addFriendPDU = makePDU(0);
    addFriendPDU->uiMsgType = static_cast<uint>(ENUM_MSG_TYPE::ENUM_MSG_TYPE_ADD_FRIEND_RESPOND);

    // 1.已经是好友的情况，客户端直接弹出已经是好友的弹出窗口
    if (res == EXIST_FRIEND)
    {
        strcpy(addFriendPDU->caData, ALREADY_IS_FRIEND);
    }
    // 2.想加的用户不在线，客户端直接弹出相加的好友不在线的窗口
    if (res == USER_NOT_ONLINE)
    {
        strcpy(addFriendPDU->caData, SEARCH_USER_NOT_ONLINE);
    }
    // 3.想加的用户在线，被加的客户端弹出窗口，是否同意加好友
    if (res == USER_IS_ONLINE)
    {
        MyTcpServer::getInstance().forwardPDU(pdu, username);   // 转发给想添加的用户
    }
    this->write((char*)addFriendPDU, addFriendPDU->uiPDULen);
    free(addFriendPDU);
    addFriendPDU = nullptr;
}

// 处理客户端发过来的同意添加好友
void MyTcpSocket::handleAgreeRequest(PDU* pdu)
{
    // 先将数据插进去，再转发消息
    char myUsername[32];
    char username[32];

    strncpy(myUsername, pdu->caData, 32);
    strncpy(username, pdu->caData + 32, 32);
//    qDebug() << 2 << myUsername << username;
    if(!OperateDB::getInstance().insertFriendInfo(myUsername, username))   // 插入friendInfo里面去
        return;    // 插入失败

    // 转发消息
    MyTcpServer::getInstance().forwardPDU(pdu, myUsername);    // 转发给申请加好友的用户申请结果
}

// 处理客户端发过来的拒绝添加好友
void MyTcpSocket::handleRefuseRequest(PDU* pdu)
{
    // 直接转发消息
    char myUsername[32];
    strncpy(myUsername, pdu->caData, 32);
    MyTcpServer::getInstance().forwardPDU(pdu, myUsername);    // 转发给申请加好友的用户申请结果
}

// 处理查询当前用户所有好友信息
void MyTcpSocket::handleSelectFriendRequest()
{
    QStringList res = OperateDB::getInstance().selectFriend(m_username); // 获取数据库的查询结果
    PDU* selectFriendPDU = makePDU(res.size() * 32);                     // 新建PDU用来传输查询结果
    for (int i = 0; i < res.size(); i++)
    {
        strncpy(selectFriendPDU->caMsg + i * 32, res[i].toStdString().c_str(), 32);    // 将查询结果放入PDU中
    }
    selectFriendPDU->uiMsgType = static_cast<uint>(ENUM_MSG_TYPE::ENUM_MSG_TYPE_SELECT_FRIEND_RESPOND);   // 设置消息类型
    this->write((char*)selectFriendPDU, selectFriendPDU->uiPDULen);      // 将PDU发送给查询的客户端
    free(selectFriendPDU);        // 释放内存
    selectFriendPDU = nullptr;
}

// 处理删除好友的消息
void MyTcpSocket::handleDeleteFriendRequest(PDU* pdu)
{
    char username[32];
    memcpy(username, pdu->caData, 32);     // 获取要删除的名字
    bool tag = OperateDB::getInstance().deleteFriend(m_username, username);   // 将自己的名字和要删除的名字传进去数据库操作
    if (!tag)
    {
        return;
    }
    PDU* deleteFriendPDU = makePDU(0);
    deleteFriendPDU->uiMsgType = static_cast<uint>(ENUM_MSG_TYPE::ENUM_MSG_TYPE_DELETE_FRIEND_RESPOND);
    memcpy(deleteFriendPDU->caData, username, 32);
    this->write((char*)deleteFriendPDU, deleteFriendPDU->uiPDULen);          // 发送给请求的客户端
    MyTcpServer::getInstance().forwardPDU(pdu, username);        // 转发给被删除的

    free(deleteFriendPDU);
    deleteFriendPDU = nullptr;

}

// 处理私聊请求
void MyTcpSocket::handlePrivateChatRequest(PDU* pdu)
{
    // 将私聊请求和消息转发给私聊的客户端
    char myUsername[32];   // 请求方的用户名
    char username[32];     // 私聊对象的用户名
    memcpy(myUsername, pdu->caData, 32);
    memcpy(username, pdu->caData + 32, 32);

    PDU* privateChatPDU = makePDU(0);     // 回复给请求方的PDU
    privateChatPDU->uiMsgType = static_cast<uint>(ENUM_MSG_TYPE::ENUM_MSG_TYPE_PRIVATE_CHAT_RESPOND);
    if (OperateDB::getInstance().searchUser(username) == USER_NOT_ONLINE)   // 判断要私聊的对象不在线
    {
        strcpy(privateChatPDU->caData, FRIEND_NOT_ONLINE);      // 设置私聊的好友不在线的消息
    }
    else     // 在线的话就把消息转发给要私聊的
    {
        MyTcpServer::getInstance().forwardPDU(pdu, username);
    }
    this->write((char*)privateChatPDU, privateChatPDU->uiPDULen);   // 发送给请求方回应
    free(privateChatPDU);
    privateChatPDU = nullptr;
}

// 处理群发请求
void MyTcpSocket::handleGroupChatRequest(PDU* pdu)
{
    // 只需要将pdu转发给在线的好友用户就可以了
    char username[32];
    memcpy(username, pdu->caData, 32);     // 获取当前用户
    QStringList res = OperateDB::getInstance().selectFriend(username);  // 查询得到当前用户的在线好友
    for (int i = 0; i < res.size(); i++)
    {
        // 为每个客户端创建一个新的PDU对象，避免共享同一个PDU导致数据混乱
        PDU* newPdu = makePDU(pdu->uiMsgLen);
        memcpy(newPdu, pdu, pdu->uiPDULen);
        MyTcpServer::getInstance().forwardPDU(newPdu, res[i]);     // 转发给所有在线的好友
        free(newPdu);
        newPdu = nullptr;
    }
}

// 处理创建文件夹请求
void MyTcpSocket::handleCreateFolderRequest(PDU* pdu)
{
    char folderName[64];
    char myCurPath[pdu->uiMsgLen];
    memcpy(folderName, pdu->caData, 64);
    memcpy(myCurPath, pdu->caMsg, pdu->uiMsgLen);

    QDir dir;
    PDU* createFolderPDU = makePDU(0);
    createFolderPDU->uiMsgType = static_cast<uint>(ENUM_MSG_TYPE::ENUM_MSG_TYPE_CREATE_FOLDER_RESPOND);

    QString newPath = QString(myCurPath) + "/" + QString(folderName);
    qDebug() << myCurPath;
    qDebug() << folderName;
    qDebug() << newPath;
    if (dir.exists(newPath))    // 要创建的新文件夹的这个路径已经存在了，即已经有这个文件夹了，创建失败
    {
        strcpy(createFolderPDU->caData, CREATE_FOLDER_EXIST);
    }
    else
    {
        dir.mkdir(newPath);
        strcpy(createFolderPDU->caData, CREATE_FOLDER_SUCCESS);
    }
    this->write((char*)createFolderPDU, createFolderPDU->uiPDULen);
    free(createFolderPDU);
    createFolderPDU = nullptr;
}

// 处理刷新文件请求
void MyTcpSocket::handleFlushFileRequest(PDU* pdu)
{
    // 获取当前路径下包含的文件和文件夹，返回给客户端显示
    char curPath[pdu->uiMsgLen];
    memcpy(curPath, pdu->caMsg, pdu->uiMsgLen);    // 获取客户端发来的路径

    qDebug() << "flush" << curPath;

    QDir dir;
    dir.setPath(curPath);     // 设置为当前目录

    // 获取当前目录下所有文件信息除了.和..    第二个参数表示排序规则，文件夹优先
    QFileInfoList fileInfoList = dir.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot, QDir::DirsFirst);

    int fileCount = fileInfoList.size();

    PDU* flushFilePDU = makePDU(fileCount * sizeof(FileInfo));
    flushFilePDU->uiMsgType = static_cast<uint>(ENUM_MSG_TYPE::ENUM_MSG_TYPE_FLUSH_FILE_RESPOND);
    for (int i = 0; i < fileCount; i++)      // 将文件信息包装到PDU里面发送给客户端
    {
        // qDebug() << fileInfoList[i].isDir() << fileInfoList[i].fileName() << fileInfoList[i].size();
        FileInfo* f = (FileInfo*)(flushFilePDU->caMsg) + i;
        strncpy(f->fileName, fileInfoList[i].fileName().toStdString().c_str(), 64);
        f->fileType = fileInfoList[i].isDir();
        f->fileSize = fileInfoList[i].size();

    }
    this->write((char*)flushFilePDU, flushFilePDU->uiPDULen);
    free(flushFilePDU);
    flushFilePDU = nullptr;
}

// 处理删除文件的请求
void MyTcpSocket::handleDeleteFileRequest(PDU* pdu)
{
    char curPath[pdu->uiMsgLen];
    memcpy(curPath, pdu->caMsg, pdu->uiMsgLen);     // 获取当前路径

    qDebug() << curPath;

    QDir dir;
    QFileInfo fileInfo(curPath);
    dir.setPath(curPath);

    PDU* deleteFilePDU = makePDU(0);
    deleteFilePDU->uiMsgType = static_cast<uint>(ENUM_MSG_TYPE::ENUM_MSG_TYPE_DELETE_FILE_RESPOND);
    if (fileInfo.isDir())
    {
        if (dir.removeRecursively())
        {
            memcpy(deleteFilePDU->caData, DELETE_SUCCESSED, 64);
        }
        else
        {
            memcpy(deleteFilePDU->caData, DELETE_FAILED, 64);
        }
    }
    else
    {
        if (dir.remove(curPath))
        {
            memcpy(deleteFilePDU->caData, DELETE_SUCCESSED, 64);
        }
        else
        {
            memcpy(deleteFilePDU->caData, DELETE_FAILED, 64);
        }
    }
    this->write((char*)deleteFilePDU, deleteFilePDU->uiPDULen);
    free(deleteFilePDU);
    deleteFilePDU = nullptr;
}

// 处理重命名文件的请求
void MyTcpSocket::handleRenameFileRequest(PDU* pdu)
{
    char curPath[pdu->uiMsgLen];
    char oldName[32];
    char newName[32];

    memcpy(curPath, pdu->caMsg, pdu->uiMsgLen);
    memcpy(oldName, pdu->caData, 32);
    memcpy(newName, pdu->caData + 32, 32);

//    qDebug() << curPath;
//    qDebug() << oldName;
//    qDebug() << newName;

    QString oldPath = QString(curPath) + "/" + oldName;
    QString newPath = QString(curPath) + "/" + newName;

    QDir dir;
    PDU* renameFilePDU = makePDU(0);
    renameFilePDU->uiMsgType = static_cast<uint>(ENUM_MSG_TYPE::ENUM_MSG_TYPE_RENAME_FILE_RESPOND);
    if (!dir.exists(oldPath) || dir.exists(newPath))    // 旧路径或者新路径存在
    {
        memcpy(renameFilePDU->caData, PATH_IS_NOT_CORRECT, 64);
    }
    else
    {
        dir.rename(oldPath, newPath);
    }
    this->write((char*)renameFilePDU, renameFilePDU->uiPDULen);
    free(renameFilePDU);
    renameFilePDU = nullptr;
}

// 处理进入文件夹的请求
void MyTcpSocket::handleEnterFolderRequest(PDU* pdu)
{
    // 服务器判断是不是文件夹，不是文件夹就不做回应, 是文件夹就将新路径发送给客户端，让客户端再调用刷新的槽函数，
    char curPath[pdu->uiMsgLen];
    memcpy(curPath, pdu->caMsg, pdu->uiMsgLen);

    QFileInfo fileInfo;
    fileInfo.setFile(curPath);
    qDebug() << curPath;
    if (!fileInfo.isDir())         // 当前路径不是文件夹
    {
        return;
    }
    PDU* EnterFolderPDU = makePDU(pdu->uiMsgLen);
    EnterFolderPDU->uiMsgType = static_cast<uint>(ENUM_MSG_TYPE::ENUM_MSG_TYPE_ENTER_FOLDER_RESPOND);
    memcpy(EnterFolderPDU->caMsg, curPath, pdu->uiMsgLen);

    this->write((char*)EnterFolderPDU, EnterFolderPDU->uiPDULen);
    free(EnterFolderPDU);
    EnterFolderPDU = nullptr;
}

// 处理返回上一级的请求
void MyTcpSocket::handleReturnPreFolderRequest(PDU* pdu)
{
    char path[pdu->uiMsgLen];
    memcpy(path, pdu->caMsg, pdu->uiMsgLen);

    PDU* returnPreFolderPDU;

    if (strcmp(path, (QString(ROOT_PATH) + "\0").toStdString().c_str()) == 0)     // 已经是根目录了，不能再返回了
    {
        qDebug() << path;
        qDebug() << ROOT_PATH;
        returnPreFolderPDU = makePDU(0);
        memcpy(returnPreFolderPDU->caData, ALREADY_IS_ROOT_FOLDER, 64);
    }
    else
    {
        returnPreFolderPDU = makePDU(QString(path).toUtf8().size());
        memcpy(returnPreFolderPDU->caMsg, path, returnPreFolderPDU->uiMsgLen);
    }
    returnPreFolderPDU->uiMsgType = static_cast<uint>(ENUM_MSG_TYPE::ENUM_MSG_TYPE_RETURN_PRE_FOLDER_RESPOND);

    this->write((char*)returnPreFolderPDU, returnPreFolderPDU->uiPDULen);
    free(returnPreFolderPDU);
    returnPreFolderPDU = nullptr;
}


void MyTcpSocket::recvMsg()
{
//    qDebug() << this->bytesAvailable();
    uint uiPDULen = 0;
    this->read((char*)&uiPDULen, sizeof(uint));    // 先读取总长度4个字节出来，到uiPDULen
    uint uiMsglen = uiPDULen - sizeof(PDU);        // 得到消息长度
    PDU* pdu = makePDU(uiMsglen);                  // 根据消息长度构造出pdu

    // (char*)pdu + sizeof(uint):前四个字节之前已经读完了，所以要读到pud偏移之前读的位置上
    // uiPDULen - sizeof(uint):之前读的不用读了，所以要减去
    this->read((char*)pdu + sizeof(uint), uiPDULen - sizeof(uint));
    // qDebug() << pdu->uiMsgType << username << password;

    switch (pdu->uiMsgType)
    {
    case static_cast<uint>(ENUM_MSG_TYPE::ENUM_MSG_TYPE_REGISTER_REQUEST):   // 注册请求
        handleRegisterRequest(pdu);
        break;

    case static_cast<uint>(ENUM_MSG_TYPE::ENUM_MSG_TYPE_LOGIN_REQUEST):     // 登录请求
        handleLoginRequest(pdu);
        break;

    case static_cast<uint>(ENUM_MSG_TYPE::ENUM_MSG_TYPE_SELECT_ONLINE_USER_REQUEST):   // 查询在线用户请求
        handleSelectOnlineRequest();
        break;

    case static_cast<uint>(ENUM_MSG_TYPE::ENUM_MSG_TYPE_SEARCH_USER_REQUEST):    // 查找用户请求
        handleSearchUserRequest(pdu);
        break;

    case static_cast<uint>(ENUM_MSG_TYPE::ENUM_MSG_TYPE_ADD_FRIEND_REQUEST):     // 添加好友请求
        handleAddFriendRequest(pdu);
        break;

    case static_cast<uint>(ENUM_MSG_TYPE::ENUM_MSG_TYPE_ADD_FRIEND_AGREE):      // 同意添加好友
        handleAgreeRequest(pdu);
        break;

    case static_cast<uint>(ENUM_MSG_TYPE::ENUM_MSG_TYPE_ADD_FRIEND_REFUSE):     // 拒绝好友请求
        handleRefuseRequest(pdu);
        break;

    case static_cast<uint>(ENUM_MSG_TYPE::ENUM_MSG_TYPE_SELECT_FRIEND_REQUEST):     // 查询当前用户所有好友
        handleSelectFriendRequest();
        break;

    case static_cast<uint>(ENUM_MSG_TYPE::ENUM_MSG_TYPE_DELETE_FRIEND_REQUEST):     // 删除好友请求
        handleDeleteFriendRequest(pdu);
        break;

    case static_cast<uint>(ENUM_MSG_TYPE::ENUM_MSG_TYPE_PRIVATE_CHAT_REQUEST):      // 私聊请求
        handlePrivateChatRequest(pdu);
        break;

    case static_cast<uint>(ENUM_MSG_TYPE::ENUM_MSG_TYPE_GROUP_CHAT_REQUEST):      // 群发请求
        handleGroupChatRequest(pdu);
        break;

    case static_cast<uint>(ENUM_MSG_TYPE::ENUM_MSG_TYPE_CREATE_FOLDER_REQUEST):      // 创建文件夹请求
        handleCreateFolderRequest(pdu);
        break;

    case static_cast<uint>(ENUM_MSG_TYPE::ENUM_MSG_TYPE_FLUSH_FILE_REQUEST):      // 刷新文件请求
        handleFlushFileRequest(pdu);
        break;

    case static_cast<uint>(ENUM_MSG_TYPE::ENUM_MSG_TYPE_DELETE_FILE_REQUEST):      // 删除文件请求
        handleDeleteFileRequest(pdu);
        break;

    case static_cast<uint>(ENUM_MSG_TYPE::ENUM_MSG_TYPE_RENAME_FILE_REQUEST):      // 重命名文件请求
        handleRenameFileRequest(pdu);
        break;

    case static_cast<uint>(ENUM_MSG_TYPE::ENUM_MSG_TYPE_ENTER_FOLDER_REQUEST):      // 进入文件夹请求
        handleEnterFolderRequest(pdu);
        break;

    case static_cast<uint>(ENUM_MSG_TYPE::ENUM_MSG_TYPE_RETURN_PRE_FOLDER_REQUEST):   // 返回上一级请求
        handleReturnPreFolderRequest(pdu);
        break;


    default:
        break;
    }
    free(pdu);
    pdu = nullptr;
}

