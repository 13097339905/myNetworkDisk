#include "mainmenu.h"
#include "ui_mainmenu.h"
#include "protocol.h"
#include "tcpclient.h"
#include <QInputDialog>
#include <QMessageBox>

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

    on_showOnlinePushButton_clicked();

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

void mainMenu::setOnlineUser(QStringList& qs)
{
    ui->onlineUserListWidget->clear();
    ui->onlineUserListWidget->addItems(qs);
}

void mainMenu::setOnlineUser(QString& qs)
{
    ui->onlineUserListWidget->clear();
    ui->onlineUserListWidget->addItem(qs);
}

void mainMenu::setMyUsername(const QString name)
{
    m_myUsername = name;
}

QString mainMenu::getMyUsername() const
{
    return m_myUsername;
}



// 显示所有在线用户按钮的槽函数
void mainMenu::on_showOnlinePushButton_clicked()
{
    ui->onlineUserListWidget->show();
    ui->addFriendPushButton->show();
    // 发送给服务器查看所有在线用户的请求，服务器收到请求后开始查询在线用户，再将查到的结果发回给客户端，再显示
    PDU* pdu = makePDU(0);      // 只需要发请求，没有附加的文件，所以开0的就行
    pdu->uiMsgType = static_cast<uint>(ENUM_MSG_TYPE::ENUM_MSG_TYPE_SELECT_ONLINE_USER_REQUEST);   // 设置消息类型
    TcpClient::getInstance().getSocket().write((char*)pdu, pdu->uiPDULen);    // 将查询在线用户的请求发出去
    free(pdu);
    pdu = nullptr;
//    if (ui->onlineUserListWidget->isHidden())     // 如果是隐藏的，点击就显示
//    {
//        ui->onlineUserListWidget->show();
//        ui->addFriendPushButton->show();
//        // 发送给服务器查看所有在线用户的请求，服务器收到请求后开始查询在线用户，再将查到的结果发回给客户端，再显示
//        PDU* pdu = makePDU(0);      // 只需要发请求，没有附加的文件，所以开0的就行
//        pdu->uiMsgType = static_cast<uint>(ENUM_MSG_TYPE::ENUM_MSG_TYPE_SELECT_ONLINE_USER_REQUEST);   // 设置消息类型
//        TcpClient::getInstance().getSocket().write((char*)pdu, pdu->uiPDULen);    // 将查询在线用户的请求发出去
//        free(pdu);
//        pdu = nullptr;
//    }
//    else                                          // 如果是显示的，点击就隐藏
//    {
//        ui->onlineUserListWidget->hide();
//        ui->addFriendPushButton->hide();
//    }
}


// 查找用户槽函数
void mainMenu::on_findPushButton_clicked()
{
    // 利用对话框获取用户输入的用户名，将用户名发送给服务器让服务器查询，然后再返回查询结果
    bool isSearch;
    QString username = QInputDialog::getText(this, "search user", "input username", QLineEdit::Normal, "", &isSearch);   // 初始化对话框，获取用户输入到对话框的用户名
    if (!isSearch)
        return;

    m_username = username;
    PDU* pdu = makePDU(0);
    pdu->uiMsgType = static_cast<uint>(ENUM_MSG_TYPE::ENUM_MSG_TYPE_SEARCH_USER_REQUEST);   // 设置消息类型
    strcpy(pdu->caData, username.toStdString().c_str());    // 将想搜索的用户名拷贝到pdu中

    TcpClient::getInstance().getSocket().write((char*)pdu, pdu->uiPDULen);    // 将消息发送给服务器
}

// 添加好友
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
