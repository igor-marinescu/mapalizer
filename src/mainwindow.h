//******************************************************************************
// Main Window
//
// Igor Marinescu
//******************************************************************************
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

//******************************************************************************
// Includes
//******************************************************************************
#include <QMainWindow>
#include <QModelIndex>
#include <memory>
#include "AppInterface.h"
#include "map_entry.h"

class ObjectTableModel;
class ObjectSortProxy;
class EntryTableModel;
class QSortFilterProxyModel;
class QLabel;

//******************************************************************************
namespace Ui {
class MainWindow;
}

//******************************************************************************
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    // Load and display a map file. Returns false (and shows an error)
    // if the file cannot be loaded; the previous content is kept then.
    bool openFile(const QString &fileName);

protected:
    void closeEvent(QCloseEvent *e) Q_DECL_OVERRIDE;
    void dragEnterEvent(QDragEnterEvent *e) Q_DECL_OVERRIDE;
    void dropEvent(QDropEvent *e) Q_DECL_OVERRIDE;

private slots:
    void on_btnOpen_clicked();
    void on_chkboxCode_clicked();
    void on_chkboxData_clicked();
    void on_chkboxUnknown_clicked();
    void on_chkboxObj_clicked();
    void on_btnFind_clicked();
    void on_edtFind_returnPressed();

    void objectRowChanged(const QModelIndex &current, const QModelIndex &previous);
    void focusFind();
    void findNext();

private:
    void refreshEntries();
    void updateTitle();
    void saveSettings();

    Ui::MainWindow *ui;
    AppInterface appInterface;
    std::unique_ptr<XMap> currMap;

    ObjectTableModel * objModel;
    ObjectSortProxy * objProxy;
    EntryTableModel * entryModel;
    QSortFilterProxyModel * entryProxy;
    QLabel * lblCount;

    QString objFilter;
    QString openedFileName;
};

#endif // MAINWINDOW_H
