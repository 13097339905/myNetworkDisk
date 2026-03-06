#ifndef OPERATEDB_H
#define OPERATEDB_H

#include <QObject>
#include <QSqlDatabase>    // 用来连接数据库
#include <QSqlQuery>       // 用来查询数据库

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

signals:

private:
    QSqlDatabase m_db;     // 用来连接数据库

};

#endif // OPERATEDB_H
