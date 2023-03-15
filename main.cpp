#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    a.setWindowIcon(QIcon(":image/logo.png"));

    MainWindow w;
    w.showFullScreen();

    return a.exec();
}
