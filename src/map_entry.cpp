//******************************************************************************
// Map File Entry
//
// Igor Marinescu
//******************************************************************************

//******************************************************************************
// Includes
//******************************************************************************
#include "map_entry.h"
#include <ostream>

//******************************************************************************
MapEntry::MapEntry(){
    clear();
}

//******************************************************************************
void MapEntry::clear(){
    valid.clear();
    name.clear();
    addr = 0;
    size = 0;
    typ1 = UNKNOWN;
    typ2.clear();
    obj.clear();
}

//******************************************************************************
std::ostream& operator<<(std::ostream& os, const MapEntry & entry){
    std::ios_base::fmtflags flags = os.flags();    // restore after hex output
    os << '{';
    if(entry.valid.name){ os << entry.name; } os << ';';
    if(entry.valid.addr){ os << "0x" << std::hex << entry.addr; } os << ';';
    if(entry.valid.size){ os << "0x" << std::hex << entry.size; } os << ';';
    if(entry.valid.typ1){ os << std::dec << (int)entry.typ1; } os << ';';
    if(entry.valid.typ2){ os << entry.typ2; } os << ';';
    if(entry.valid.obj ){ os << entry.obj;  }
    os << '}';
    os.flags(flags);
    return os;
}

//******************************************************************************
ObjEntry::ObjEntry(){
    clear();
}

//******************************************************************************
void ObjEntry::clear(){
    name.clear();
    code = 0;
    data = 0;
    unknown = 0;
}

//******************************************************************************
std::ostream& operator<<(std::ostream& os, const ObjEntry & entry){
    std::ios_base::fmtflags flags = os.flags();    // restore after hex output
    os << '{';
    os << entry.name << ';';
    os << "0x" << std::hex << entry.code << ';';
    os << "0x" << std::hex << entry.data << ';';
    os << "0x" << std::hex << entry.unknown << ';';
    os << '}';
    os.flags(flags);
    return os;
}

//******************************************************************************
//******************************************************************************
XMap::XMap(){
    clear();
}

//******************************************************************************
XMap::~XMap(){
}

//******************************************************************************
void XMap::clear(){
    entMap.clear();
    objMap.clear();
    codeSize = 0;
    dataSize = 0;
    unknownSize = 0;
    maxAddr = 0;
}

//******************************************************************************
bool XMap::isValidSymbolName(const std::string & name){

    if(name.empty())
        return false;

    char c = name[0];
    return (c == '_') || (c == '.') || (c == '?') || (c == '$') || (c == '@')
        || ((c >= 'a') && (c <= 'z'))
        || ((c >= 'A') && (c <= 'Z'));
}

//******************************************************************************
void XMap::addEntry(const MapEntry & entry, bool countSize){

    entMap.push_back(entry);

    if(entry.addr > maxAddr)
        maxAddr = entry.addr;

    MapValue size = (countSize && entry.valid.size) ? entry.size : 0;

    // Object entry (inserted if not already in map)
    ObjEntry * objEntry = nullptr;
    if(entry.valid.obj){
        ObjMap::iterator it = objMap.find(entry.obj);
        if(it == objMap.end()){
            ObjEntry newObj;
            newObj.name = entry.obj;
            it = objMap.insert(ObjMap::value_type(entry.obj, newObj)).first;
        }
        objEntry = &it->second;
    }

    // Update object and total size fields
    switch(entry.typ1){
    case MapEntry::CODE:
        if(objEntry) objEntry->code += size;
        codeSize += size;
        break;
    case MapEntry::DATA:
        if(objEntry) objEntry->data += size;
        dataSize += size;
        break;
    default:
        if(objEntry) objEntry->unknown += size;
        unknownSize += size;
        break;
    }
}

//******************************************************************************
