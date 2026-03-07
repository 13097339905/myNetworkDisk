#include "tcpclient.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

//    TcpClient w;
//    w.show();

    // TcpClient变成单例模式了，只能这样调用了
    TcpClient::getInstance().show();


    return a.exec();
}
