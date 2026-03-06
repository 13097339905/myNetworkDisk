#include "tcpserver.h"
#include "ui_tcpserver.h"
#include "mytcpserver.h"
#include <QFile>      // 用来读取配置文件中的连接信息，IP、端口号
#include <QByteArray>
#include <QDebug>
#include <QMessageBox>
#include <QHostAddress>

TcpServer::TcpServer(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TcpServer)
{
    ui->setupUi(this);
    loadConfig();           // 加载配置
    MyTcpServer::getInstance().listen(QHostAddress(m_strIP), m_usPort);    // 创建服务器对象，开始监听
}

TcpServer::~TcpServer()
{
    delete ui;
}

void TcpServer::loadConfig()
{
    // .qrc就是一份资源清单，Qt会根据这份清单把指定文件嵌入程序，最终通过 :前缀名 进行路径访问
    QFile file(":/server.config");         // 先将文件加入Qt资源系统，再通过:/路径访问，创建一个file对象，里面传入要加载的文件路径
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
