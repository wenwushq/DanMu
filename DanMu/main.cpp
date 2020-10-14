#include "DanMu.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    DanMu w;
    w.show();
    return a.exec();
}
