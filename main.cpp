#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;

    QFile styleF;
    styleF.setFileName(":/resources/style.css");
    styleF.open(QFile::ReadOnly);
    QString qssStr = styleF.readAll();
    w.setStyleSheet(qssStr);

    w.show();

    return a.exec();
}
