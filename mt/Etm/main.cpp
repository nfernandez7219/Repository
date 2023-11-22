#include "mainwindow.h"

#include <QApplication>
#include "client.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    client clientapp;
    clientapp.Connect();

    return a.exec();
}
