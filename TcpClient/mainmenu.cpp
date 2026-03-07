#include "mainmenu.h"
#include "ui_mainmenu.h"

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


// 显示所有在线用户按钮的槽函数，实现点击转变状态（显示或者不显示）
void mainMenu::on_showOnlinePushButton_clicked()
{
    if (ui->onlineUserListWidget->isHidden())     // 如果是隐藏的，点击就显示
    {
        ui->onlineUserListWidget->show();
        ui->addFriendPushButton->show();
    }
    else                                          // 如果是显示的，点击就隐藏
    {
        ui->onlineUserListWidget->hide();
        ui->addFriendPushButton->hide();
    }

}


