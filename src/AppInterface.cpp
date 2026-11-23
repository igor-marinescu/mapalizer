//******************************************************************************
// Application Interface
//
// Igor Marinescu
//******************************************************************************

//******************************************************************************
// Includes
//******************************************************************************
#include <QFileDialog>
#include <QStandardPaths>
#include <QCoreApplication>
#include <QDir>
#include <QFile>

#include "AppInterface.h"
#include "debug_cout.h"

//******************************************************************************
// Defines
//******************************************************************************
static const char CONFIG_FILE_NAME[] = "config.cfg";

//******************************************************************************
AppInterface::AppInterface(){

    // Configuration is stored in the user's application data directory
    // (independent of the working directory, always writable)
    QString dir = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
    QDir().mkpath(dir);
    QString path = QDir(dir).filePath(CONFIG_FILE_NAME);

    if(QFile::exists(path)){
        confFile.load(path);
    }
    else{
        // Migrate a configuration of an older version (stored in the working
        // directory or next to the executable). Saved to the new location.
        QString legacy = QDir::current().filePath(CONFIG_FILE_NAME);
        if(!QFile::exists(legacy))
            legacy = QDir(QCoreApplication::applicationDirPath()).filePath(CONFIG_FILE_NAME);
        if(QFile::exists(legacy))
            confFile.load(legacy);
        confFile.setFileName(path);
    }
}

//******************************************************************************
AppInterface::~AppInterface(){
}

//******************************************************************************
bool AppInterface::saveConfig(){

    bool ok = confFile.save();
    if(!ok){
        DBG_ERROR("AppInterface::saveConfig failed: " << confFile.fileName().toStdString());
    }
    return ok;
}

//******************************************************************************
// Save Widget Geometry to the configuration
// (position, size and maximized state, see QWidget::saveGeometry)
//******************************************************************************
void AppInterface::saveWidgetGeometry(const QWidget * wid, const char * category){

    // remove values of older versions (Left, Top, Width, ...)
    confFile.clear(category);
    confFile.set_str(category, "Geometry", wid->saveGeometry().toBase64().constData());
}

//******************************************************************************
// Load Widget Geometry from the configuration
// (restoreGeometry also moves a window back to a visible screen)
//******************************************************************************
void AppInterface::loadWidgetGeometry(QWidget * wid, const char * category){

    std::string geometry;
    if(confFile.get_str(category, "Geometry", &geometry) && !geometry.empty())
        wid->restoreGeometry(QByteArray::fromBase64(QByteArray(geometry.c_str())));
}

//******************************************************************************
// Messages
//******************************************************************************
int AppInterface::MsgErr(QWidget * parent, const QString & txt, const QString & info){

    DBG_INFO("AppInterface::MsgErr");
    QMessageBox msgBox(parent);
    msgBox.setText(txt);
    msgBox.setInformativeText(info);
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.setDefaultButton(QMessageBox::Ok);
    msgBox.setIcon(QMessageBox::Critical);
    return msgBox.exec();
}

//******************************************************************************
int AppInterface::MsgWrn(QWidget * parent,
                        const QString & txt,
                        const QString & info,
                        QMessageBox::StandardButtons btns,
                        QMessageBox::StandardButton btn){

    QMessageBox msgBox(parent);
    msgBox.setText(txt);
    msgBox.setInformativeText(info);
    msgBox.setStandardButtons(btns);
    msgBox.setDefaultButton(btn);
    msgBox.setIcon(QMessageBox::Warning);
    return msgBox.exec();
}

//******************************************************************************
int AppInterface::MsgInf(QWidget * parent,
                        const QString & txt,
                        const QString & info){

    QMessageBox msgBox(parent);
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

    QFileDialog dialog(parent,
                       QFileDialog::tr("Save File"),
                       directory,
                       filter);

    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setDefaultSuffix(extension);
    dialog.setAcceptMode(QFileDialog::AcceptSave);

    if(dialog.exec()){
        QStringList fileNames = dialog.selectedFiles();
        if(!fileNames.isEmpty())
            return fileNames.first();
    }

    return QString();
}

//******************************************************************************
QString AppInterface::OpenDialog(QWidget * parent,
                                 const QString & directory,
                                 const QString & filter,
                                 QString * selFilter){

    return QFileDialog::getOpenFileName(parent,
        QFileDialog::tr("Open File"),
        directory,
        filter,
        selFilter);
}

//******************************************************************************
