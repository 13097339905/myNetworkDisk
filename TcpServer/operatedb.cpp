#include "operatedb.h"
#include <QMessageBox>
#include <QDebug>
#include <QSqlError>

OperateDB::OperateDB(QObject *parent) : QObject(parent)
{
    m_db = QSqlDatabase::addDatabase("QMYSQL");      // 添加数据库驱动，是操作什么数据库
    init();
    m_db.open();
}

// 在析构函数中关闭数据库
OperateDB::~OperateDB()
{
    m_db.close();
}

// 单例模式获取唯一实例
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
//    if (m_db.open())
//    {
//        QSqlQuery query;
//        query.exec("select * from userInfo");
//        while(query.next())
//        {
//            QString data = QString("%1, %2, %3, %4").arg(query.value(0).toString()).arg(query.value(1).toString())
//                            .arg(query.value(2).toString()).arg(query.value(3).toString());
//            qDebug() << data;
//        }
//    }
//    else
//    {
//        qDebug() << "数据库连接失败:" << m_db.lastError().text();
//        QMessageBox::critical(nullptr, "connct to database", "connct to database failed");
    //    }
}

bool OperateDB::insertUserInfo(const QString username, const QString password)
{
    QSqlQuery query;
    query.prepare("insert into userinfo (name, pwd) values (:username, :password)");
    query.bindValue(":username", username);
    query.bindValue(":password", password);

    if (!query.exec())
    {
        qDebug() << "插入失败:" << query.lastError().text();
        return false;
    }
    return true;
}

bool OperateDB::deleteUserInfo(const QString username, const QString password)
{
    QSqlQuery query;
    query.prepare("delete from userinfo where name = :username and pwd = :password");
    query.bindValue(":username", username);
    query.bindValue(":password", password);

    if (!query.exec())
    {
        qDebug() << "删除失败:" << query.lastError().text();
        return false;
    }
    return true;
}

bool OperateDB::selectUserInfo(const QString username, const QString password)
{
    QSqlQuery query;
    query.prepare("select * from userinfo where name = :username and pwd = :password and online = :online");
    query.bindValue(":username", username);
    query.bindValue(":password", password);
    query.bindValue(":online", 0);
    query.exec();

    if (!query.next())
    {
        qDebug() << "查询失败:" << query.lastError().text();
        return false;
    }
    return true;
}

void OperateDB::updateOnline(const QString username)
{
    QSqlQuery query;
    query.prepare("select online from userinfo where name = :username");
    query.bindValue(":username", username);
    query.exec();
    query.next();    // 判断是否查询到了
    int online = query.value(0).toInt();

    if (online == 1)
        online = 0;
    else
        online = 1;
    query.prepare("update userinfo set online = :online where name = :username");
    query.bindValue(":online", online);
    query.bindValue(":username", username);
    if (!query.exec())
    {
        qDebug() << "更新失败:" << query.lastError().text();
    }
}

// 不能根据逗号进行分隔开，需要改一下
QString OperateDB::selectOnlineUser()
{
    QSqlQuery query;
    query.prepare("select name from userinfo where online = :online");
    query.bindValue(":online",  1);
    if (!query.exec())
    {
        qDebug() << "查询失败:" << query.lastError().text();
    }
    QString onlineUser;
    while (query.next())
    {
        onlineUser += query.value(0).toString();
        onlineUser += ',';     // 根据,逗号进行分隔
    }
    return onlineUser;
}

// 查找用户
int OperateDB::searchUser(const QString username)
{
    QSqlQuery query;
    query.prepare("select online from userinfo where name = :username");
    query.bindValue(":username", username);
    if (!query.exec())
    {
        qDebug() << "查询失败:" << query.lastError().text();
    }
    if (!query.next())    // 没有查找到该用户
    {
        return USER_NOT_EXIST;
    }
    int online = query.value(0).toInt();
    return online == 1 ? USER_IS_ONLINE : USER_NOT_ONLINE;
}

int OperateDB::addFriendSearch(const QString myUsername, const QString username)
{
    QSqlQuery query;
    query.prepare("select id from userinfo where name = :myUsername");    // 先把自己的id查出来存到myId
    query.bindValue(":myUsername", myUsername);
    if (!query.exec())
    {
        qDebug() << "查询失败:" << query.lastError().text();
    }
    query.next();    // 获取值之前记得调用next
    int myId = query.value(0).toInt();

    query.prepare("select id, online from userinfo where name = :username");   // 查询要加的好友的id和在线状态
    query.bindValue(":username", username);
    if (!query.exec())
    {
        qDebug() << "查询失败:" << query.lastError().text();
    }
    query.next();    // 获取值之前记得调用next
    int id = query.value(0).toInt();
    int online = query.value(1).toInt();

    query.prepare("select * from friendInfo where (id = :myId and friendId = :id) or (id = :id and friendId = :myId)");
    query.bindValue(":myId", myId);
    query.bindValue(":id", id);
    if (!query.exec())
    {
        qDebug() << "查询失败:" << query.lastError().text();
    }
    if (query.next())   // 已经是好友了直接返回
    {
        return EXIST_FRIEND;
    }

    return online == 1 ? USER_IS_ONLINE : USER_NOT_ONLINE;    // 要加的用户是否在线
}

// 插入好友信息
bool OperateDB::insertFriendInfo(const QString myUsername, const QString username)
{
    qDebug() << myUsername << username;
    QSqlQuery query;
    query.prepare("select id from userinfo where name = :myUsername");    // 先把自己的id查出来存到myId
    query.bindValue(":myUsername", myUsername);
    if (!query.exec())
    {
        qDebug() << "查询失败:" << query.lastError().text();
        return false;
    }
    query.next();    // 获取值之前记得调用next
    int myId = query.value(0).toInt();

    query.prepare("select id from userinfo where name = :username");    // 查询要加的好友的id和在线状态
    query.bindValue(":username", username);
    if (!query.exec())
    {
        qDebug() << "查询失败:" << query.lastError().text();
        return false;
    }
    query.next();    // 获取值之前记得调用next
    int id = query.value(0).toInt();

    query.prepare("insert into friendInfo values (:myId, :id)");       // 插入好友信息
    qDebug() << myId << id;
    query.bindValue(":myId", myId);
    query.bindValue(":id", id);
    if (!query.exec())
    {
        qDebug() << "插入失败:" << query.lastError().text();
        return false;
    }
    return true;
}

// 查询当前用户的所有好友返回去
QString OperateDB::selectFriend(const QString username)
{
    QSqlQuery query;
    query.prepare("select id from userinfo where name = :username");    // 先把当前用户的id查出来存到myId
    query.bindValue(":username", username);
    if (!query.exec())
    {
        qDebug() << "查询失败:" << query.lastError().text();
        return "";
    }
    query.next();
    int myId = query.value(0).toInt();

    QString res = "";
    query.prepare("select friendId from friendInfo where id = :myId");     // 查出myId的好友friendId
    query.bindValue(":myId", myId);
    if (!query.exec())
    {
        qDebug() << "查询失败:" << query.lastError().text();
        return "";
    }

    query.prepare("select name, online from userinfo where id in "
                  "((select friendId from friendInfo where id = :myId) union (select id from friendInfo where friendId = :myId))");
    query.bindValue(":myId", myId);
    if (!query.exec())
    {
        qDebug() << "查询失败:" << query.lastError().text();
        return "";
    }
    while (query.next())     // 不能根据逗号进行分隔开，需要改一下
    {
        res += query.value(0).toString();
        res += ' ';
        res += query.value(1).toInt() == 1 ? "1" : "0";
        res += ',';
    }
    return res;
}
