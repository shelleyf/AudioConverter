#include <QApplication>
#include "mainwindow.h"
#include "globalapp.h"
#include "loginwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    //MainWindow w;
    //w.show();
    LoginWindow w;
    w.show();
    w.app=&a;

    return a.exec();
}
