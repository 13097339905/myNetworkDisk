#include "operatedb.h"
#include <QMessageBox>
#include <QDebug>
#include <QSqlError>

OperateDB::OperateDB(QObject *parent) : QObject(parent)
{
    m_db = QSqlDatabase::addDatabase("QMYSQL");      // 添加数据库驱动，是操作什么数据库
}

// 在析构函数中关闭数据库
OperateDB::~OperateDB()
{
    m_db.close();
}

OperateDB &OperateDB::getInstance()
{
    static OperateDB instance;
    return instance;
}

// 连接数据库
void OperateDB::init()
{
    m_db.setHostName("localhost");
    m_db.setPort(3306);
    m_db.setUserName("root");
    m_db.setPassword("root");
    m_db.setDatabaseName("networkdiskdb");
    if (m_db.open())
    {
        QSqlQuery query;
        query.exec("select * from userInfo");
        while(query.next())
        {
            QString data = QString("%1, %2, %3, %4").arg(query.value(0).toString()).arg(query.value(1).toString())
                            .arg(query.value(2).toString()).arg(query.value(3).toString());
            qDebug() << data;
        }
    }
    else
    {
        qDebug() << "数据库连接失败:" << m_db.lastError().text();
        QMessageBox::critical(nullptr, "connct to database", "connct to database failed");
    }
}
