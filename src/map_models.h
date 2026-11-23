//******************************************************************************
// Table models used to display the map content
//
// Igor Marinescu
//******************************************************************************
#ifndef MAP_MODELS_H
#define MAP_MODELS_H

//******************************************************************************
// Includes
//******************************************************************************
#include <QAbstractTableModel>
#include <QSortFilterProxyModel>
#include <vector>
#include "map_entry.h"

//******************************************************************************
// Role used for sorting (raw values: numbers are sorted numerically)
//******************************************************************************
enum { SortRole = Qt::UserRole };

//******************************************************************************
// Objects table: first row "< All >" (totals), then one row per object
//******************************************************************************
class ObjectTableModel : public QAbstractTableModel{

public:

    enum Column { COL_NAME, COL_CODE, COL_DATA, COL_UNKNOWN, COL_COUNT };

    explicit ObjectTableModel(QObject * parent = nullptr);

    // Display the objects of 'map' (nullptr: empty table)
    void setMap(const XMap * map);

    // true for the "< All >" row (source model row)
    static bool isAllRow(int row) { return row == 0; }

    // Object name of the (source model) row, empty for the "< All >" row
    QString objectAt(int row) const;

    int rowCount(const QModelIndex & parent = QModelIndex()) const Q_DECL_OVERRIDE;
    int columnCount(const QModelIndex & parent = QModelIndex()) const Q_DECL_OVERRIDE;
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;

private:
    const XMap * m_map;
    std::vector<const ObjEntry *> m_objs;
};

//******************************************************************************
// Sort proxy keeping the "< All >" row always on top
//******************************************************************************
class ObjectSortProxy : public QSortFilterProxyModel{

public:
    explicit ObjectSortProxy(QObject * parent = nullptr);

protected:
    bool lessThan(const QModelIndex & left, const QModelIndex & right) const Q_DECL_OVERRIDE;
};

//******************************************************************************
// Entries (symbols) table, filtered by object and type
//******************************************************************************
class EntryTableModel : public QAbstractTableModel{

public:

    enum Column { COL_NAME, COL_ADDR, COL_SIZE, COL_TYPE, COL_OBJECT, COL_COUNT };

    explicit EntryTableModel(QObject * parent = nullptr);

    // Display the entries of 'map' (nullptr: empty table)
    void setMap(const XMap * map);

    // Show only entries of 'object' (empty: all objects) and of the
    // selected types
    void setFilter(const QString & object, bool code, bool data, bool unknown);

    // Number of entries in the map (without filter)
    int totalCount() const;

    int rowCount(const QModelIndex & parent = QModelIndex()) const Q_DECL_OVERRIDE;
    int columnCount(const QModelIndex & parent = QModelIndex()) const Q_DECL_OVERRIDE;
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;

private:
    bool accepts(const MapEntry & entry) const;
    void rebuild();

    const XMap * m_map;
    std::vector<int> m_rows;    // indexes into m_map->entMap
    std::string m_object;
    bool m_code;
    bool m_data;
    bool m_unknown;
    int m_addrDigits;           // hex digits used to display addresses
};

//******************************************************************************
#endif // MAP_MODELS_H
