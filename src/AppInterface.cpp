//******************************************************************************
// Application Interface
//
// Igor Marinescu
//******************************************************************************

//******************************************************************************
// Includes
//******************************************************************************
#include <QFileDialog>
#include <QApplication>

#include "AppInterface.h"
#include "Windows.h"
#include "debug_cout.h"

//******************************************************************************
AppInterface::AppInterface() :
    confFile("config.cfg"){

}

//******************************************************************************
AppInterface::~AppInterface(){

}

//******************************************************************************
// Save Widget Geometry to the configuratoin
//******************************************************************************
void AppInterface::saveWidgetGeometry(QWidget * wid, const char * category){

    // Do not save geometry if it was modified by loadWidgetGeometry
    if(in_loadWidgetGeometry
    &&(wid == wid_loadWidgetGeometry))
        return;

    // if maximized or minimized save only status
    if(wid->isMinimized())
        confFile.set(category, "Minimized", 1);
    else if(wid->isMaximized())
        confFile.set(category, "Maximized", 1);
    else{
        // Save position and size only if not maximized/minimized
        confFile.set(category, "Left",      wid->pos().x());
        confFile.set(category, "Top",       wid->pos().y());
        confFile.set(category, "Width",     wid->size().width());
        confFile.set(category, "Height",    wid->size().height());
        confFile.set(category, "Maximized", 0);
        confFile.set(category, "Minimized", 0);
    }
}

//******************************************************************************
// Load Widget Geometry from the configuration
//******************************************************************************
void AppInterface::loadWidgetGeometry(QWidget * wid, const char * category){

    int x, y, w, h, min, max;

    in_loadWidgetGeometry = true;
    wid_loadWidgetGeometry = wid;

    // Restore window geometry
    if(confFile.get(category, "Left", &x)
    && confFile.get(category, "Top", &y))
        wid->move(x, y);

    if(confFile.get(category, "Width", &w)
    && confFile.get(category, "Height", &h))
        wid->resize(w, h);

    // Restore status
    min = max = 0;
    confFile.get(category, "Minimized", &min);
    confFile.get(category, "Maximized", &max);
    if(max)
        wid->showMaximized();
    else if(min)
        wid->showMinimized();

    in_loadWidgetGeometry = false;
}

//******************************************************************************
// Messages
//******************************************************************************
int AppInterface::MsgErr(const QString & txt, const QString & info){

    DBG_INFO("AppInterface::MsgErr");
    QMessageBox msgBox;
    msgBox.setText(txt);
    msgBox.setInformativeText(info);
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.setDefaultButton(QMessageBox::Ok);
    msgBox.setIcon(QMessageBox::Critical);
    return msgBox.exec();
}

//******************************************************************************
int AppInterface::MsgWrn(const QString & txt,
                        const QString & info,
                        QMessageBox::StandardButtons btns,
                        QMessageBox::StandardButton btn){

    QMessageBox msgBox;
    msgBox.setText(txt);
    msgBox.setInformativeText(info);
    msgBox.setStandardButtons(btns);
    msgBox.setDefaultButton(btn);
    msgBox.setIcon(QMessageBox::Warning);
    return msgBox.exec();
}

//******************************************************************************
int AppInterface::MsgInf(const QString & txt,
                        const QString & info){

    QMessageBox msgBox;
    msgBox.setText(txt);
    msgBox.setInformativeText(info);
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.setDefaultButton(QMessageBox::Ok);
    msgBox.setIcon(QMessageBox::Information);
    return msgBox.exec();
}

//******************************************************************************
QString AppInterface::SaveDialog(QWidget * parent,
                                 const QString & directory,
                                 const QString & filter,
                                 const QString & extension){

    QStringList fileNames;
    QFileDialog dialog(parent,
                       parent->tr("Save File"),
                       directory,
                       filter);

    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setDefaultSuffix(extension);
    dialog.setAcceptMode(QFileDialog::AcceptSave);

    if(dialog.exec()){
        fileNames = dialog.selectedFiles();
        return fileNames[0];
    }

    return QString();
}

//******************************************************************************
QString AppInterface::OpenDialog(QWidget * parent,
                                 const QString & directory,
                                 const QString & filter,
                                 QString * selFilter){

    return QFileDialog::getOpenFileName(parent,
        parent->tr("Open File"),
        directory,
        filter,
        selFilter);
}

//******************************************************************************
/*
QString AppInterface::OpenDialog2(QWidget * parent,
                                 const QString & directory,
                                 const QString & filter,
                                 QString * selFilter){
    (void) parent;
    (void) directory;
    (void) filter;
    (void) selFilter;

    TCHAR Buffer[300];
    memset(Buffer, 0, sizeof(Buffer));

    OPENFILENAME ofn;
    memset(&ofn, 0, sizeof(ofn));
    ofn.lStructSize = sizeof(OPENFILENAMEA);
    ofn.hwndOwner = (HWND) parent->effectiveWinId();
    ofn.lpstrFilter = NULL; //"Text Files","*.TXT;*.DOC;*.BAK", NULL,NULL
    ofn.lpstrCustomFilter = NULL;
    ofn.nFilterIndex = 0;
    ofn.lpstrFile = Buffer;
    ofn.nMaxFile = 300;
    ofn.lpstrFileTitle = NULL;
    ofn.lpstrInitialDir = NULL;
    ofn.lpstrTitle = NULL;
    ofn.Flags = OFN_ENABLESIZING | OFN_EXPLORER;

    GetOpenFileName(&ofn);
    return "";
}
*/
