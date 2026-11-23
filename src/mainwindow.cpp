//******************************************************************************
// Main Window
//
// Igor Marinescu
//******************************************************************************

//******************************************************************************
// Includes
//******************************************************************************
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "debug_cout.h"
#include "map_loader.h"
#include "map_models.h"
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QCloseEvent>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QUrl>
#include <QShortcut>
#include <QStatusBar>
#include <QLabel>
#include <sstream>

//******************************************************************************
// Defines
//******************************************************************************
static const char APP_TITLE[] = "mapalizer";
static const char FILE_FILTER[] =
    "IAR MAP File (*.map);;GCC MAP File (*.map);;ST/Keil L166 MAP File (*.m66);;Any File (*.*)";
static const int STATUS_TIMEOUT_MS = 3000;

//******************************************************************************
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    objModel(nullptr),
    objProxy(nullptr),
    entryModel(nullptr),
    entryProxy(nullptr),
    lblCount(nullptr){
    DBG_ENTER("MainWindow::MainWindow");

    ui->setupUi(this);

    appInterface.loadWidgetGeometry(this, "WinPos");

    setAcceptDrops(true);

    // Filter
    int val;
    if(appInterface.confFile.get("Filter", "Objects", &val))
        ui->chkboxObj->setChecked(val != 0);
    if(appInterface.confFile.get("Filter", "Code", &val))
        ui->chkboxCode->setChecked(val != 0);
    if(appInterface.confFile.get("Filter", "Data", &val))
        ui->chkboxData->setChecked(val != 0);
    if(appInterface.confFile.get("Filter", "Unknown", &val))
        ui->chkboxUnknown->setChecked(val != 0);

    // Models
    objModel = new ObjectTableModel(this);
    objProxy = new ObjectSortProxy(this);
    objProxy->setSourceModel(objModel);

    entryModel = new EntryTableModel(this);
    entryProxy = new QSortFilterProxyModel(this);
    entryProxy->setSourceModel(entryModel);
    entryProxy->setSortRole(SortRole);
    entryProxy->setSortCaseSensitivity(Qt::CaseInsensitive);

    // Table background
    QPalette p = ui->tabObjects->palette();
    p.setColor(QPalette::Base, Qt::gray);
    ui->tabObjects->setPalette(p);
    ui->tabEntries->setPalette(p);

    // Objects table
    ui->tabObjects->setModel(objProxy);
    ui->tabObjects->setColumnWidth(ObjectTableModel::COL_NAME,    140);
    ui->tabObjects->setColumnWidth(ObjectTableModel::COL_CODE,     60);
    ui->tabObjects->setColumnWidth(ObjectTableModel::COL_DATA,     60);
    ui->tabObjects->setColumnWidth(ObjectTableModel::COL_UNKNOWN,  60);
    ui->tabObjects->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->tabObjects->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->tabObjects->sortByColumn(ObjectTableModel::COL_NAME, Qt::AscendingOrder);
    ui->tabObjects->setVisible(ui->chkboxObj->isChecked());

    // Entries table
    ui->tabEntries->setModel(entryProxy);
    ui->tabEntries->setColumnWidth(EntryTableModel::COL_NAME,   170);
    ui->tabEntries->setColumnWidth(EntryTableModel::COL_ADDR,    80);
    ui->tabEntries->setColumnWidth(EntryTableModel::COL_SIZE,    60);
    ui->tabEntries->setColumnWidth(EntryTableModel::COL_TYPE,    60);
    ui->tabEntries->setColumnWidth(EntryTableModel::COL_OBJECT, 120);
    ui->tabEntries->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->tabEntries->sortByColumn(EntryTableModel::COL_NAME, Qt::AscendingOrder);

    // Status bar: number of displayed symbols
    lblCount = new QLabel(this);
    statusBar()->addPermanentWidget(lblCount);

    connect(ui->tabObjects->selectionModel(),
            SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),
            this, SLOT(objectRowChanged(QModelIndex,QModelIndex)));

    // Shortcuts: Ctrl+F (find), F3 (find next)
    QShortcut * scFind = new QShortcut(QKeySequence::Find, this);
    connect(scFind, SIGNAL(activated()), this, SLOT(focusFind()));
    QShortcut * scFindNext = new QShortcut(QKeySequence::FindNext, this);
    connect(scFindNext, SIGNAL(activated()), this, SLOT(findNext()));

    refreshEntries();

    DBG_EXIT("MainWindow::MainWindow");
}

//******************************************************************************
MainWindow::~MainWindow(){
    DBG_ENTER("MainWindow::~MainWindow");

    // detach the models before the map is deleted
    objModel->setMap(nullptr);
    entryModel->setMap(nullptr);
    currMap.reset();

    delete ui;

    DBG_EXIT("MainWindow::~MainWindow");
}

//******************************************************************************
void MainWindow::saveSettings(){

    appInterface.confFile.set("Filter", "Objects", ui->chkboxObj->isChecked() ? 1 : 0);
    appInterface.confFile.set("Filter", "Code",    ui->chkboxCode->isChecked() ? 1 : 0);
    appInterface.confFile.set("Filter", "Data",    ui->chkboxData->isChecked() ? 1 : 0);
    appInterface.confFile.set("Filter", "Unknown", ui->chkboxUnknown->isChecked() ? 1 : 0);

    appInterface.saveWidgetGeometry(this, "WinPos");
    appInterface.saveConfig();
}

//******************************************************************************
void MainWindow::closeEvent(QCloseEvent *e){
    saveSettings();
    QMainWindow::closeEvent(e);
}

//******************************************************************************
void MainWindow::dragEnterEvent(QDragEnterEvent *e){

    const QList<QUrl> urls = e->mimeData()->urls();
    for(int i = 0; i < urls.size(); ++i){
        if(urls.at(i).isLocalFile()){
            e->acceptProposedAction();
            return;
        }
    }
}

//******************************************************************************
void MainWindow::dropEvent(QDropEvent *e){

    // Only one map can be displayed: open the first local file
    const QList<QUrl> urls = e->mimeData()->urls();
    for(int i = 0; i < urls.size(); ++i){
        if(urls.at(i).isLocalFile()){
            openFile(urls.at(i).toLocalFile());
            return;
        }
    }
}

//******************************************************************************
void MainWindow::on_btnOpen_clicked(){

    // name/filter of last opened file
    QString last_filename = appInterface.confFile.get_qstr("UI", "LastFile");
    QString last_fileFilter = appInterface.confFile.get_qstr("UI", "LastFileFilter");

    // show open file dialog
    QString fileName = AppInterface::OpenDialog(this,
                last_filename, FILE_FILTER, &last_fileFilter);

    // cancel pressed
    if(fileName.isEmpty())
        return;

    // save last selected filter (the file name is saved by openFile)
    appInterface.confFile.set_qstr("UI", "LastFileFilter", last_fileFilter);

    openFile(fileName);
}

//******************************************************************************
bool MainWindow::openFile(const QString &fileName){

    if(fileName.isEmpty())
        return false;

    // Read the whole file (QFile handles Unicode file names)
    QFile file(fileName);
    if(!file.open(QIODevice::ReadOnly)){
        AppInterface::MsgErr(this, tr("Cannot open file."),
                QDir::toNativeSeparators(fileName) + "\n" + file.errorString());
        return false;
    }
    QByteArray rawContent = file.readAll();
    file.close();
    std::string content(rawContent.constData(), rawContent.size());
    rawContent.clear();

    // Detect format and parse
    MapFormat format = detectMapFormat(content, QFileInfo(fileName).suffix().toStdString());
    std::unique_ptr<XMap> newMap(createMap(format));
    std::istringstream in(content);
    bool recognized = newMap->parse(in);

    if(!recognized || newMap->entMap.empty()){
        AppInterface::MsgErr(this, tr("No symbols found."),
                QDir::toNativeSeparators(fileName) + "\n"
                + tr("The file is not a supported map file (IAR, GCC, ST/Keil L166)."));
        return false;
    }

    // save last opened file
    appInterface.confFile.set_qstr("UI", "LastFile", fileName);

    // Display the new map (models first, then delete the old map)
    objFilter.clear();
    objModel->setMap(newMap.get());
    entryModel->setMap(newMap.get());
    currMap = std::move(newMap);

    openedFileName = fileName;
    refreshEntries();
    return true;
}

//******************************************************************************
void MainWindow::on_chkboxCode_clicked(){
    refreshEntries();
}
//******************************************************************************
void MainWindow::on_chkboxData_clicked(){
    refreshEntries();
}
//******************************************************************************
void MainWindow::on_chkboxUnknown_clicked(){
    refreshEntries();
}
//******************************************************************************
void MainWindow::on_chkboxObj_clicked(){
    ui->tabObjects->setVisible(ui->chkboxObj->isChecked());
}

//******************************************************************************
void MainWindow::objectRowChanged(const QModelIndex &current, const QModelIndex &previous){

    (void) previous;

    objFilter.clear();
    if(current.isValid())
        objFilter = objModel->objectAt(objProxy->mapToSource(current).row());

    refreshEntries();
}

//******************************************************************************
// Apply the object/type filter to the entries table and update title/status
//******************************************************************************
void MainWindow::refreshEntries(){

    entryModel->setFilter(objFilter,
                          ui->chkboxCode->isChecked(),
                          ui->chkboxData->isChecked(),
                          ui->chkboxUnknown->isChecked());

    lblCount->setText(tr("%1 of %2 symbols")
                      .arg(entryModel->rowCount())
                      .arg(entryModel->totalCount()));
    updateTitle();
}

//******************************************************************************
void MainWindow::updateTitle(){

    QString title = APP_TITLE;
    if(!openedFileName.isEmpty())
        title += " - " + QDir::toNativeSeparators(openedFileName);
    if(!objFilter.isEmpty())
        title += "::" + objFilter;
    setWindowTitle(title);
}

//******************************************************************************
void MainWindow::focusFind(){
    ui->edtFind->setFocus();
    ui->edtFind->selectAll();
}

//******************************************************************************
// Find the next entry (after the current row, wrapping around) whose name
// contains the search text
//******************************************************************************
void MainWindow::findNext(){

    // Remove whitespaces from start and end
    QString findTxt = ui->edtFind->text().trimmed();
    int rows = entryProxy->rowCount();
    if(findTxt.isEmpty() || (rows <= 0))
        return;

    int current = ui->tabEntries->currentIndex().row();    // -1 if none

    for(int i = 1; i <= rows; ++i){
        int row = (current + i) % rows;
        QModelIndex idx = entryProxy->index(row, EntryTableModel::COL_NAME);
        if(idx.data().toString().contains(findTxt, Qt::CaseInsensitive)){
            ui->tabEntries->setCurrentIndex(idx);
            ui->tabEntries->scrollTo(idx);
            return;
        }
    }

    statusBar()->showMessage(tr("'%1' not found").arg(findTxt), STATUS_TIMEOUT_MS);
}

//******************************************************************************
void MainWindow::on_btnFind_clicked(){
    findNext();
}

//******************************************************************************
void MainWindow::on_edtFind_returnPressed(){
    findNext();
}

//******************************************************************************
