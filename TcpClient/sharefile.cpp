#include "sharefile.h"
#include "ui_sharefile.h"
#include "tcpclient.h"
#include "protocol.h"

#include <QCheckBox>
#include <QVBoxLayout>
#include <QTimer>
#include <QMessageBox>
#include <QLayoutItem>
#include <cstring>

// 线程安全的懒汉单例模式，等到第一次调用才会创建唯一一个实例化对象
shareFile& shareFile::getInstance()
{
    static shareFile s;   // 因为是static定义一个对象，所以内部保证了线程安全
    return s;
}

// 设置好友列表
void shareFile::setFriendList(QStringList names)
{
    // 从 ui->scrollAreaWidgetContents 获取现有的布局，并转换成 QVBoxLayout
    QVBoxLayout* layout = qobject_cast<QVBoxLayout*>(ui->scrollAreaWidgetContents->layout());

    if (layout == nullptr)   // 如果当前没有布局的话就新建一个布局
    {
        layout = new QVBoxLayout(ui->scrollAreaWidgetContents);   // 创建一个新的垂直布局，并指定父控件
    }
    else
    {
        // 之前就有布局的话，清空布局里所有旧的控件，防止重复添加
        while (layout->count() > 0)
        {
            QLayoutItem* item = layout->takeAt(0);   // 一直取布局里面第0个位置
            if (item == nullptr)    // 如果为空了就break
                break;
            if (item->widget())     // 如果这个元素是一个控件的话
                delete item->widget();     // 删除这个控件，释放内存
            delete item;     // 再删除item
        }
    }

    m_friendChecks.clear();     // 清空之前的好友checkbox
    if (names.isEmpty())        // 当前没有好友可分享，直接return
        return;
    for (const QString& name : names)      // 遍历好友的名字
    {
        QCheckBox* cb = new QCheckBox(name, ui->scrollAreaWidgetContents);  // new一个checkbox出来
        layout->addWidget(cb);        // 加入到垂直布局中
        m_friendChecks.append(cb);    // 加入到m_friendChecks好友checkbox中，方便后面的全选和取消选择操作
    }
    layout->addStretch(1);    // 添加弹簧，让界面好看一点
}

// 设置分享的文件的路径
void shareFile::setShareSourcePath(const QString& path)
{
    m_shareSourcePath = path;
}

shareFile::shareFile(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::shareFile)
{
    ui->setupUi(this);

    // 点击全选
    connect(ui->selectAllPushButton, &QPushButton::clicked, this, [this]() {
        for (auto* cb : m_friendChecks)
            cb->setChecked(true);
    });

    // 点击取消选择
    connect(ui->cancelSelectPushButton, &QPushButton::clicked, this, [this]() {
        for (auto* cb : m_friendChecks)
            cb->setChecked(false);
    });
}

shareFile::~shareFile()
{
    delete ui;
}

// 点击取消触发的槽函数
void shareFile::on_cancelPushButton_clicked()
{
    this->hide();
}

// 点击确定触发的槽函数
void shareFile::on_confirmPushButton_clicked()
{
    QStringList selectedFriends;        // 选中的好友
    for (auto* cb : m_friendChecks)
    {
        if (cb->isChecked())
            selectedFriends.push_back(cb->text());
    }

    if (selectedFriends.isEmpty())      // 没有选中就弹窗
    {
        QMessageBox::information(this, "share file", "please select at least one friend");
        return;
    }

    for (int i = 0; i < selectedFriends.size(); i++)    // 依次发送给服务器，分享的好友的名字和分享的文件路径
    {
        QString curFriend = selectedFriends[i];       // 当前好友名字
        QByteArray path = m_shareSourcePath.toUtf8(); // 需要分享的文件路径

        PDU* pdu = makePDU(path.size() + 1);
        pdu->uiMsgType = static_cast<uint>(ENUM_MSG_TYPE::ENUM_MSG_TYPE_SHARE_FILE_REQUEST);

        memcpy(pdu->caData, curFriend.toStdString().c_str(), 32);
        memcpy(pdu->caMsg, path.constData(), path.size());
        pdu->caMsg[path.size()] = '\0';

        TcpClient::getInstance().getSocket().write((char*)pdu, pdu->uiPDULen);
        free(pdu);
    }
}
