#include "qticallymainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{

    QApplication a(argc, argv);
    QticallyMainWindow w;
    w.show();
    return a.exec();
}
