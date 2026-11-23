//******************************************************************************
// Table models used to display the map content
//
// Igor Marinescu
//******************************************************************************

//******************************************************************************
// Includes
//******************************************************************************
#include "map_models.h"
#include <QColor>

//******************************************************************************
// Defines
//******************************************************************************
static const QColor COLOR_ALL       (0xff, 0xff, 0xff);
static const QColor COLOR_OBJECT    (0xff, 0xe9, 0xbd);
static const QColor COLOR_CODE      (0xbe, 0xe6, 0xea);
static const QColor COLOR_DATA      (0xff, 0xd9, 0xad);
static const QColor COLOR_UNKNOWN   (0xe0, 0xe0, 0xe0);

static const char ALL_OBJECTS_TEXT[] = "< All >";

//******************************************************************************
//******************************************************************************
ObjectTableModel::ObjectTableModel(QObject * parent) :
    QAbstractTableModel(parent),
    m_map(nullptr){
}

//******************************************************************************
void ObjectTableModel::setMap(const XMap * map){

    beginResetModel();
    m_map = map;
    m_objs.clear();
    if(m_map != nullptr){
        m_objs.reserve(m_map->objMap.size());
        for(XMap::ObjMap::const_iterator it = m_map->objMap.begin(); it != m_map->objMap.end(); ++it)
            m_objs.push_back(&it->second);
    }
    endResetModel();
}

//******************************************************************************
QString ObjectTableModel::objectAt(int row) const{

    if(isAllRow(row) || (row < 1) || (row > (int)m_objs.size()))
        return QString();
    return QString::fromStdString(m_objs[row - 1]->name);
}

//******************************************************************************
int ObjectTableModel::rowCount(const QModelIndex & parent) const{
    if(parent.isValid() || (m_map == nullptr))
        return 0;
    return (int)m_objs.size() + 1 /* < All > */;
}

//******************************************************************************
int ObjectTableModel::columnCount(const QModelIndex & parent) const{
    return parent.isValid() ? 0 : COL_COUNT;
}

//******************************************************************************
QVariant ObjectTableModel::data(const QModelIndex & index, int role) const{

    if(!index.isValid() || (m_map == nullptr))
        return QVariant();

    int row = index.row();
    bool all = isAllRow(row);

    if(role == Qt::BackgroundRole)
        return all ? COLOR_ALL : COLOR_OBJECT;

    if((role != Qt::DisplayRole) && (role != SortRole))
        return QVariant();

    if(index.column() == COL_NAME)
        return all ? QString(ALL_OBJECTS_TEXT) : objectAt(row);

    MapValue code, data, unknown;
    if(all){
        code = m_map->codeSize;
        data = m_map->dataSize;
        unknown = m_map->unknownSize;
    }
    else{
        const ObjEntry * obj = m_objs[row - 1];
        code = obj->code;
        data = obj->data;
        unknown = obj->unknown;
    }

    switch(index.column()){
    case COL_CODE:      return qulonglong(code);
    case COL_DATA:      return qulonglong(data);
    case COL_UNKNOWN:   return qulonglong(unknown);
    default:            return QVariant();
    }
}

//******************************************************************************
QVariant ObjectTableModel::headerData(int section, Qt::Orientation orientation, int role) const{

    if((orientation != Qt::Horizontal) || (role != Qt::DisplayRole))
        return QAbstractTableModel::headerData(section, orientation, role);

    switch(section){
    case COL_NAME:      return tr("Object");
    case COL_CODE:      return tr("Code");
    case COL_DATA:      return tr("Data");
    case COL_UNKNOWN:   return tr("Unknown");
    default:            return QVariant();
    }
}

//******************************************************************************
//******************************************************************************
ObjectSortProxy::ObjectSortProxy(QObject * parent) :
    QSortFilterProxyModel(parent){
    setSortRole(SortRole);
}

//******************************************************************************
bool ObjectSortProxy::lessThan(const QModelIndex & left, const QModelIndex & right) const{

    bool leftAll = ObjectTableModel::isAllRow(left.row());
    bool rightAll = ObjectTableModel::isAllRow(right.row());

    // "< All >" must be the first row for both sort orders
    if(leftAll || rightAll){
        if(leftAll && rightAll)
            return false;
        bool ascending = (sortOrder() == Qt::AscendingOrder);
        return leftAll ? ascending : !ascending;
    }

    return QSortFilterProxyModel::lessThan(left, right);
}

//******************************************************************************
//******************************************************************************
EntryTableModel::EntryTableModel(QObject * parent) :
    QAbstractTableModel(parent),
    m_map(nullptr),
    m_code(true),
    m_data(true),
    m_unknown(true),
    m_addrDigits(8){
}

//******************************************************************************
void EntryTableModel::setMap(const XMap * map){

    beginResetModel();
    m_map = map;
    m_object.clear();
    m_addrDigits = ((m_map != nullptr) && (m_map->maxAddr > 0xFFFFFFFFULL)) ? 16 : 8;
    rebuild();
    endResetModel();
}

//******************************************************************************
void EntryTableModel::setFilter(const QString & object, bool code, bool data, bool unknown){

    beginResetModel();
    m_object = object.toStdString();
    m_code = code;
    m_data = data;
    m_unknown = unknown;
    rebuild();
    endResetModel();
}

//******************************************************************************
int EntryTableModel::totalCount() const{
    return (m_map != nullptr) ? (int)m_map->entMap.size() : 0;
}

//******************************************************************************
bool EntryTableModel::accepts(const MapEntry & entry) const{

    // Object filter
    if(!m_object.empty() && (entry.obj != m_object))
        return false;

    // Type filter
    switch(entry.typ1){
    case MapEntry::CODE:    return m_code;
    case MapEntry::DATA:    return m_data;
    default:                return m_unknown;
    }
}

//******************************************************************************
void EntryTableModel::rebuild(){

    m_rows.clear();
    if(m_map == nullptr)
        return;

    m_rows.reserve(m_map->entMap.size());
    for(std::vector<MapEntry>::size_type i = 0; i < m_map->entMap.size(); ++i){
        if(accepts(m_map->entMap[i]))
            m_rows.push_back((int)i);
    }
}

//******************************************************************************
int EntryTableModel::rowCount(const QModelIndex & parent) const{
    return parent.isValid() ? 0 : (int)m_rows.size();
}

//******************************************************************************
int EntryTableModel::columnCount(const QModelIndex & parent) const{
    return parent.isValid() ? 0 : COL_COUNT;
}

//******************************************************************************
QVariant EntryTableModel::data(const QModelIndex & index, int role) const{

    if(!index.isValid() || (m_map == nullptr) || (index.row() >= (int)m_rows.size()))
        return QVariant();

    const MapEntry & entry = m_map->entMap[m_rows[index.row()]];

    if(role == Qt::BackgroundRole){
        switch(entry.typ1){
        case MapEntry::CODE:    return COLOR_CODE;
        case MapEntry::DATA:    return COLOR_DATA;
        default:                return COLOR_UNKNOWN;
        }
    }

    if((role != Qt::DisplayRole) && (role != SortRole))
        return QVariant();

    switch(index.column()){

    case COL_NAME:
        return QString::fromStdString(entry.name);

    case COL_ADDR:
        if(role == SortRole)
            return qulonglong(entry.addr);
        return "0x" + QString("%1").arg(qulonglong(entry.addr), m_addrDigits, 16, QChar('0')).toUpper();

    case COL_SIZE:
        if(role == SortRole)
            return qulonglong(entry.valid.size ? entry.size : 0);
        if(!entry.valid.size)
            return QString();       // size not available
        return qulonglong(entry.size);

    case COL_TYPE:
        switch(entry.typ1){
        case MapEntry::CODE:    return tr("Code");
        case MapEntry::DATA:    return tr("Data");
        default:                return QString();
        }

    case COL_OBJECT:
        return QString::fromStdString(entry.obj);

    default:
        return QVariant();
    }
}

//******************************************************************************
QVariant EntryTableModel::headerData(int section, Qt::Orientation orientation, int role) const{

    if((orientation != Qt::Horizontal) || (role != Qt::DisplayRole))
        return QAbstractTableModel::headerData(section, orientation, role);

    switch(section){
    case COL_NAME:      return tr("Name");
    case COL_ADDR:      return tr("Address");
    case COL_SIZE:      return tr("Size");
    case COL_TYPE:      return tr("Type");
    case COL_OBJECT:    return tr("Object");
    default:            return QVariant();
    }
}

//******************************************************************************
