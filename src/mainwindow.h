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
#include "AppInterface.h"
#include "iar_map.h"
#include "st_map.h"

//******************************************************************************
namespace Ui {
class MainWindow;
}

//******************************************************************************
class MainWindow : public QMainWindow
{
    Q_OBJECT

protected:
    bool eventFilter(QObject* obj, QEvent* event);

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void dragEnterEvent(QDragEnterEvent *e);
    void dropEvent(QDropEvent *e);
    void openFile(const QString &fileName);
    void displayObj(XMap * map);
    void displayMap(const QString &object, XMap * map);

private slots:
    void on_btnOpen_clicked();
    void on_chkboxCode_clicked();
    void on_chkboxData_clicked();
    void on_chkboxUnknown_clicked();
    void on_tabObjects_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn);

    void on_chkboxObj_clicked();

    void on_btnFind_clicked();

    void on_edtFind_returnPressed();

private:
    Ui::MainWindow *ui;
    AppInterface appInterface;
    IarMap * iarMap;
    StMap * stMap;
    XMap * currMap;
    QString objFilter;
    QString openedFileName;
};

#endif // MAINWINDOW_H
