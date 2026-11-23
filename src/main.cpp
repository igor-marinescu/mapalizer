//******************************************************************************
// mapalizer - parser and viewer of linker-generated map files
//
// Usage: mapalizer [map file]
//
// Igor Marinescu
//******************************************************************************

//******************************************************************************
// Includes
//******************************************************************************
#include "mainwindow.h"
#include <QApplication>
#include <QStringList>
#include "debug_cout.h"

//******************************************************************************
int main(int argc, char *argv[])
{
#ifdef DEBUGCOUT
    DebugCout debugCout;    // redirect std::cout to debug.txt
#endif

    DBG_ENTER("main");
    DBG_INFO("main:: argc=" << argc);
    for(int i = 0; i < argc; ++i){
        DBG_INFO("main:: [" << i << "]=" << argv[i]);
    }

    QApplication a(argc, argv);
    QApplication::setApplicationName("mapalizer");

    MainWindow w;
    w.show();

    // Open the file passed on the command line ("Open with", drag on exe)
    QStringList args = QApplication::arguments();
    if(args.size() >= 2)
        w.openFile(args.at(1));

    int ret = a.exec();
    DBG_EXIT("main");
    return ret;
}
