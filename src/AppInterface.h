//******************************************************************************
// Application Interface
//
// Igor Marinescu
//******************************************************************************
#ifndef APPINTERFACE_H
#define APPINTERFACE_H

//******************************************************************************
// Includes
//******************************************************************************
#include "ConfFile.h"
#include <QWidget>
#include <QMessageBox>

//******************************************************************************
// Class
//******************************************************************************
class AppInterface{

    bool in_loadWidgetGeometry;
    QWidget * wid_loadWidgetGeometry;

public:

    ConfFile confFile;

    AppInterface();
    virtual ~AppInterface();

    void saveWidgetGeometry(QWidget * wid, const char * category);
    void loadWidgetGeometry(QWidget * wid, const char * category);

    // Messages
    static int MsgErr(const QString & txt, const QString & info);
    static int MsgWrn(const QString & txt, const QString & info,
                QMessageBox::StandardButtons btns,
                QMessageBox::StandardButton  btn);
    static int MsgInf(const QString & txt, const QString & info = "");

    // Open/Save dialogs
    QString SaveDialog(QWidget * parent,
                       const QString & directory,
                       const QString & filter,
                       const QString & extension);
    QString OpenDialog(QWidget * parent,
                       const QString & directory,
                       const QString & filter,
                       QString * selFilter = NULL);

/*
    QString OpenDialog2(QWidget * parent,
                       const QString & directory,
                       const QString & filter,
                       QString * selFilter = NULL);
*/
};

//******************************************************************************
#endif // APPINTERFACE_H
