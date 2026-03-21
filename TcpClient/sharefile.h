#ifndef SHAREFILE_H
#define SHAREFILE_H

#include <QWidget>
#include <QStringList>
#include <QList>

class QCheckBox;

namespace Ui {
class shareFile;
}

// 将分享文件设置成单例模式，只需要一个对象用来显示
class shareFile : public QWidget
{
    Q_OBJECT

public:
    // 线程安全的懒汉单例模式，等到第一次调用才会创建唯一一个实例化对象，并且因为是static定义一个对象，所以内部保证了线程安全
    static shareFile& getInstance();

    // 设置好友列表
    void setFriendList(QStringList s);

    // 设置分享的文件的路径
    void setShareSourcePath(const QString& path);

private slots:
    void on_cancelPushButton_clicked();

    void on_confirmPushButton_clicked();

private:
    explicit shareFile(QWidget *parent = nullptr);
    ~shareFile();
    shareFile(const shareFile& s) = delete;
    shareFile& operator=(const shareFile& s) = delete;

private:
    Ui::shareFile *ui;

    QString m_shareSourcePath;           // 需要分享的文件的路径
    QList<QCheckBox*> m_friendChecks;    // 存放当前有哪些好友的checkbox
};

#endif // SHAREFILE_H
