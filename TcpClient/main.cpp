#include "tcpclient.h"

#include <QApplication>
#include <mainmenu.h>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    mainMenu m;
    m.show();

//    TcpClient w;
//    w.show();
    return a.exec();
}
