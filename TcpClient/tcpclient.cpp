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

    connect(&m_tcpSocket, &QTcpSocket::connected, this, [this](){
        QMessageBox::information(this, "connect server", "connect server success");
    });
}

TcpClient::~TcpClient()
{
    delete ui;
}

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


void TcpClient::on_sendQPushButton_clicked()
{
    QString strMsg = ui->lineEdit->text();
    if (strMsg.isEmpty())
    {
        QMessageBox::warning(this, "message send", "message is not allow null");
    }
    PDU* pdu = makePDU(strMsg.size());
    pdu->uiMsgType = 8888;
    memcpy(pdu->caMsg, strMsg.toStdString().c_str(), strMsg.size());
    m_tcpSocket.write((char*)pdu, pdu->uiPDULen);
    free(pdu);
    pdu = nullptr;
}
