#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QCoreApplication::addLibraryPath("./plugins");
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
    MainWindow w;
    w.show();

    return a.exec();
}
