//******************************************************************************
// Map File Entry
//
// Igor Marinescu
//******************************************************************************
#ifndef MAP_ENTRY_H
#define MAP_ENTRY_H

//******************************************************************************
// Includes
//******************************************************************************
#include <string>
#include <vector>
#include <map>
#include <iosfwd>
#include "utils.h"

//******************************************************************************
// One symbol of the map file
//******************************************************************************
class MapEntry{
public:

    enum Type {
        UNKNOWN,
        DATA,
        CODE
    };

    // Validity flags of the fields below
    struct Valid{
        bool name;
        bool addr;
        bool size;
        bool typ1;
        bool typ2;
        bool obj;

        Valid(){ clear(); }
        void clear(){ name = addr = size = typ1 = typ2 = obj = false; }
    };

    Valid           valid;
    std::string     name;
    MapValue        addr;
    MapValue        size;
    Type            typ1;
    std::string     typ2;
    std::string     obj;

    MapEntry();
    void clear();
};

std::ostream& operator<<(std::ostream& os, const MapEntry & entry);

//******************************************************************************
// Accumulated sizes of one object (module/section)
//******************************************************************************
class ObjEntry{
public:

    std::string     name;
    MapValue        code;
    MapValue        data;
    MapValue        unknown;

    ObjEntry();
    void clear();
};

std::ostream& operator<<(std::ostream& os, const ObjEntry & entry);

//******************************************************************************
// Base class of all map file parsers
//******************************************************************************
class XMap{

public:

    typedef std::map<std::string, ObjEntry> ObjMap;

    std::vector<MapEntry>   entMap;
    ObjMap                  objMap;

    // Totals of all entries (with or without object)
    MapValue codeSize;
    MapValue dataSize;
    MapValue unknownSize;

    // Highest address of all entries (used to choose the display width)
    MapValue maxAddr;

    XMap();
    virtual ~XMap();

    // Parse the map file content. Previous content is cleared.
    // Returns false if the content is not recognized as this map format.
    virtual bool parse(std::istream & in) = 0;

    void clear();

protected:

    // true if 'name' starts with a character allowed for a symbol name
    static bool isValidSymbolName(const std::string & name);

    // Add a valid entry to the entry list and update object/total sizes.
    // countSize = false: the size is not added to the object/total sizes
    // (e.g. alias of a symbol already counted).
    void addEntry(const MapEntry & entry, bool countSize = true);

private:
    // not copyable
    XMap(const XMap &);
    XMap & operator=(const XMap &);
};

//******************************************************************************
#endif // MAP_ENTRY_H
