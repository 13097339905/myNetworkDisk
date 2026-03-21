#ifndef MAINMENU_H
#define MAINMENU_H

#include <QWidget>
#include <QString>
#include "privatechat.h"
#include <QMap>
#include <QListWidgetItem>
#include <QFileDialog>
#include "sharefile.h"

namespace Ui {
class mainMenu;
}

// 将主菜单设置成单例模式，只需要一个对象用来显示，只能登录一个用户
class mainMenu : public QWidget
{
    Q_OBJECT

private:
    explicit mainMenu(QWidget *parent = nullptr);
    ~mainMenu();
    mainMenu(const mainMenu& m) = delete;
    mainMenu& operator=(const mainMenu& m) = delete;

public:
    static mainMenu& getInstance();

    // 设置显示在线用户
    void setOnlineUser(QStringList sl);
    void setOnlineUser(QString s);

    // 在登录完成后设置自己的用户名，保存起来
    void setMyUsername(QString name);

    // 获取用户名
    QString getMyUsername() const;

    // 设置显示在线的好友
    void setFriend(QString s);

    QMap<QString, privateChat*> m_privateChatMap;    // 存储当前客户端开的私聊窗口

    void setGroup(QString username, QString msg);

    // 设置文件信息
    void setFileInfo(bool isDir, QString fileName, long long fileSize);

    // 释放刷新的信号，去触发刷新的槽函数
    void emitFlushFileSignal();

    // 获取要上传的文件的路径
    QString getUploadFilePath() const;

    // 获取要保存的路径
    QString getDownloadFilePath() const;

private slots:
    // 显示所有在线用户按钮的槽函数
    void on_showOnlinePushButton_clicked();

    // 查找用户槽函数
    void on_findPushButton_clicked();

    // 添加好友槽函数
    void on_addFriendPushButton_clicked();

    // 刷新在线好友槽函数
    void on_flushPushButton_clicked();

    void on_deletePushButton_clicked();

    void on_privateChatPushButton_clicked();

    void on_sendPushButton_clicked();

    void on_newFolderPushButton_clicked();

    void on_flushFilePushButton_clicked();

    void on_deleteFilePushButton_clicked();

    void on_reNamePushButton_clicked();

    void on_fileListWidget_itemDoubleClicked(QListWidgetItem *item);

    void on_returnPushButton_clicked();

    void on_uploadPushButton_clicked();

    void on_downloadPushButton_clicked();

    void on_shareFilePushButton_clicked();

private:
    Ui::mainMenu *ui;

    QString m_myUsername;   // 在登录完成到主页面后保存自己用户名

    QString m_uploadFilePath;   // 要上传的文件的路径

    QString m_downloadFilePath;    // 下载文件到哪里的路径
};

#endif // MAINMENU_H
