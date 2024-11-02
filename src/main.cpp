#include "mainwindow.h"
#include <QApplication>
#include "debug_cout.h"

int main(int argc, char *argv[])
{
    DBG_ENTER("main");
    DBG_INFO("main:: argc="<<argc);
    if(argc >= 1){
        DBG_INFO("main:: [0]="<<argv[0]);
    }
    if(argc >= 2){
        DBG_INFO("main:: [1]="<<argv[1]);
    }

    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
