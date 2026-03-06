#include "tcpclient.h"
#include "ui_tcpclient.h"
#include <QFile>      // 用来读取配置文件中的连接信息，IP、端口号
#include <QByteArray>
#include <QDebug>
#include <QMessageBox>
#include <QHostAddress>
#include "protocol.h"

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

//// 点击发送按钮后进行数据发送
//void TcpClient::on_sendQPushButton_clicked()
//{
//    QString strMsg = ui->lineEdit->text();     // 获取输入框的文本
//    if (strMsg.isEmpty())
//    {
//        QMessageBox::warning(this, "message send", "message is not allow null");
//    }
//    PDU* pdu = makePDU(strMsg.size());         // 创建一个PDU，用来发送消息类型
//    pdu->uiMsgType = 8888;                     // 设置PDU的消息类型
//    memcpy( pdu->caMsg, strMsg.toStdString().c_str(), strMsg.size());  // 将要发送的数据拷贝到pdu里面
//    m_tcpSocket.write((char*)pdu, pdu->uiPDULen);      // 发送数据
//    free(pdu);        // 释放内存
//    pdu = nullptr;
//}


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

void TcpClient::recvMsg()
{
    qDebug() << m_tcpSocket.bytesAvailable();
    uint uiPDULen = 0;
    m_tcpSocket.read((char*)&uiPDULen, sizeof(uint));    // 先读取总长度4个字节出来，到uiPDULen
    uint uiMsglen = uiPDULen - sizeof(PDU);        // 得到消息长度
    PDU* pdu = makePDU(uiMsglen);                  // 根据消息长度构造出pdu
    // (char*)pdu + sizeof(uint):前四个字节之前已经读完了，所以要读到pud偏移之前读的位置上
    // uiPDULen - sizeof(uint):之前读的不用读了，所以要减去
    m_tcpSocket.read((char*)pdu + sizeof(uint), uiPDULen - sizeof(uint));
    // qDebug() << pdu->uiMsgType << username << password;

    switch (pdu->uiMsgType)
    {
        case static_cast<uint>(ENUM_MSG_TYPE::ENUM_MSG_TYPE_REGISTER_RESPOND):   // 注册请求
        {
            qDebug() << strcmp(pdu->caData, REGISTER_SUCCESSED);
            qDebug() << strcmp(pdu->caData, REGISTER_FAILED);

            if (strcmp(pdu->caData, REGISTER_SUCCESSED) == 0)    // 收到服务器传来的注册成功的消息
            {
                QMessageBox::information(this, "register info", REGISTER_SUCCESSED);
            }
            else if (strcmp(pdu->caData, REGISTER_FAILED) == 0)       // 收到服务器传来的注册失败的消息
            {
                QMessageBox::information(this, "register info", REGISTER_FAILED);
            }
        }
    }
    free(pdu);
    pdu = nullptr;
}
