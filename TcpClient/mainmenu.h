#ifndef MAINMENU_H
#define MAINMENU_H

#include <QWidget>
#include <QString>

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

    void setMyUsername(QString name);
    QString getMyUsername() const;

    QString m_username;

    void setFriend(QStringList sl);

private slots:
    // 显示所有在线用户按钮的槽函数，实现点击转变状态（显示或者不显示）
    void on_showOnlinePushButton_clicked();

    void on_findPushButton_clicked();

    void on_addFriendPushButton_clicked();

    void on_flushPushButton_clicked();

private:
    Ui::mainMenu *ui;

    QString m_myUsername;
};

#endif // MAINMENU_H
