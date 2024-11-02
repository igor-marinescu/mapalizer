//******************************************************************************
// Map File Entry
//
// Igor Marinescu
//******************************************************************************

//******************************************************************************
// Includes
//******************************************************************************
#include "map_entry.h"
#include "debug_cout.h"

//******************************************************************************
MapEntry::MapEntry(){
    clear();
}

//******************************************************************************
void MapEntry::clear(){
    valid.value = 0;
    name.clear();
    addr = 0UL;
    size = 0UL;
    typ1 = UNKNOWN;
    typ2.clear();
    obj.clear();
}
//******************************************************************************
std::ostream& operator<<(std::ostream& os, const MapEntry & entry){
    os << '{';
    if(entry.valid.name){ os << entry.name; } os << ';';
    if(entry.valid.addr){ os << "0x" << std::hex << entry.addr; } os << ';';
    if(entry.valid.size){ os << "0x" << std::hex << entry.size; } os << ';';
    if(entry.valid.typ1){ os << (int)entry.typ1; } os << ';';
    if(entry.valid.typ2){ os << entry.typ2; } os << ';';
    if(entry.valid.obj ){ os << entry.obj;  }
    os << '}';
    return os;
}
//******************************************************************************
ObjEntry::ObjEntry(){
    clear();
}

//******************************************************************************
void ObjEntry::clear(){
    name.clear();
    code = 0UL;
    data = 0UL;
    unknown = 0UL;
}
//******************************************************************************
std::ostream& operator<<(std::ostream& os, const ObjEntry & entry){
    os << '{';
    os << entry.name << ';';
    os << "0x" << std::hex << entry.code << ';';
    os << "0x" << std::hex << entry.data << ';';
    os << "0x" << std::hex << entry.unknown << ';';
    os << '}';
    return os;
}

//******************************************************************************
