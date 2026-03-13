#include "mainmenu.h"
#include "ui_mainmenu.h"
#include "protocol.h"
#include "tcpclient.h"
#include <QInputDialog>
#include <QMessageBox>
#include <QTimer>
#include <privatechat.h>

mainMenu::mainMenu(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::mainMenu)
{
    ui->setupUi(this);

    // 1. 用户点击 QListWidget 中的某一项
    // 2. QListWidget 发出 currentRowChanged 信号，传递被点击项的行号（如 0、1、2...）
    // 3. 信号被 QStackedWidget 的 setCurrentIndex 槽函数接收
    // 4. QStackedWidget 根据收到的行号切换到对应索引的页面
    connect(ui->listWidget, &QListWidget::currentRowChanged, ui->stackedWidget, &QStackedWidget::setCurrentIndex);


    on_flushPushButton_clicked();           // 一跑起来就显示所有好友
//    on_showOnlinePushButton_clicked();    // 一跑起来就显示所有在线用户

    // 延迟发送第二个请求，避免冲突，不然会导致粘包，丢失第二个请求
    QTimer::singleShot(100, this, [this]() {
        on_showOnlinePushButton_clicked();
    });

    // 延迟发送第三个请求，避免冲突，不然会导致粘包，丢失第二个请求
    QTimer::singleShot(200, this, [this]() {
        on_flushFilePushButton_clicked();
    });

}

mainMenu::~mainMenu()
{
    delete ui;
}

mainMenu &mainMenu::getInstance()
{
    static mainMenu m;
    return m;
}


void mainMenu::setOnlineUser(QString s)
{
    if (s == mainMenu::getInstance().m_myUsername)
        return;
    ui->onlineUserListWidget->addItem(s);
}

void mainMenu::setMyUsername(QString name)
{
    m_myUsername = name;
    ui->welcomeLabel->setText(name + "的群聊框");
}

QString mainMenu::getMyUsername() const
{
    return m_myUsername;
}

void mainMenu::setFriend(QString s)
{
    ui->friendListWidget->addItem(s);
}

void mainMenu::setGroup(QString username, QString msg)
{
    ui->messageTextEdit->append(username + ":  " + msg);
}

void mainMenu::setFileInfo(bool isDir, QString fileName, long long fileSize)
{
    QListWidgetItem* item = new QListWidgetItem;
    if (isDir)
    {
        item->setIcon(QIcon(QPixmap(":/icons/folder.webp")));
    }
    else
    {
        item->setIcon(QIcon(QPixmap(":/icons/file.webp")));
    }
    fileSize = fileSize % 1024 == 0 ? fileSize / 1024 : fileSize / 1024 + 1;
    item->setText(QString("%1\t%2KB").arg(fileName).arg(fileSize));
    ui->fileListWidget->addItem(item);
}

void mainMenu::emitFlushFileSignal()
{
    emit ui->flushFilePushButton->click();
}



// 显示所有在线用户按钮的槽函数
void mainMenu::on_showOnlinePushButton_clicked()
{
    ui->onlineUserListWidget->clear();     // 先把之前的状态清空掉，再重新向服务器发送请求得到结果刷新列表
    // 发送给服务器查看所有在线用户的请求，服务器收到请求后开始查询在线用户，再将查到的结果发回给客户端，再显示
    PDU* pdu = makePDU(0);      // 只需要发请求，没有附加的文件，所以开0的就行
    pdu->uiMsgType = static_cast<uint>(ENUM_MSG_TYPE::ENUM_MSG_TYPE_SELECT_ONLINE_USER_REQUEST);   // 设置消息类型
    TcpClient::getInstance().getSocket().write((char*)pdu, pdu->uiPDULen);    // 将查询在线用户的请求发给服务器
    free(pdu);
    pdu = nullptr;
}

// 查找用户槽函数
void mainMenu::on_findPushButton_clicked()
{
//    // 利用对话框获取用户输入的用户名，将用户名发送给服务器让服务器查询，然后再返回查询结果
//    bool isSearch;
//    QString username = QInputDialog::getText(this, "search user", "input username", QLineEdit::Normal, "", &isSearch);   // 初始化对话框，获取用户输入到对话框的用户名
//    if (!isSearch)
//        return;

    // 使用对象方式创建对话框，避免 setGeometry 警告
    QString username;
    QInputDialog inputDialog(this);
    inputDialog.setWindowTitle("search user");
    inputDialog.setLabelText("input username");
    inputDialog.setInputMode(QInputDialog::TextInput);
    inputDialog.adjustSize();     // 自适应调整大小，不出警告
    if (inputDialog.exec() != QDialog::Accepted) // 用户点击取消
        return;
    username = inputDialog.textValue();

    PDU* pdu = makePDU(0);
    pdu->uiMsgType = static_cast<uint>(ENUM_MSG_TYPE::ENUM_MSG_TYPE_SEARCH_USER_REQUEST);   // 设置消息类型
    strcpy(pdu->caData, username.toStdString().c_str());    // 将想搜索的用户名拷贝到pdu中

    TcpClient::getInstance().getSocket().write((char*)pdu, pdu->uiPDULen);    // 将消息发送给服务器
    ui->onlineUserListWidget->clear();     // 先把之前的状态清空掉，再重新向服务器发送请求得到结果刷新列表
}

// 添加好友的槽函数
void mainMenu::on_addFriendPushButton_clicked()
{
    QListWidgetItem * item = ui->onlineUserListWidget->currentItem();   // 得到当前列表指向的item
    if (item == nullptr) {
        QMessageBox::information(this, "add friend", "please select a online user");
        return;
    }

    QString username = item->text();   // 想要添加的用户名
    if (username == m_myUsername)      // 不能添加自己为好友
    {
        QMessageBox::information(this, "add friend", "can't add self");
        return;
    }

    // 将自己的用户名和想加的好友的用户名，打包发送给服务器
    PDU* pdu = makePDU(0);
    pdu->uiMsgType = static_cast<uint>(ENUM_MSG_TYPE::ENUM_MSG_TYPE_ADD_FRIEND_REQUEST);
    strncpy(pdu->caData, m_myUsername.toStdString().c_str(), 32);
    strncpy(pdu->caData + 32, username.toStdString().c_str(), 32);

    TcpClient::getInstance().getSocket().write((char*)pdu, pdu->uiPDULen);
    free(pdu);
    pdu = nullptr;
}

// 刷新在线好友的槽函数
void mainMenu::on_flushPushButton_clicked()
{
    ui->friendListWidget->clear();    // 先把之前的状态清空掉，再重新向服务器发送请求得到结果刷新列表
    // 发送给服务器，查询当前用户的所有好友
    PDU* pdu = makePDU(0);
    pdu->uiMsgType = static_cast<uint>(ENUM_MSG_TYPE::ENUM_MSG_TYPE_SELECT_FRIEND_REQUEST);
    TcpClient::getInstance().getSocket().write((char*)pdu, pdu->uiPDULen);   // 因为服务器端的socket存了当前socket登录的用户名，所以这里不需要将用户名写进去发给服务器
    free(pdu);
    pdu = nullptr;
}

// 删除好友的槽函数
void mainMenu::on_deletePushButton_clicked()
{
    QListWidgetItem* item = ui->friendListWidget->currentItem();
    if (item == nullptr)
    {
        QMessageBox::information(this, "delete friend", "please select a online friend");
        return;
    }
    PDU* pdu = makePDU(0);
    pdu->uiMsgType = static_cast<uint>(ENUM_MSG_TYPE::ENUM_MSG_TYPE_DELETE_FRIEND_REQUEST);   // 设置删除好友的类型请求

    memcpy(pdu->caData, item->text().toStdString().c_str(), 32);       // 将要删除的好友的名字放到caData的前32个字节
    memcpy(pdu->caData + 32, m_myUsername.toStdString().c_str(), 32);  // 将自己的名字放到后32字节
    TcpClient::getInstance().getSocket().write((char*)pdu, pdu->uiPDULen);
    free(pdu);
    pdu = nullptr;
}

// 点击私聊触发的槽函数
void mainMenu::on_privateChatPushButton_clicked()
{
    // 选中好友进行私聊
    QString username;
    QListWidgetItem* item = ui->friendListWidget->currentItem();
    if (item == nullptr)   // 不选用户不能私聊
    {
        QMessageBox::information(this, "private chat", "please select a online friend to chat");
        return;
    }
    username = item->text();    // 获取要进行私聊的用户名
    if (m_privateChatMap.find(username) != m_privateChatMap.end())   // 如果已经打开过私聊窗口了，就直接return
    {
        QMessageBox::information(this, "private chat", "already open the private chat window with " + username);
        return;
    }

    // 新开一个窗口进行私聊
    privateChat* newChat = new privateChat(m_myUsername, username);  // 默认叉掉窗口是隐藏窗口，所以需要设置属性
    newChat->setAttribute(Qt::WA_DeleteOnClose);       // 设置成窗口关闭时自动销毁对象，从而触发destroyed信号
    m_privateChatMap.insert(username, newChat);
    newChat->show();

    // 如果关闭窗口就销毁对象，并且从map里面删除
    connect(newChat, &privateChat::destroyed, this, [this, username](){
//        QMap<QString, privateChat*>::iterator it = m_privateChatMap.find(username);   // 找到是哪个窗口
//        delete it.value();            // 销毁对象，所以槽函数这里也不需要自己delete对象了
//        it.value() = nullptr;
        qDebug() << username;
        m_privateChatMap.remove(username);   // 从map里面删除，直接利用key删除，不需要迭代器获取value去delete了
    });
}

// 点击群发触发的槽函数
void mainMenu::on_sendPushButton_clicked()
{
    QString qsMsg = ui->sendMsgLineEdit->text();   // 获取用户在发送框输入的消息
    ui->sendMsgLineEdit->clear();                  // 获取之后输入框就可以清空了
    QByteArray qbaMsg = qsMsg.toUtf8();        // 将QString转换成QByteArray，因为用户可能输入中文，得到的消息的准确字节数，得用QByteArray
    PDU* pdu = makePDU(qbaMsg.size());         // 这里的size就是字节长度，而不是字符个数
    pdu->uiMsgType = static_cast<uint>(ENUM_MSG_TYPE::ENUM_MSG_TYPE_GROUP_CHAT_REQUEST);
    memcpy(pdu->caData, m_myUsername.toStdString().c_str(), 32);       // 将自己的用户名写入caData前32字节
    memcpy(pdu->caMsg, qbaMsg.data(), qbaMsg.size());                  // 将用户输入的消息拷贝到caMsg
    TcpClient::getInstance().getSocket().write((char*)pdu, pdu->uiPDULen);   // 将群发的消息发送给服务器

    ui->messageTextEdit->append(m_myUsername + ":  " + qsMsg);  // 将发送的消息在聊天框

    free(pdu);
    pdu = nullptr;
}

// 点击创建文件夹触发的槽函数
void mainMenu::on_newFolderPushButton_clicked()
{
//    bool isInput;    // 这样用会有警告
//    // 初始化对话框，获取用户输入到对话框的文件夹名字
//    QString folderName = QInputDialog::getText(this, "create folder", "input folder name", QLineEdit::Normal, "", &isInput);
//    if (!isInput)    // 用户点击取消就取消
//        return;
//    if (folderName.isEmpty())
//    {
//        QMessageBox::information(this, "create folder", "folder name can't is empty");
//        return;
//    }

    // 使用对象方式创建对话框，避免 setGeometry 警告
    QString folderName;
    QInputDialog inputDialog(this);
    inputDialog.setWindowTitle("create folder");
    inputDialog.setLabelText("input folder name");
    inputDialog.setInputMode(QInputDialog::TextInput);
    inputDialog.adjustSize();     // 自适应调整大小，不出警告
    if (inputDialog.exec() != QDialog::Accepted) // 用户点击取消
        return;
    folderName = inputDialog.textValue();

    // 将当前路径和要创建的新文件夹名字发送给服务器
    QString myCurPath = TcpClient::getInstance().getMyCurPath();
    QByteArray ba = myCurPath.toUtf8();
    PDU* pdu = makePDU(ba.size());
    pdu->uiMsgType = static_cast<uint>(ENUM_MSG_TYPE::ENUM_MSG_TYPE_CREATE_FOLDER_REQUEST);
    memcpy(pdu->caData, folderName.toStdString().c_str(), 64);
    memcpy(pdu->caMsg, ba.data(), ba.size());
    TcpClient::getInstance().getSocket().write((char*)pdu, pdu->uiPDULen);
    free(pdu);
    pdu = nullptr;
}

// 点击刷新文件触发的槽函数
void mainMenu::on_flushFilePushButton_clicked()
{
    // 将路径发送给服务器，服务器返回当前路径下包含的文件和文件夹
    ui->fileListWidget->clear();
    QString myCurPath = TcpClient::getInstance().getMyCurPath();
    QByteArray ba = myCurPath.toUtf8();


    PDU* pdu = makePDU(ba.size() + 1);    // 要多开一个字节用来存放\0
    pdu->uiMsgType = static_cast<uint>(ENUM_MSG_TYPE::ENUM_MSG_TYPE_FLUSH_FILE_REQUEST);
    memcpy(pdu->caMsg, ba.data(), ba.size());
    pdu->caMsg[ba.size()] = '\0';         // 对于字符串的处理一定要在结尾加上\0，不然会出很多莫名奇妙的问题乱码
    qDebug() << "flush " << pdu->caMsg;

    TcpClient::getInstance().getSocket().write((char*)pdu, pdu->uiPDULen);
    free(pdu);
    pdu = nullptr;

}

// 点击删除文件触发的槽函数
void mainMenu::on_deleteFilePushButton_clicked()
{
    // 将路径发送给服务器，让服务器进行删除
    QString myCurPath = TcpClient::getInstance().getMyCurPath();    // 获取当前路径
    QListWidgetItem* item = ui->fileListWidget->currentItem();
    if (item == nullptr)
    {
        QMessageBox::information(this, "delete file", "please select a file or folder");
        return;
    }
    QString fileName = item->text().split('\t')[0];                 // 获取要删除的文件名
    qDebug() << fileName;
    myCurPath += "/";          // 拼接成新的路径
    myCurPath += fileName;
    QByteArray ba = myCurPath.toUtf8();

    PDU* pdu = makePDU(ba.size() + 1);
    pdu->uiMsgType = static_cast<uint>(ENUM_MSG_TYPE::ENUM_MSG_TYPE_DELETE_FILE_REQUEST);
    memcpy(pdu->caMsg, ba.data(), ba.size());

    TcpClient::getInstance().getSocket().write((char*)pdu, pdu->uiPDULen);
    free(pdu);
    pdu = nullptr;
}

// 点击重命名触发的槽函数
void mainMenu::on_reNamePushButton_clicked()
{
    // 将旧名字（旧路径）和新路径发送给服务器
    QString myCurPath = TcpClient::getInstance().getMyCurPath();    // 获取当前路径
    QListWidgetItem* item = ui->fileListWidget->currentItem();
    if (item == nullptr)
    {
        QMessageBox::information(this, "rename", "please select a file or folder");
        return;
    }
    QString oldName = item->text().split('\t')[0];      // 获取要重命名的文件名

    // 使用对象方式创建对话框，避免 setGeometry 警告
    QInputDialog inputDialog(this);
    inputDialog.setWindowTitle("rename");
    inputDialog.setLabelText("input new name");
    inputDialog.setInputMode(QInputDialog::TextInput);
    inputDialog.adjustSize();     // 自适应调整大小，不出警告
    if (inputDialog.exec() != QDialog::Accepted) // 用户点击取消
        return;
    QString newName = inputDialog.textValue();    // 获取新的文件的名字
    if (newName.toUtf8().size() > 32)             // 名字不能超过32个字节
    {
        QMessageBox::information(this, "rename", "the new name is to long");
        return;
    }

//    qDebug() << myCurPath;
//    qDebug() << oldName;
//    qDebug() << newName;

    PDU* pdu = makePDU(myCurPath.toUtf8().size());        // 将当前路径和旧名字和新名字发送给服务器
    pdu->uiMsgType = static_cast<uint>(ENUM_MSG_TYPE::ENUM_MSG_TYPE_RENAME_FILE_REQUEST);
    memcpy(pdu->caData, oldName.toStdString().c_str(), 32);
    memcpy(pdu->caData + 32, newName.toStdString().c_str(), 32);
    memcpy(pdu->caMsg, myCurPath.toStdString().c_str(), pdu->uiMsgLen);

    TcpClient::getInstance().getSocket().write((char*)pdu, pdu->uiPDULen);
    free(pdu);
    pdu = nullptr;
}

// 双击文件列表中的某一个后触发的槽函数
void mainMenu::on_fileListWidget_itemDoubleClicked(QListWidgetItem *item)
{
    // 将当前路径拼接文件名后发送给服务器
    QString myCurPath = TcpClient::getInstance().getMyCurPath();    // 获取当前路径

    QString fileName = item->text().split('\t')[0];                 // 获取要进入的文件名
    myCurPath += "/";          // 拼接成新的路径
    myCurPath += fileName;
    QByteArray ba = myCurPath.toUtf8();

    PDU* pdu = makePDU(ba.size() + 1);
    pdu->uiMsgType = static_cast<uint>(ENUM_MSG_TYPE::ENUM_MSG_TYPE_ENTER_FOLDER_REQUEST);
    memcpy(pdu->caMsg, ba.data(), ba.size());

    TcpClient::getInstance().getSocket().write((char*)pdu, pdu->uiPDULen);
    free(pdu);
    pdu = nullptr;
}

// 点击返回后触发的槽函数
void mainMenu::on_returnPushButton_clicked()
{
    // 将当前路径处理成要返回的路径，发送给服务器
    QString curPath = TcpClient::getInstance().getMyCurPath();
    int index = curPath.lastIndexOf('/');
    curPath = curPath.remove(index, curPath.size() - index + 1);    // 分割出上一级路径出来

    PDU* pdu = makePDU(curPath.size() + 1);  // 多开一个放\0，不然可能会出问题乱码
    pdu->uiMsgType = static_cast<uint>(ENUM_MSG_TYPE::ENUM_MSG_TYPE_RETURN_PRE_FOLDER_REQUEST);
    memcpy(pdu->caMsg, curPath.toStdString().c_str(), curPath.size());
    pdu->caMsg[curPath.size()] = '\0';       // 记得放\0

    TcpClient::getInstance().getSocket().write((char*)pdu, pdu->uiPDULen);
    free(pdu);
    pdu = nullptr;
}
