#ifndef OPERATEDB_H
#define OPERATEDB_H

#include <QObject>
#include <QSqlDatabase>    // 用来连接数据库
#include <QSqlQuery>       // 用来查询数据库

#define USER_NOT_EXIST     -1   // 查找到不存在的用户
#define USER_IS_ONLINE      1   // 用户在线
#define USER_NOT_ONLINE     0   // 用户不在线

class OperateDB : public QObject
{
    Q_OBJECT
private:
    explicit OperateDB(QObject *parent = nullptr);   // explicit防止构造函数参数的隐式转换
    OperateDB(const OperateDB&) = delete;
    OperateDB& operator=(const OperateDB&) = delete;
    ~OperateDB();

public:
    // 获取唯一实例
    static OperateDB& getInstance();

    // 初始化数据库连接
    void init();

    // 插入用户信息
    bool insertUserInfo(const QString username, const QString password);

    // 删除用户信息
    bool deleteUserInfo(const QString username, const QString password);

    // 查询用户信息
    bool selectUserInfo(const QString username, const QString password);

    // 更新用户信息
    void updateOnline(const QString username);

    // 查询在线用户
    QString selectOnlineUser();

    // 查找用户
    int searchUser(QString username);

signals:

private:
    QSqlDatabase m_db;     // 用来连接数据库

};

#endif // OPERATEDB_H
