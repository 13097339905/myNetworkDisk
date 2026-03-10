#include "privatechat.h"
#include "ui_privatechat.h"
#include <QDebug>
#include "tcpclient.h"


privateChat::privateChat(QString myUsername, QString username, QWidget *parent) :
    QWidget(parent), m_myUsername(myUsername), m_username(username),
    ui(new Ui::privateChat)
{
    ui->setupUi(this);
    ui->usernameLabel->setText(m_username);
}

privateChat::~privateChat()
{
    delete ui;
}

void privateChat::setRecordTextEdit(QString msg)
{
    ui->recordTextEdit->append(m_username + ":  " + msg);  // 这是对方发的消息显示，所以是m_username
}

// 点击私聊的发送后触发的槽函数
void privateChat::on_sendPushButton_clicked()
{
    QString qsMsg = ui->msgLineEdit->text();   // 获取用户在发送框输入的消息
    ui->msgLineEdit->clear();                  // 获取之后输入框就可以清空了
    QByteArray qbaMsg = qsMsg.toUtf8();        // 将QString转换成QByteArray，因为用户可能输入中文，得到的消息的准确字节数，得用QByteArray
    PDU* pdu = makePDU(qbaMsg.size());         // 这里的size就是字节长度，而不是字符个数
    pdu->uiMsgType = static_cast<uint>(ENUM_MSG_TYPE::ENUM_MSG_TYPE_PRIVATE_CHAT_REQUEST);
    memcpy(pdu->caData, m_myUsername.toStdString().c_str(), 32);       // 将自己的用户名写入caData前32字节
    memcpy(pdu->caData + 32, m_username.toStdString().c_str(), 32);    // 将要私聊的用户名写入caData后32字节
    memcpy(pdu->caMsg, qbaMsg.data(), qbaMsg.size());     // 将用户输入的消息拷贝到caMsg
    TcpClient::getInstance().getSocket().write((char*)pdu, pdu->uiPDULen);   // 将私聊的消息发送给服务器

    ui->recordTextEdit->append(m_myUsername + ":  " + qsMsg);  // 将发送的消息在聊天框

    free(pdu);
    pdu = nullptr;
}
