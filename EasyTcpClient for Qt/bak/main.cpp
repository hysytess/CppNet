#include "easytcpclientwidget.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    EasyTcpClientWidget w;
    w.show();
    return a.exec();
}
