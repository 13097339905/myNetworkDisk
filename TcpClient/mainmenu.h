#ifndef MAINMENU_H
#define MAINMENU_H

#include <QWidget>

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

private slots:
    // 显示所有在线用户按钮的槽函数，实现点击转变状态（显示或者不显示）
    void on_showOnlinePushButton_clicked();

private:
    Ui::mainMenu *ui;
};

#endif // MAINMENU_H
