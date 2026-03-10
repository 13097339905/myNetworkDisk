#ifndef PRIVATECHAT_H
#define PRIVATECHAT_H

#include <QWidget>
#include "protocol.h"

namespace Ui {
class privateChat;
}

class privateChat : public QWidget
{
    Q_OBJECT

public:
    explicit privateChat(QString myUsername, QString username, QWidget *parent = nullptr);
    ~privateChat();

    void setRecordTextEdit(QString msg);

private slots:
    void on_sendPushButton_clicked();

private:
    Ui::privateChat *ui;

    QString m_myUsername;    // 我的用户名
    QString m_username;      // 私聊对象的用户名
};

#endif // PRIVATECHAT_H
