/********************************************************************************
** Form generated from reading UI file 'tcpclient.ui'
**
** Created by: Qt User Interface Compiler version 5.12.8
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_TCPCLIENT_H
#define UI_TCPCLIENT_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_TcpClient
{
public:
    QWidget *layoutWidget;
    QVBoxLayout *verticalLayout_2;
    QVBoxLayout *verticalLayout;
    QGridLayout *gridLayout;
    QLabel *label_3;
    QLineEdit *passwordLineEdit;
    QLabel *label;
    QLineEdit *usernameLineEdit;
    QPushButton *loginPushButtone;
    QPushButton *registerPushButton;
    QPushButton *cancellationPushButton;

    void setupUi(QWidget *TcpClient)
    {
        if (TcpClient->objectName().isEmpty())
            TcpClient->setObjectName(QString::fromUtf8("TcpClient"));
        TcpClient->resize(495, 263);
        layoutWidget = new QWidget(TcpClient);
        layoutWidget->setObjectName(QString::fromUtf8("layoutWidget"));
        layoutWidget->setGeometry(QRect(20, 20, 443, 211));
        verticalLayout_2 = new QVBoxLayout(layoutWidget);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        verticalLayout_2->setContentsMargins(0, 0, 0, 0);
        verticalLayout = new QVBoxLayout();
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        gridLayout = new QGridLayout();
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        label_3 = new QLabel(layoutWidget);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        QFont font;
        font.setFamily(QString::fromUtf8("AniMe Vision - MB_EN"));
        font.setPointSize(14);
        label_3->setFont(font);

        gridLayout->addWidget(label_3, 1, 0, 1, 1);

        passwordLineEdit = new QLineEdit(layoutWidget);
        passwordLineEdit->setObjectName(QString::fromUtf8("passwordLineEdit"));
        passwordLineEdit->setFont(font);
        passwordLineEdit->setEchoMode(QLineEdit::Password);

        gridLayout->addWidget(passwordLineEdit, 1, 1, 1, 1);

        label = new QLabel(layoutWidget);
        label->setObjectName(QString::fromUtf8("label"));
        label->setFont(font);

        gridLayout->addWidget(label, 0, 0, 1, 1);

        usernameLineEdit = new QLineEdit(layoutWidget);
        usernameLineEdit->setObjectName(QString::fromUtf8("usernameLineEdit"));
        QFont font1;
        font1.setFamily(QString::fromUtf8("\345\276\256\350\275\257\351\233\205\351\273\221"));
        font1.setPointSize(14);
        usernameLineEdit->setFont(font1);

        gridLayout->addWidget(usernameLineEdit, 0, 1, 1, 1);


        verticalLayout->addLayout(gridLayout);

        loginPushButtone = new QPushButton(layoutWidget);
        loginPushButtone->setObjectName(QString::fromUtf8("loginPushButtone"));
        loginPushButtone->setFont(font);

        verticalLayout->addWidget(loginPushButtone);


        verticalLayout_2->addLayout(verticalLayout);

        registerPushButton = new QPushButton(layoutWidget);
        registerPushButton->setObjectName(QString::fromUtf8("registerPushButton"));
        registerPushButton->setFont(font);

        verticalLayout_2->addWidget(registerPushButton);

        cancellationPushButton = new QPushButton(layoutWidget);
        cancellationPushButton->setObjectName(QString::fromUtf8("cancellationPushButton"));
        cancellationPushButton->setFont(font);

        verticalLayout_2->addWidget(cancellationPushButton);


        retranslateUi(TcpClient);

        QMetaObject::connectSlotsByName(TcpClient);
    } // setupUi

    void retranslateUi(QWidget *TcpClient)
    {
        TcpClient->setWindowTitle(QApplication::translate("TcpClient", "TcpClient", nullptr));
        label_3->setText(QApplication::translate("TcpClient", "\345\257\206   \347\240\201\357\274\232", nullptr));
        label->setText(QApplication::translate("TcpClient", "\347\224\250\346\210\267\345\220\215:", nullptr));
        loginPushButtone->setText(QApplication::translate("TcpClient", "\347\231\273\345\275\225", nullptr));
        registerPushButton->setText(QApplication::translate("TcpClient", "\346\263\250\345\206\214", nullptr));
        cancellationPushButton->setText(QApplication::translate("TcpClient", "\346\263\250\351\224\200", nullptr));
    } // retranslateUi

};

namespace Ui {
    class TcpClient: public Ui_TcpClient {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_TCPCLIENT_H
