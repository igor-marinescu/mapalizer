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

public:

    ConfFile confFile;

    // Loads the configuration from the user's application data directory.
    // QCoreApplication must exist and applicationName must be set.
    AppInterface();
    virtual ~AppInterface();

    // Write the configuration file. Returns false on error.
    bool saveConfig();

    void saveWidgetGeometry(const QWidget * wid, const char * category);
    void loadWidgetGeometry(QWidget * wid, const char * category);

    // Messages
    static int MsgErr(QWidget * parent, const QString & txt, const QString & info = QString());
    static int MsgWrn(QWidget * parent, const QString & txt, const QString & info,
                QMessageBox::StandardButtons btns,
                QMessageBox::StandardButton  btn);
    static int MsgInf(QWidget * parent, const QString & txt, const QString & info = QString());

    // Open/Save dialogs (return an empty string if canceled)
    static QString SaveDialog(QWidget * parent,
                       const QString & directory,
                       const QString & filter,
                       const QString & extension);
    static QString OpenDialog(QWidget * parent,
                       const QString & directory,
                       const QString & filter,
                       QString * selFilter = nullptr);
};

//******************************************************************************
#endif // APPINTERFACE_H
