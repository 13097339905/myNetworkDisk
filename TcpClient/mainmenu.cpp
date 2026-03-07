#include "mainmenu.h"
#include "ui_mainmenu.h"
#include "protocol.h"
#include "tcpclient.h"

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
    ui->onlineUserListWidget->addItems(qs);
}



// 显示所有在线用户按钮的槽函数
void mainMenu::on_showOnlinePushButton_clicked()
{
    if (ui->onlineUserListWidget->isHidden())     // 如果是隐藏的，点击就显示
    {
        ui->onlineUserListWidget->show();
        ui->addFriendPushButton->show();
        // 发送给服务器查看所有在线用户的请求，服务器收到请求后开始查询在线用户，再将查到的结果发回给客户端，再显示
        PDU* pdu = makePDU(0);      // 只需要发请求，没有附加的文件，所以开0的就行
        pdu->uiMsgType = static_cast<uint>(ENUM_MSG_TYPE::ENUM_MSG_TYPE_SELECT_ONLINE_USER_REQUEST);   // 设置消息类型
        TcpClient::getInstance().getSocket().write((char*)pdu, pdu->uiPDULen);    // 将查询在线用户的请求发出去
        free(pdu);
        pdu = nullptr;
    }
    else                                          // 如果是显示的，点击就隐藏
    {
        ui->onlineUserListWidget->hide();
        ui->addFriendPushButton->hide();
        ui->onlineUserListWidget->clear();
    }
}


