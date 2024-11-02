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
#include <QFile>
#include <QTextStream>
#include <QDragEnterEvent>
#include <QMimeData>
#include "Windows.h"

//******************************************************************************
// Custom widget used to select size
class IntTableWidgetItem : public QTableWidgetItem {
    public:
        bool operator <(const QTableWidgetItem &other) const
        {
            return text().toInt() < other.text().toInt();
        }
};

//******************************************************************************
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    iarMap(NULL),
    stMap(NULL),
    currMap(NULL){
    DBG_ENTER("MainWindow::MainWindow");

    ui->setupUi(this);

    appInterface.loadWidgetGeometry(this, "WinPos");

    // set window title
    this->setWindowTitle("mapalizer");
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

/*
    ui->chkboxCode->setStyleSheet("QCheckBox { background-color: #bee6ea }");
    ui->chkboxData->setStyleSheet("QCheckBox { background-color: #ffd9ad }");
    ui->chkboxUnknown->setStyleSheet("QCheckBox { background-color: #f0f0f0 }");
*/

    ui->tabObjects->setVisible(ui->chkboxObj->isChecked());

    displayObj(currMap);
    displayMap(objFilter, currMap);

    qApp->installEventFilter(this);
    DBG_EXIT("MainWindow::MainWindow");
}

//******************************************************************************
MainWindow::~MainWindow(){
    DBG_ENTER("MainWindow::~MainWindow");

    appInterface.confFile.set("Filter", "Objects", (int) ui->chkboxObj->isChecked());
    appInterface.confFile.set("Filter", "Code", (int) ui->chkboxCode->isChecked());
    appInterface.confFile.set("Filter", "Data", (int) ui->chkboxData->isChecked());
    appInterface.confFile.set("Filter", "Unknown", (int) ui->chkboxUnknown->isChecked());

    appInterface.saveWidgetGeometry(this, "WinPos");
    delete ui;

    if(iarMap)
        delete iarMap;

    if(stMap)
        delete stMap;

    DBG_EXIT("MainWindow::~MainWindow");
}

//******************************************************************************
void MainWindow::dragEnterEvent(QDragEnterEvent *e){

    if (e->mimeData()->hasUrls()) {
        e->acceptProposedAction();
    }
}

//******************************************************************************
void MainWindow::dropEvent(QDropEvent *e){

    foreach (const QUrl &url, e->mimeData()->urls()) {

        const QString &fileName = url.toLocalFile();
        openFile(fileName);
    }
}

//******************************************************************************
void MainWindow::on_btnOpen_clicked(){

    // name/filter of last opened file
    QString last_filename = appInterface.confFile.get_qstr("UI", "LastFile");
    QString last_fileFilter = appInterface.confFile.get_qstr("UI", "LastFileFilter");

    // show open file dialog
    QString fileName = appInterface.OpenDialog(this,
                last_filename, "Iar MAP File (*.map);;ST MAP File (*.m66);;Any File (*.*)",
                &last_fileFilter);

    // cancel pressed
    if(fileName.isEmpty())
        return;

    openFile(fileName);

    // save last opened file name/filter
    appInterface.confFile.set_qstr("UI", "LastFile", fileName);
    appInterface.confFile.set_qstr("UI", "LastFileFilter", last_fileFilter);
}

//******************************************************************************
void MainWindow::openFile(const QString &fileName){

    // cancel pressed
    if(fileName.isEmpty())
        return;

    // save last opened file
    appInterface.confFile.set_qstr("UI", "LastFile", fileName);

    // open selected file
    if(iarMap)
        delete iarMap;
    if(stMap)
        delete stMap;
    currMap = nullptr;

    QByteArray ba = fileName.toLatin1();
    if(fileName.endsWith(".M66", Qt::CaseInsensitive)){
        currMap = stMap = new StMap(ba.data());
    }
    else{
        currMap = iarMap = new IarMap(ba.data());
    }

    objFilter.clear();
    displayObj(currMap);
    displayMap(objFilter, currMap);

    // set window title
    openedFileName = fileName;
    this->setWindowTitle("mapalizer - " + openedFileName);
}

//******************************************************************************
void MainWindow::on_chkboxCode_clicked(){
    displayMap(objFilter, currMap);
}
//******************************************************************************
void MainWindow::on_chkboxData_clicked(){
    displayMap(objFilter, currMap);
}
//******************************************************************************
void MainWindow::on_chkboxUnknown_clicked(){
    displayMap(objFilter, currMap);
}
//******************************************************************************
void MainWindow::on_chkboxObj_clicked(){
    ui->tabObjects->setVisible(ui->chkboxObj->isChecked());
}
//******************************************************************************
void MainWindow::on_tabObjects_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn){

    (void) previousRow;
    (void) previousColumn;
    (void) currentColumn;

    objFilter.clear();

    QTableWidgetItem * item = ui->tabObjects->item(currentRow, 0);
    if(item != nullptr){
        objFilter = item->text();
        if(!objFilter.compare("< All >"))
            objFilter.clear();
    }

    displayMap(objFilter, currMap);

    // set window title to current object filter
    QString txt = objFilter;
    if(!txt.isEmpty())
        txt = "::" + txt;
    this->setWindowTitle("mapalizer - " + openedFileName + txt);
}

//******************************************************************************
void MainWindow::displayObj(XMap * mmap){

    QTableWidgetItem * item;
    QColor color(0xff, 0xe9, 0xbd);

    // clear old table content
    ui->tabObjects->clear();
    ui->tabObjects->setRowCount(0);

    // Columns names
    QStringList tabEntriesHeader;
    tabEntriesHeader << tr("Object") << tr("Code") << tr("Data");
    ui->tabObjects->setHorizontalHeaderLabels(tabEntriesHeader);
    ui->tabObjects->setColumnCount(3);
    ui->tabObjects->setColumnWidth(0, 150); // Name
    ui->tabObjects->setColumnWidth(1, 60);  // Address
    ui->tabObjects->setColumnWidth(2, 60);  // Size
    ui->tabObjects->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->tabObjects->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);

    // Table background
    QPalette p = ui->tabObjects->palette();
    p.setColor(QPalette::Base, Qt::gray);
    ui->tabObjects->setPalette(p);

    // return if no data to display
    if(!mmap)
        return;

    ui->tabObjects->setSortingEnabled(false);
    ui->tabObjects->setRowCount(mmap->objCnt + 1 /*<All>*/);

    // All
    item = new QTableWidgetItem("< All >");
    item->setFlags(item->flags() ^ Qt::ItemIsEditable);
    item->setBackground(QColor(0xff, 0xff, 0xff));
    ui->tabObjects->setItem(0, 0, item);

    // Code
    item = new IntTableWidgetItem();
    item->setText(QString::number(mmap->codeSize));
    item->setFlags(item->flags() ^ Qt::ItemIsEditable);
    item->setBackground(QColor(0xff, 0xff, 0xff));
    ui->tabObjects->setItem(0, 1, item);

    // Data
    item = new IntTableWidgetItem();
    item->setText(QString::number(mmap->dataSize));
    item->setFlags(item->flags() ^ Qt::ItemIsEditable);
    item->setBackground(QColor(0xff, 0xff, 0xff));
    ui->tabObjects->setItem(0, 2, item);

    // Show entries
    int idx = 1;
    for(map<string,ObjEntry>::iterator it = mmap->objMap.begin(); it != mmap->objMap.end(); ++it){

        // Name
        item = new QTableWidgetItem(it->second.name.c_str());
        item->setFlags(item->flags() ^ Qt::ItemIsEditable);
        item->setBackground(color);
        ui->tabObjects->setItem(idx, 0, item);

        // Code
        item = new IntTableWidgetItem();
        item->setText(QString::number(it->second.code));
        item->setFlags(item->flags() ^ Qt::ItemIsEditable);
        item->setBackground(color);
        ui->tabObjects->setItem(idx, 1, item);

        // Data
        item = new IntTableWidgetItem();
        item->setText(QString::number(it->second.data));
        item->setFlags(item->flags() ^ Qt::ItemIsEditable);
        item->setBackground(color);
        ui->tabObjects->setItem(idx, 2, item);

        ++idx;
    }

    ui->tabObjects->setSortingEnabled(true);
}

//******************************************************************************
void MainWindow::displayMap(const QString &object, XMap * mmap){

    QTableWidgetItem * item;
    MapEntry * entry;
    QColor color;

    // clear old table content
    ui->tabEntries->clear();
    ui->tabEntries->setRowCount(0);

    // Columns names
    QStringList tabEntriesHeader;
    tabEntriesHeader << tr("Name") << tr("Address") << tr("Size") << tr("Type") << tr("Object");
    ui->tabEntries->setHorizontalHeaderLabels(tabEntriesHeader);
    ui->tabEntries->setColumnCount(5);
    ui->tabEntries->setColumnWidth(0, 170); // Name
    ui->tabEntries->setColumnWidth(1, 80);  // Address
    ui->tabEntries->setColumnWidth(2, 60);  // Size
    ui->tabEntries->setColumnWidth(3, 60);  // Type
    ui->tabEntries->setColumnWidth(4, 120); // Object
//    ui->tabEntries->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->tabEntries->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);

    // Table background
    QPalette p = ui->tabEntries->palette();
    p.setColor(QPalette::Base, Qt::gray);
    ui->tabEntries->setPalette(p);

    // return if no data to display
    if(!mmap)
        return;

    ui->tabEntries->setSortingEnabled(false);

    bool obj_filter = !object.isEmpty();

    // Count number of rows required to display content based on filter
    int count = 0;
    for(std::vector<MapEntry>::size_type i = 0; i != mmap->entMap.size(); i++){
        entry = &mmap->entMap[i];

        if(entry == nullptr)
            continue;

        if(obj_filter){
            if(object.compare(entry->obj.c_str()) != 0)
                continue;
        }

        if(entry->typ1 == MapEntry::CODE){
            if(ui->chkboxCode->isChecked())
                ++count;
        }
        else if(entry->typ1 == MapEntry::DATA){
            if(ui->chkboxData->isChecked())
                ++count;
        }
        else{
            if(ui->chkboxUnknown->isChecked())
                ++count;
        }
    }

    // Show entries
    ui->tabEntries->setRowCount(count);
    int idx = 0;
    for(std::vector<MapEntry>::size_type i = 0; i != mmap->entMap.size(); i++){
        entry = &mmap->entMap[i];

        if(entry == nullptr)
            continue;

        // Check object filter
        if(obj_filter){
            if(object.compare(entry->obj.c_str()) != 0)
                continue;
        }

        // Type
        if(entry->typ1 == MapEntry::CODE){
            if(!ui->chkboxCode->isChecked())
                continue;
            item = new QTableWidgetItem("Code");
            color = QColor(0xbe, 0xe6, 0xea);
        }
        else if(entry->typ1 == MapEntry::DATA){
            if(!ui->chkboxData->isChecked())
                continue;
            item = new QTableWidgetItem("Data");
            color = QColor(0xff, 0xd9, 0xad);
        }
        else{
            if(!ui->chkboxUnknown->isChecked())
                continue;
            item = new QTableWidgetItem("");
            color = QColor(0xe0, 0xe0, 0xe0);
        }
        item->setFlags(item->flags() ^ Qt::ItemIsEditable);
        item->setBackground(color);
        ui->tabEntries->setItem(idx, 3, item);

        // Name
        item = new QTableWidgetItem(entry->name.c_str());
        item->setFlags(item->flags() ^ Qt::ItemIsEditable);
        item->setBackground(color);
        ui->tabEntries->setItem(idx, 0, item);

        // Address
        item = new QTableWidgetItem("0x" + QString("%1").arg(entry->addr, 8, 16, QChar('0')).toUpper());
        item->setFlags(item->flags() ^ Qt::ItemIsEditable);
        item->setBackground(color);
        ui->tabEntries->setItem(idx, 1, item);

        // Size
        item = new IntTableWidgetItem();
        item->setText(QString::number(entry->size, 10));
        item->setFlags(item->flags() ^ Qt::ItemIsEditable);
        item->setBackground(color);
        ui->tabEntries->setItem(idx, 2, item);

        // Object
        item = new QTableWidgetItem(entry->obj.c_str());
        item->setFlags(item->flags() ^ Qt::ItemIsEditable);
        item->setBackground(color);
        ui->tabEntries->setItem(idx, 4, item);

        ++idx;
    }

    ui->tabEntries->setSortingEnabled(true);
}

//******************************************************************************
void MainWindow::on_btnFind_clicked(){

    QTableWidgetItem * item = nullptr;

    // Remove whitespaces from start and end
    QString findTxt = ui->edtFind->text().trimmed();
    if(findTxt.isEmpty() || (ui->tabEntries->rowCount() <= 0))
        return;

    // Go through the rest of the table
    int row = ui->tabEntries->currentRow() + 1;
    while(row < ui->tabEntries->rowCount()){
        item = ui->tabEntries->item(row, 0);
        if(item != nullptr){
            if(item->text().contains(findTxt, Qt::CaseInsensitive))
                break;
        }
        row++;
    }

    // Found?
    if(row < ui->tabEntries->rowCount()){
        ui->tabEntries->setCurrentCell(row, 0);
        return;
    }

    // Not found, start from the beginning, until the current row
    row = 0;
    while(row < ui->tabEntries->currentRow()){
        item = ui->tabEntries->item(row, 0);
        if(item != nullptr){
            if(item->text().contains(findTxt, Qt::CaseInsensitive))
                break;
        }
        row++;
    }

    // Found?
    if(row < ui->tabEntries->rowCount()){
        ui->tabEntries->setCurrentCell(row, 0);
    }
}

void MainWindow::on_edtFind_returnPressed(){
    on_btnFind_clicked();
}

//******************************************************************************
bool MainWindow::eventFilter(QObject* obj, QEvent* event){

    if(event->type()==QEvent::KeyPress){
        QKeyEvent* key = static_cast<QKeyEvent*>(event);

        // Ctrl+F pressed?
        if((key->key() == Qt::Key_F) && (key->modifiers() & Qt::ControlModifier)){
            ui->edtFind->setFocus();
        }

        // F3 pressed? Qt::Key_F3
        if(key->key() == Qt::Key_F3){
            //on_btnFind_clicked();
        }
    }

    return QObject::eventFilter(obj, event);
}
