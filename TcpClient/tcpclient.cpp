#include "tcpclient.h"
#include "ui_tcpclient.h"
#include <QFile>      // 用来读取配置文件中的连接信息，IP、端口号
#include <QByteArray>
#include <QDebug>
#include <QMessageBox>
#include <QHostAddress>
#include "mainmenu.h"
#include "ui_mainmenu.h"
#include "privatechat.h"
#include <QTimer>
#include <QThread>

TcpClient::TcpClient(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TcpClient)
{
    ui->setupUi(this);
    loadConfig();    // 在构造函数中加载配置文件，初始化ip和port

    m_tcpSocket.connectToHost(QHostAddress(m_strIP), m_usPort);     // 与服务器进行连接

    connect(&m_tcpSocket, &QTcpSocket::connected, this, [this](){   // 如果连接上了，就会发出已连接的信号，调用槽函数
        QMessageBox::information(this, "connect server", "connect server success");
    });

    connect(&m_tcpSocket, &QTcpSocket::readyRead, this, &TcpClient::recvMsg);   // 客户端接收服务端的消息，触发槽函数

}

TcpClient::~TcpClient()
{
    delete ui;
}

TcpClient &TcpClient::getInstance()
{
    static TcpClient instance;
    return instance;
}

QTcpSocket& TcpClient::getSocket()
{
    return m_tcpSocket;
}

// 读取配置文件中的信息（ip，端口号）
void TcpClient::loadConfig()
{
    // .qrc就是一份资源清单，Qt会根据这份清单把指定文件嵌入程序，最终通过 :前缀名 进行路径访问
    QFile file(":/client.config");         // 先将文件加入Qt资源系统，再通过:/路径访问，创建一个file对象，里面传入要加载的文件路径
    if (file.open(QIODevice::ReadOnly))    // 以只读的方式打开文件
    {
       QString strData(file.readAll());    // 读取文件所有内容，readAll()返回的是QByteArray，直接用其构造出QString
       file.close();
       QStringList configList = strData.split("\r\n");     // 分割分别得到ip和port
       m_strIP = configList[0];
       m_usPort = configList[1].toUShort();   // 初始化成员变量
       qDebug() << "ip:" << m_strIP << " port:" << m_usPort;
    }
    else
    {
        QMessageBox::critical(this, "open config", "open config failed");    // 打开配置文件失败，弹出窗口警告
    }
}


// 发送注册请求给服务器
void TcpClient::on_registerPushButton_clicked()
{
    QString username = ui->usernameLineEdit->text();     // 获取用户输入的用户名和密码
    QString password = ui->passwordLineEdit->text();
    if (username.isEmpty() || password.isEmpty())        // 判断用户名或者密码是否为空
    {
        QMessageBox::information(this, "login info", "please input the username and password");
        return;
    }
    PDU* pdu = makePDU(0);     // caData就够存用户名和密码了，所以不需要弹性的数组
    pdu->uiMsgType = static_cast<uint>(ENUM_MSG_TYPE::ENUM_MSG_TYPE_REGISTER_REQUEST);   // enum class是强类型，没有自动转换
    strncpy(pdu->caData, username.toStdString().c_str(), 32);        // 由于数据库设定的32位，所以最多只需要拷贝前32位
    strncpy(pdu->caData + 32, password.toStdString().c_str(), 32);
    m_tcpSocket.write((char*)pdu, pdu->uiPDULen);
    free(pdu);
    pdu = nullptr;
}

void TcpClient::on_cancellationPushButton_clicked()
{
    QString username = ui->usernameLineEdit->text();     // 获取用户输入的用户名和密码
    QString password = ui->passwordLineEdit->text();
    if (username.isEmpty() || password.isEmpty())        // 判断用户名或者密码是否为空
    {
        QMessageBox::information(this, "cancellation info", "please input the username and password");
        return;
    }

//    if (OperateDB::getInstance().deleteUserInfo(username, password))    // 删除用户信息，相当于注销
//    {
//        QMessageBox::information(this, "cancellation info", "cancellation successed");
//    }
//    else   // 用户名或者密码不对，删除失败
//    {
//        QMessageBox::information(this, "cancellation info", "username or password is not correct");
    //    }
}


// 处理服务器发来的注册回复
void TcpClient::handleRegisterRespond(PDU* pdu)
{
//            qDebug() << strcmp(pdu->caData, REGISTER_SUCCESSED);
//            qDebug() << strcmp(pdu->caData, REGISTER_FAILED);

    if (strcmp(pdu->caData, REGISTER_SUCCESSED) == 0)    // 收到服务器传来的注册成功的消息
    {
        QMessageBox::information(this, "register info", REGISTER_SUCCESSED);
    }
    else if (strcmp(pdu->caData, REGISTER_FAILED) == 0)       // 收到服务器传来的注册失败的消息
    {
        QMessageBox::information(this, "register info", REGISTER_FAILED);
    }
}

// 处理服务器发来的登录回复
void TcpClient::handleLoginRespond(PDU* pdu)
{
    if (strcmp(pdu->caData, LOGIN_SUCCESSED) == 0)    // 收到服务器传来的登录成功的消息
    {
        // 登录成功后读取当前所在的路径
        char curPath[pdu->uiMsgLen];
        memcpy(curPath, pdu->caMsg, pdu->uiMsgLen);
        m_myCurPath = curPath;

        QMessageBox::information(this, "login info", LOGIN_SUCCESSED);
        mainMenu::getInstance().setMyUsername(ui->usernameLineEdit->text());    // 登录时记下用户名到mainMenu
        mainMenu::getInstance().show();       // 登陆成功，显示主菜单
        this->hide();                         // 隐藏登录窗口
    }
    else if (strcmp(pdu->caData, LOGIN_FAILED) == 0)       // 收到服务器传来的登录失败的消息
    {
        QMessageBox::information(this, "login info", LOGIN_FAILED);
    }
}

//void TcpClient::handleSelectOnlineUserRespond(PDU* pdu)
//{

//    char onlineUsers[pdu->uiMsgLen];
//    // 就是这一步错了，strcpy拷贝所有的用户到onlineUsers，但是第一个用户后面有/0，所以第一个用户能正常显示
//    // strcpy看到/0就结束了，后面的就不拷贝了，所以第一个用户显示正常，后面的用户全是乱码
//    strcpy(onlineUsers, (char*)pdu->caMsg);     // 将收到的在线用户存到onlineUser中;
//    for (int i = 0; i < pdu->uiMsgLen / 32; i++)
//    {
//        char substr[32];
//        strncpy(substr, onlineUsers + i * 32, 32);      // 每次从res中取出32个字节设置到在线好友上
//        qDebug() << substr;
//        mainMenu::getInstance().setOnlineUser(substr);
//    }
//}

// 处理服务器发来的查询所有在线用户的回复
void TcpClient::handleSelectOnlineUserRespond(PDU* pdu)
{
    for (int i = 0; i < pdu->uiMsgLen / 32; i++)
    {
        char substr[32];
        strncpy(substr, pdu->caMsg + i * 32, 32);      // 每次从res中取出32个字节设置到在线好友上
        qDebug() << substr;
        mainMenu::getInstance().setOnlineUser(substr);
    }

}


// 处理服务器发来的搜索用户的回复
void TcpClient::handleSearchUserRespond(PDU* pdu)
{
    if (strcmp(pdu->caData, SEARCH_USER_NOT_EIXST) == 0)        // 搜索的用户不存在
    {
        QMessageBox::information(this, "search user", SEARCH_USER_NOT_EIXST);
    }
    else if (strcmp(pdu->caData, SEARCH_USER_NOT_ONLINE) == 0)  // 搜索的用户不在线
    {
        QMessageBox::information(this, "search user", SEARCH_USER_NOT_ONLINE);
    }
    else     // 搜索的用户在线
    {
        char username[32];
        strcpy(username, pdu->caData);
        mainMenu::getInstance().setOnlineUser(username);
    }
}

// 处理服务器发来的加好友的回复
void TcpClient::handleAddFriendRespond(PDU* pdu)
{
    if (strcmp(pdu->caData, SEARCH_USER_NOT_ONLINE) == 0)  // 要加的好友不在线
    {
        QMessageBox::information(this, "add friend", SEARCH_USER_NOT_ONLINE);
    }
    else if (strcmp(pdu->caData, ALREADY_IS_FRIEND) == 0)  // 已经是好友了
    {
        QMessageBox::information(this, "add friend", ALREADY_IS_FRIEND);
    }
}

// 处理服务器转发的的加好友的请求
void TcpClient::handleAddFriendRequest(PDU* pdu)
{
    char username[32];
    char myUsername[32];
    strncpy(username, pdu->caData, 32);
    strncpy(myUsername, pdu->caData + 32, 32);
    int res = QMessageBox::information(this, "add friend", QString(username) + " want to add you as friend",
                             QMessageBox::Yes, QMessageBox::No);

    PDU* addFriendPDU = makePDU(0);
    strncpy(addFriendPDU->caData, username, 32);
    strncpy(addFriendPDU->caData + 32, myUsername, 32);
    if (res == QMessageBox::Yes)      // 接收加好友的请求，那么就再发信息给服务器，通过加好友，那么就写入数据库
    {
//        qDebug() << 3 << username << myUsername;
        addFriendPDU->uiMsgType = static_cast<uint>(ENUM_MSG_TYPE::ENUM_MSG_TYPE_ADD_FRIEND_AGREE);
        QMessageBox::information(this, "add friend", "you can chat with " + QString(username) + " now");
    }
    else                              // 拒绝加好友的请求，那么服务器就再转发给请求的客户端，说明拒绝加好友
    {
        addFriendPDU->uiMsgType = static_cast<uint>(ENUM_MSG_TYPE::ENUM_MSG_TYPE_ADD_FRIEND_REFUSE);
    }
    m_tcpSocket.write((char*)addFriendPDU, addFriendPDU->uiPDULen);     // 发送给服务器是否同意加好友的消息
    free(addFriendPDU);
    addFriendPDU = nullptr;
}

// 处理同意加好友的情况
void TcpClient::handleAgreeFriend(PDU* pdu)
{
    char name[32];
    strncpy(name, pdu->caData + 32, 32);
    QMessageBox::information(this, "", QString(name) + " agree you apply, you can chat now");
}

// 处理不同意加好友的情况
void TcpClient::handleRefuseFriend(PDU* pdu)
{
    char name[32];
    strncpy(name, pdu->caData + 32, 32);
    QMessageBox::information(this, "", "sorry," + QString(name) + " refuse you apply");
}

// 处理查询所有好友
void TcpClient::handleSelectFriend(PDU* pdu)
{
    for (int i = 0; i < pdu->uiMsgLen / 32; i++)
    {
        char substr[32];
        strncpy(substr, pdu->caMsg + i * 32, 32);              // 每次从res中取出32个字节设置到在线好友上
        mainMenu::getInstance().setFriend(substr);
    }
}

// 处理删除好友的回复
void TcpClient::handleDeleteFriendRespond(PDU *pdu)
{
    char username[32];
    memcpy(username, pdu->caData, 32);
    QMessageBox::information(this, "delete friend", QString(username) + " already be deleted");
}

// 处理删除好友的请求
void TcpClient::handleDeleteFriendRequest(PDU *pdu)
{
    char username[32];
    memcpy(username, pdu->caData + 32, 32);
    QMessageBox::information(this, "delete friend", QString(username) + " delete you");
}

// 处理私聊好友的请求
void TcpClient::handlePrivateChatRequest(PDU* pdu)
{
    char myUsername[32];
    char username[32];
    memcpy(username, pdu->caData, 32);     // 这是收到私聊请求的客户端，所以名字要反过来读取，先读username
    memcpy(myUsername, pdu->caData + 32, 32);

    // 修改前：依赖 \0，不安全
    // QString msg = QString::fromUtf8(pdu->caMsg);
    // makePDU 分配的内存正好只能装下消息内容， 并没有在末尾多分配一个字节来存放 \0 。
    // 虽然 makePDU 内部可能调用了 memset 清零，但 memcpy 把分配的 caMsg 区域填满了，导致 caMsg 变成了一个 非 Null 结尾 的字符数组。
    // QString::fromUtf8(const char *str) 默认认为传入的是一个标准的 C 风格字符串，它会从 pdu->caMsg 开始读取，一直读到遇到 \0 为止。
    // 由于 pdu->caMsg 末尾没有 \0 ，函数会继续向后读取属于其他变量或未分配的内存区域，直到运气好碰巧遇到一个 0 字节。

    // 修改后：指定长度读取，安全
    // 显式告诉 QString 消息的长度 ，这样它就不会依赖 \0 来判断结束了。
    QString msg = QString::fromUtf8(pdu->caMsg, pdu->uiMsgLen);         // UTF-8

    // 如果已经和改用户打开过私聊窗口了，就直接在之前的窗口追加显示
    if (mainMenu::getInstance().m_privateChatMap.find(username) != mainMenu::getInstance().m_privateChatMap.end())
    {
        mainMenu::getInstance().m_privateChatMap[username]->setRecordTextEdit(msg);
        return;
    }
    // 第一次收到私聊请求就要创建新窗口弹出
    privateChat* newChat = new privateChat(myUsername, username);
    newChat->setAttribute(Qt::WA_DeleteOnClose);       // 设置成窗口关闭时自动销毁对象，从而触发destroyed信号
    mainMenu::getInstance().m_privateChatMap.insert(username, newChat);
    newChat->show();
    newChat->setRecordTextEdit(msg);

    // 如果关闭窗口就销毁对象，并且从map里面删除
    connect(newChat, &privateChat::destroyed, this, [username](){
//        QMap<QString, privateChat*>::iterator it = m_privateChatMap.find(username);   // 找到是哪个窗口
//        delete it.value();            // 销毁对象，所以槽函数这里也不需要自己delete对象了
//        it.value() = nullptr;
        mainMenu::getInstance().m_privateChatMap.remove(username);   // 从map里面删除，直接利用key删除，不需要迭代器获取value去delete了
    });
}

// 处理私聊好友的回复
void TcpClient::handlePrivateChatRespond(PDU* pdu)
{
    if (strcmp(pdu->caData, FRIEND_NOT_ONLINE) == 0)   // 好友不在线发送失败
    {
        QMessageBox::information(this, "private chat", FRIEND_NOT_ONLINE);
    }
}

// 处理收到群聊消息的请求
void TcpClient::handleGroupChatRequest(PDU* pdu)
{
    char username[32];
    memcpy(username, pdu->caData, 32);
    // 修改前：依赖 \0，不安全
    // QString msg = QString::fromUtf8(pdu->caMsg);

    // 修改后：指定长度读取，安全
    QString msg = QString::fromUtf8(pdu->caMsg, pdu->uiMsgLen);     // UTF-8
    mainMenu::getInstance().setGroup(username, msg);
}

// 处理创建文件夹的回复
void TcpClient::handleCreateFolderRespond(PDU* pdu)
{
    if (strcmp(pdu->caData, CREATE_FOLDER_SUCCESS) == 0)   // 创建文件夹成功
    {
        QMessageBox::information(this, "create folder", CREATE_FOLDER_SUCCESS);
    }
    else
    {
        QMessageBox::information(this, "create folder", CREATE_FOLDER_EXIST);
    }
}

// 处理刷新文件的请求
void TcpClient::handleFlushFileRespond(PDU* pdu)
{
    // 1.将文件信息提取出来
    int fileCount = pdu->uiMsgLen / sizeof(FileInfo);
    for (int i = 0; i < fileCount; i++)
    {
        FileInfo* f = (FileInfo*)(pdu->caMsg) + i;

        // 2.将文件信息显示到ui上面
        mainMenu::getInstance().setFileInfo(f->fileType, f->fileName, f->fileSize);
    }
}

// 处理收到删除文件的回复
void TcpClient::handleDeleteFileRespond(PDU* pdu)
{
    if (strcmp(pdu->caData, DELETE_FAILED) ==  0)     // 删除文件失败
    {
        QMessageBox::information(this, "delete file", DELETE_FAILED);
    }
    else if (strcmp(pdu->caData, DELETE_SUCCESSED)  == 0)   // 删除文件成功
    {
        QMessageBox::information(this, "delete file", DELETE_SUCCESSED);
    }
}

// 处理重命名文件的回复
void TcpClient::handleRenameFileRespond(PDU* pdu)
{
    if (strcmp(pdu->caData, PATH_IS_NOT_CORRECT) == 0)    // 重命名文件失败
    {
        QMessageBox::information(this, "rename", PATH_IS_NOT_CORRECT);
    }
    else
    {
        QMessageBox::information(this, "rename", "rename successed");
    }
}

// 处理进入文件夹的请求
void TcpClient::handleEnterFolderRespond(PDU* pdu)
{
    m_myCurPath = pdu->caMsg;    // 进入文件夹了，重新设置当前路径了
    qDebug() << m_myCurPath;
    mainMenu::getInstance().emitFlushFileSignal();       // 发出刷新的信号
}

// 处理返回上一级文件夹的请求
void TcpClient::handleReturnPreFolderRespond(PDU* pdu)
{
    if (strcmp(pdu->caData, ALREADY_IS_ROOT_FOLDER) == 0)   // 已经是根目录了，不能再返回了
    {
        QMessageBox::information(this, "return previous folder", ALREADY_IS_ROOT_FOLDER);
    }
    else
    {
        m_myCurPath = pdu->caMsg;    // 返回文件夹了，重新设置当前路径了
        qDebug() << m_myCurPath;
        mainMenu::getInstance().emitFlushFileSignal();    // 发出刷新的信号
    }
}

// 处理上传文件的回复
void TcpClient::handleUploadFileRespond(PDU* pdu)
{
    if (strcmp(pdu->caData, FILE_IS_EXIST) == 0)    // 上传失败
    {
        QMessageBox::information(this, "upload file", FILE_IS_EXIST);
        return;
    }
    else    // 上传成功
    {
        QString uploadFilePath = mainMenu::getInstance().getUploadFilePath();   // 获取要上传的文件的路径
        QFile file(uploadFilePath);

        int index = uploadFilePath.lastIndexOf('/');
        QString uploadFileName = uploadFilePath.right(uploadFilePath.size() - index - 1);   // 提取出要上传的文件的名字
        QString curPath = m_myCurPath + "/" + uploadFileName;       // 拼接成服务器所在文件的路径

        if (file.open(QIODevice::ReadOnly))
        {
//            QByteArray fileData = file.readAll();    // 读取出数据

//            PDU* pdu = makePDU(fileData.size());
//            pdu->uiMsgType = static_cast<uint>(ENUM_MSG_TYPE::ENUM_MSG_TYPE_TRANSFER_DATA_REQUEST);

//            memcpy(pdu->caData, curPath.toStdString().c_str(), 64);
//            memcpy(pdu->caMsg, fileData.toStdString().c_str(), pdu->uiMsgLen);

//            m_tcpSocket.write((char*)pdu, pdu->uiPDULen);
//            free(pdu);
//            pdu = nullptr;
//            file.close();
            // 不能一次将readAll里面读取的所有的都发送出去，不然会出现半包的情况，要是文件太大的话，读取不到后面的文件内容
            // 一次 readyRead 触发时，缓冲区里可能是：
            // 半包	你只发了一帧 PDU，但 TCP 拆成多个报文传输，或内核还没都拷进缓冲区 → 这次 read 只能读到这一帧的前面一段
            // 粘包	你连续发了两帧（或一帧 + 下一帧的开头），它们都在缓冲区里 → 一次 read 可能一次读到超过一帧的数据。
            char onceMsg[4096];     // 4096个字节读写效率更高
            qint64 onceSize = 0;
            while (true)
            {
                onceSize = file.read(onceMsg, 4096);     // 一次最多读取4096大小的数据到onceMsg，返回值是实际读取到的数据大小
                // qDebug() << onceSize;
                if (onceSize > 0 && onceSize <= 4096)
                {
                    m_tcpSocket.write(onceMsg, onceSize);   // 一次最多发4096的大小的数据
                }
                if (onceSize <= 0)      // 整个文件都发送完了就break
                {
                    break;
                }
            }

            PDU* pdu = makePDU(0);      // 发送完数据后，发送PDU说明传输完成了
            pdu->uiMsgType = static_cast<uint>(ENUM_MSG_TYPE::ENUM_MSG_TYPE_TRANSFER_DATA_REQUEST);
            memcpy(pdu->caData, curPath.toStdString().c_str(), 64);
            m_tcpSocket.write((char*)pdu, pdu->uiPDULen);
            free(pdu);
            pdu = nullptr;
            file.close();
        }
        else
        {
            QMessageBox::information(this, "open file", "open file failed");
        }
    }
}

// 处理传输数据的回复
void TcpClient::handleTransferDataRespond(PDU* pdu)
{
    if (strcmp(pdu->caData, TRANSFER_DATA_SUCCESSED) == 0)
    {
        QMessageBox::information(this, "transfer data", TRANSFER_DATA_SUCCESSED);
    }
    else
    {
        QMessageBox::information(this, "transfer data", TRANSFER_DATA_FAILED);
    }
}

// 处理下载文件的回复
void TcpClient::handleDownloadFileRespond(PDU* pdu)
{
    // 客户端接收文件数据
    qint64 totalSize;
    char curData[pdu->uiMsgLen];
    memcpy(&totalSize, pdu->caData, sizeof(qint64));   // 获取文件剩余大小
    memcpy(curData, pdu->caMsg, pdu->uiMsgLen);   // 获取当前数据
    // m_totalData += curData;   // 客户端用 m_totalData += curData; 追加二进制会被截断
    // 如果这里走的是 QByteArray::operator+=(const char*) 那种“按 C 字符串遇到 \0 截断”的重载
    // 那么下载文件只要中间出现 0x00，就会提前截断，导致保存内容错误、长度不对。
    m_totalData.append(curData, pdu->uiMsgLen);

    // qDebug() << m_totalData.size();

    if (m_totalData.size() >= totalSize)      // 接收完了文件的所有数据，保存到用户指定的路径
    {
        QString downloadPath = mainMenu::getInstance().getDownloadFilePath();   // 得到下载路径
        QFile file(downloadPath);
        if (file.open(QIODevice::WriteOnly))     // 以只写的方式打开文件，如果不存在，会新建一个
        {
            file.write(m_totalData, m_totalData.size());   // 把所有数据全部写入当前文件
            m_totalData.clear();  // 清除当前数据
            file.close();
            QMessageBox::information(this, "download file", "download file successed");
        }
        else
        {
            QMessageBox::information(this, "download file", "download file failed");
        }
    }
}

//void TcpClient::recvMsg()
//{
////    qDebug() << m_tcpSocket.bytesAvailable();
//    uint uiPDULen = 0;      // 将收到的PDU的总长度读到uiPDULen
//    m_tcpSocket.read((char*)&uiPDULen, sizeof(uint));    // 先读取收到的PDU的总长度4个字节出来，到uiPDULen
//    uint uiMsglen = uiPDULen - sizeof(PDU);        // 得到消息数据部分大小
//    PDU* pdu = makePDU(uiMsglen);                  // 根据消息数据部分大小构造出pdu
//    // (char*)pdu + sizeof(uint):前四个字节之前已经读完了，所以要读到pud偏移之前读的位置上
//    // uiPDULen - sizeof(uint):之前读的不用读了，所以要减去
//    m_tcpSocket.read((char*)pdu + sizeof(uint), uiPDULen - sizeof(uint));  // 将剩余部分全部读到pdu中
//    // qDebug() << pdu->uiMsgType << username << password;

//    switch (pdu->uiMsgType)
//    {
//    case static_cast<uint>(ENUM_MSG_TYPE::ENUM_MSG_TYPE_REGISTER_RESPOND):   // 收到服务器的注册回复
//        handleRegisterRespond(pdu);
//        break;

//    case static_cast<uint>(ENUM_MSG_TYPE::ENUM_MSG_TYPE_LOGIN_RESPOND):     // 收到服务器的登录回复
//        handleLoginRespond(pdu);
//        break;

//    case static_cast<uint>(ENUM_MSG_TYPE::ENUM_MSG_TYPE_SELECT_ONLINE_USER_RESPOND):    // 收到服务器的查询在线用户回复
//        handleSelectOnlineUserRespond(pdu);
//        break;

//    case static_cast<uint>(ENUM_MSG_TYPE::ENUM_MSG_TYPE_SEARCH_USER_RESPOND):       // 收到服务器的查询用户回复
//        handleSearchUserRespond(pdu);
//        break;

//    case static_cast<uint>(ENUM_MSG_TYPE::ENUM_MSG_TYPE_ADD_FRIEND_RESPOND):        // 处理服务器的加好友的回复
//        handleAddFriendRespond(pdu);
//        break;

//    case static_cast<uint>(ENUM_MSG_TYPE::ENUM_MSG_TYPE_ADD_FRIEND_REQUEST):        // 处理服务器转发的加好友的请求
//        handleAddFriendRequest(pdu);
//        break;

//    case static_cast<uint>(ENUM_MSG_TYPE::ENUM_MSG_TYPE_ADD_FRIEND_AGREE):          // 收到同意加好友的消息
//        handleAgreeFriend(pdu);
//        break;

//    case static_cast<uint>(ENUM_MSG_TYPE::ENUM_MSG_TYPE_ADD_FRIEND_REFUSE):        // 收到拒绝加好友的消息
//        handleRefuseFriend(pdu);
//        break;

//    case static_cast<uint>(ENUM_MSG_TYPE::ENUM_MSG_TYPE_SELECT_FRIEND_RESPOND):    // 收到查询好友的回复
//        handleSelectFriend(pdu);
//        break;

//    case static_cast<uint>(ENUM_MSG_TYPE::ENUM_MSG_TYPE_DELETE_FRIEND_RESPOND):    // 收到删除好友的回复
//        handleDeleteFriendRespond(pdu);
//        break;

//    case static_cast<uint>(ENUM_MSG_TYPE::ENUM_MSG_TYPE_DELETE_FRIEND_REQUEST):    // 被删除的收到删除好友的请求
//        handleDeleteFriendRequest(pdu);
//        break;

//    case static_cast<uint>(ENUM_MSG_TYPE::ENUM_MSG_TYPE_PRIVATE_CHAT_REQUEST):    // 处理收到好友私聊的请求
//        handlePrivateChatRequest(pdu);
//        break;

//    case static_cast<uint>(ENUM_MSG_TYPE::ENUM_MSG_TYPE_PRIVATE_CHAT_RESPOND):    // 处理收到好友私聊的回复
//        handlePrivateChatRespond(pdu);
//        break;

//    case static_cast<uint>(ENUM_MSG_TYPE::ENUM_MSG_TYPE_GROUP_CHAT_REQUEST):    // 处理收到群聊的请求
//        handleGroupChatRequest(pdu);
//        break;

//    case static_cast<uint>(ENUM_MSG_TYPE::ENUM_MSG_TYPE_CREATE_FOLDER_RESPOND):    // 处理收到创建文件夹的回复
//        handleCreateFolderRespond(pdu);
//        break;

//    case static_cast<uint>(ENUM_MSG_TYPE::ENUM_MSG_TYPE_FLUSH_FILE_RESPOND):    // 处理收到刷新文件的回复
//        handleFlushFileRespond(pdu);
//        break;

//    case static_cast<uint>(ENUM_MSG_TYPE::ENUM_MSG_TYPE_DELETE_FILE_RESPOND):    // 处理收到删除文件的回复
//        handleDeleteFileRespond(pdu);
//        break;

//    case static_cast<uint>(ENUM_MSG_TYPE::ENUM_MSG_TYPE_RENAME_FILE_RESPOND):    // 处理收到重命名文件的回复
//        handleRenameFileRespond(pdu);
//        break;

//    case static_cast<uint>(ENUM_MSG_TYPE::ENUM_MSG_TYPE_ENTER_FOLDER_RESPOND):    // 处理收到进入文件夹的回复
//        handleEnterFolderRespond(pdu);
//        break;

//    case static_cast<uint>(ENUM_MSG_TYPE::ENUM_MSG_TYPE_RETURN_PRE_FOLDER_RESPOND):    // 处理返回上一级文件夹的回复
//        handleReturnPreFolderRespond(pdu);
//        break;

//    case static_cast<uint>(ENUM_MSG_TYPE::ENUM_MSG_TYPE_UPLOAD_FILE_RESPOND):    // 处理上传文件的回复
//        handleUploadFileRespond(pdu);
//        break;

//    case static_cast<uint>(ENUM_MSG_TYPE::ENUM_MSG_TYPE_TRANSFER_DATA_RESPOND):    // 处理传输数据的回复
//        handleTransferDataRespond(pdu);
//        break;

//    case static_cast<uint>(ENUM_MSG_TYPE::ENUM_MSG_TYPE_DOWNLOAD_FILE_RESPOND):    // 处理下载文件的回复
//        handleDownloadFileRespond(pdu);
//        break;

//    }
//    free(pdu);
//    pdu = nullptr;
//}

void TcpClient::recvMsg()
{
    // 循环处理所有粘包PDU，直到数据不够
    while (true)
    {
        // 步骤1：判断缓冲区里有没有至少 4 字节数据
        // bytesAvailable()：返回当前 TCP 接收缓冲区里 未读取的字节数
        // sizeof(uint) = 4 字节，这是 PDU 协议最开头的 "总长度字段"
        // 作用：如果连 4 个字节都没有，连 PDU 多大都不知道，直接退出，不读了
        // 解决问题：防止无数据硬读，出现 maxSize < 0 报错
        if (m_tcpSocket.bytesAvailable() < sizeof(uint))
            break;

        // 步骤2：偷看（peek）前 4 字节，获取 PDU 总长度
        // peek：读取数据，但不从缓冲区移除（只看不拿）
        // 作用：先知道这个 PDU 一共有多大，为后面判断做准备
        uint PDULen = 0;
        m_tcpSocket.peek((char*)&PDULen, sizeof(uint));

        // 步骤3：安全校验，判断 PDU 长度是否合法
        // 1. 不能小于 PDU 头部本身大小（无效包）
        // 2. 不能大于 10MB（防止恶意数据、内存爆炸）
        // 作用：过滤错误数据、乱码数据，防止程序崩溃
        if (PDULen < sizeof(PDU) || PDULen > 10 * 1024 * 1024)
            break;

        // 步骤4：判断缓冲区数据是否足够读完 一整个PDU
        // TCP 是流式传输，一个包可能分两次到达
        // 如果缓冲区数据 < PDU总长度 → 包没收完，不能读，等下一波数据
        // 作用：保证永远只读“完整的包”，不读半包，不解析错乱
        if (m_tcpSocket.bytesAvailable() < PDULen)
            break;

        // 步骤5：【真正读取】从缓冲区拿走数据，读取完整 PDU
        // 5.1 读取 4 字节长度（真正从缓冲区移除数据）
        m_tcpSocket.read((char*)&PDULen, sizeof(uint));

        // 5.2 计算实际消息体（caMsg）的长度
        // PDU总长度 - PDU固定头部长度 = 真实数据长度
        uint MsgLen = PDULen - sizeof(PDU);

        // 5.3 根据计算出的长度，创建对应的 PDU 结构体
        PDU* pdu = makePDU(MsgLen);

        // 5.4 读取 PDU 剩余的所有数据
        // (char*)pdu + sizeof(uint)：跳过已经读过的 4 字节长度
        // uiPDULen - sizeof(uint)：读取剩下的全部协议内容
        m_tcpSocket.read((char*)pdu + sizeof(uint), PDULen - sizeof(uint));

        // 6. 处理PDU
        switch (pdu->uiMsgType)
        {
        case static_cast<uint>(ENUM_MSG_TYPE::ENUM_MSG_TYPE_REGISTER_RESPOND):
            handleRegisterRespond(pdu);
            break;
        case static_cast<uint>(ENUM_MSG_TYPE::ENUM_MSG_TYPE_LOGIN_RESPOND):
            handleLoginRespond(pdu);
            break;
        case static_cast<uint>(ENUM_MSG_TYPE::ENUM_MSG_TYPE_SELECT_ONLINE_USER_RESPOND):
            handleSelectOnlineUserRespond(pdu);
            break;
        case static_cast<uint>(ENUM_MSG_TYPE::ENUM_MSG_TYPE_SEARCH_USER_RESPOND):
            handleSearchUserRespond(pdu);
            break;
        case static_cast<uint>(ENUM_MSG_TYPE::ENUM_MSG_TYPE_ADD_FRIEND_RESPOND):
            handleAddFriendRespond(pdu);
            break;
        case static_cast<uint>(ENUM_MSG_TYPE::ENUM_MSG_TYPE_ADD_FRIEND_REQUEST):
            handleAddFriendRequest(pdu);
            break;
        case static_cast<uint>(ENUM_MSG_TYPE::ENUM_MSG_TYPE_ADD_FRIEND_AGREE):
            handleAgreeFriend(pdu);
            break;
        case static_cast<uint>(ENUM_MSG_TYPE::ENUM_MSG_TYPE_ADD_FRIEND_REFUSE):
            handleRefuseFriend(pdu);
            break;
        case static_cast<uint>(ENUM_MSG_TYPE::ENUM_MSG_TYPE_SELECT_FRIEND_RESPOND):
            handleSelectFriend(pdu);
            break;
        case static_cast<uint>(ENUM_MSG_TYPE::ENUM_MSG_TYPE_DELETE_FRIEND_RESPOND):
            handleDeleteFriendRespond(pdu);
            break;
        case static_cast<uint>(ENUM_MSG_TYPE::ENUM_MSG_TYPE_DELETE_FRIEND_REQUEST):
            handleDeleteFriendRequest(pdu);
            break;
        case static_cast<uint>(ENUM_MSG_TYPE::ENUM_MSG_TYPE_PRIVATE_CHAT_REQUEST):
            handlePrivateChatRequest(pdu);
            break;
        case static_cast<uint>(ENUM_MSG_TYPE::ENUM_MSG_TYPE_PRIVATE_CHAT_RESPOND):
            handlePrivateChatRespond(pdu);
            break;
        case static_cast<uint>(ENUM_MSG_TYPE::ENUM_MSG_TYPE_GROUP_CHAT_REQUEST):
            handleGroupChatRequest(pdu);
            break;
        case static_cast<uint>(ENUM_MSG_TYPE::ENUM_MSG_TYPE_CREATE_FOLDER_RESPOND):
            handleCreateFolderRespond(pdu);
            break;
        case static_cast<uint>(ENUM_MSG_TYPE::ENUM_MSG_TYPE_FLUSH_FILE_RESPOND):
            handleFlushFileRespond(pdu);
            break;
        case static_cast<uint>(ENUM_MSG_TYPE::ENUM_MSG_TYPE_DELETE_FILE_RESPOND):
            handleDeleteFileRespond(pdu);
            break;
        case static_cast<uint>(ENUM_MSG_TYPE::ENUM_MSG_TYPE_RENAME_FILE_RESPOND):
            handleRenameFileRespond(pdu);
            break;
        case static_cast<uint>(ENUM_MSG_TYPE::ENUM_MSG_TYPE_ENTER_FOLDER_RESPOND):
            handleEnterFolderRespond(pdu);
            break;
        case static_cast<uint>(ENUM_MSG_TYPE::ENUM_MSG_TYPE_RETURN_PRE_FOLDER_RESPOND):
            handleReturnPreFolderRespond(pdu);
            break;
        case static_cast<uint>(ENUM_MSG_TYPE::ENUM_MSG_TYPE_UPLOAD_FILE_RESPOND):
            handleUploadFileRespond(pdu);
            break;
        case static_cast<uint>(ENUM_MSG_TYPE::ENUM_MSG_TYPE_TRANSFER_DATA_RESPOND):
            handleTransferDataRespond(pdu);
            break;
        case static_cast<uint>(ENUM_MSG_TYPE::ENUM_MSG_TYPE_DOWNLOAD_FILE_RESPOND):
            handleDownloadFileRespond(pdu);
            break;
        default:
            break;
        }

        free(pdu);
        pdu = nullptr;
    }
}

// 发送登录请求给服务器
void TcpClient::on_loginPushButtone_clicked()
{
    QString username = ui->usernameLineEdit->text();
    QString password = ui->passwordLineEdit->text();
    PDU* pdu = makePDU(0);
    pdu->uiMsgType = static_cast<uint>(ENUM_MSG_TYPE::ENUM_MSG_TYPE_LOGIN_REQUEST);
    strncpy(pdu->caData, username.toStdString().c_str(), 32);
    strncpy(pdu->caData + 32, password.toStdString().c_str(), 32);
    m_tcpSocket.write((char*)pdu, pdu->uiPDULen);
    free(pdu);
    pdu = nullptr;
}

QString TcpClient::getMyCurPath() const
{
    return m_myCurPath;
}
