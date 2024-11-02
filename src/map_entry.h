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
#include <set>
#include <map>

using namespace std;

//******************************************************************************
class MapEntry{
public:

    typedef enum {
        UNKNOWN,
        DATA,
        CODE
    } Type;

    union {
      struct
      {
        unsigned char name : 1;
        unsigned char addr : 1;
        unsigned char size : 1;
        unsigned char typ1 : 1;
        unsigned char typ2 : 1;
        unsigned char obj  : 1;
      };
      unsigned char value;
    } valid;

    string          name;
    unsigned long   addr;
    unsigned long   size;
    Type            typ1;
    string          typ2;
    string          obj;

public:
    MapEntry();

//    MapEntry& operator=(const MapEntry & other);

    void clear();

    friend std::ostream& operator<<(std::ostream& os, const MapEntry & entry);
};

//******************************************************************************
class ObjEntry{
public:

    string          name;
    unsigned long   code;
    unsigned long   data;
    unsigned long   unknown;

public:
    ObjEntry();


    void clear();

    friend std::ostream& operator<<(std::ostream& os, const ObjEntry & entry);
};

//******************************************************************************
class XMap{

public:

    string filename;
    std::vector<MapEntry>       entMap;
    std::map<string,ObjEntry>   objMap;
    int objCnt;

    unsigned long codeSize;
    unsigned long dataSize;
    unsigned long unknownSize;
};

//******************************************************************************
#endif // MAP_ENTRY_H
