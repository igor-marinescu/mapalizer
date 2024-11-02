//******************************************************************************
// IAR Map File
//
// Igor Marinescu
//******************************************************************************
//******************************************************************************
// Includes
//******************************************************************************
#include "iar_map.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include "debug_cout.h"
#include "utils.h"

//******************************************************************************
IarMap::IarMap(const char * filename){
    DBG_ENTER("IarMap::IarMap");

    objCnt = 0;
    codeSize = 0;
    dataSize = 0;
    unknownSize = 0;

    string line;
    char cline[256];
    char * cptr0 = cline;
    char * cptr1;
    char chapter;

    MapEntry entry, prevEntry;
    entMap.reserve(1000);

    this->filename = filename;
    ifstream ifstr(filename);

    // Check if file is open
    if(!ifstr.is_open())
        return;

    // Extract all variables/lines from file
    while(getline(ifstr, line)){

        if(line.empty())
            continue;

        // remarks
        if(line[0] == '#')
            continue;

        // Convert to char array line
        strncpy_s(cline, 256, line.c_str(), line.length());
        cptr0 = cline;

        // chapter
        if(*cptr0 == '*'){

            // First word: ***
            cptr0 = extractWord(cptr0, &cptr1);
            // Second word (chapter name)
            cptr0 = extractWord(cptr0, &cptr1);
            if((cptr1 != nullptr) && (*cptr1 != '\0'))
                chapter = *cptr1;
            continue;
        }

        // "ENTRY" chapter
        if(chapter == 'E'){

            entry.clear();

            // Entity Name
            if(*cptr0 != ' '){                              // entry name should not contain
                cptr0 = extractWord(cptr0, &cptr1);         // leading spaces
                if((cptr1 != nullptr) && (*cptr1 != '\0')){
                    entry.name = string(cptr1);
                    entry.valid.name = true;
                }
            }

            // Entity Address
            cptr0 = extractWord(cptr0, &cptr1);
            entry.valid.addr = interpretHex(cptr1, &entry.addr);

            if(!entry.valid.addr){      // if address not valid
                prevEntry = entry;      // ignore the rest of the line
                continue;
            }

            // Entity Size
            cptr0 = extractWord(cptr0, &cptr1);
            entry.valid.size = interpretHex(cptr1, &entry.size);

            // Entity Type
            if(entry.valid.size)                        // if size not valid, size is missing
                cptr0 = extractWord(cptr0, &cptr1);     // instead the extracted word contains type
            if((cptr1 != nullptr) && (*cptr1 != '\0')){
                entry.valid.typ1 = true;
                if(!strcmp(cptr1, "Code"))
                    entry.typ1 = MapEntry::CODE;
                else if(!strcmp(cptr1, "Data"))
                    entry.typ1 = MapEntry::DATA;
                else
                    entry.typ1 = MapEntry::UNKNOWN;
            }

            // Entity Type 2
            cptr0 = extractWord(cptr0, &cptr1);
            if((cptr1 != nullptr) && (*cptr1 != '\0')){
                entry.typ2 = string(cptr1);
                entry.valid.typ2 = true;
            }

            // Entity Object
            cptr1 = extractRest(cptr0);
            entry.valid.obj = ((cptr1 != nullptr) && (*cptr1 != '\0'));
            if(entry.valid.obj)
                entry.obj = string(cptr1);

            // Check entity name validity
            if(entry.valid.name){
                if(entry.name.empty())
                    entry.valid.name = 0;
                else if((entry.name[0] == '_')
                     || (entry.name[0] == '.')
                     ||((entry.name[0] >= 'a') && (entry.name[0] <= 'z'))
                     ||((entry.name[0] >= 'A') && (entry.name[0] <= 'Z')))
                        ; // correct name
                else
                    entry.valid.name = 0;
            }

            // In case the new entry has an invalid (empty) name but a valid address
            // check the previous entry. If the previous has a valid name
            // but all other fields empty - current entry is the continuation of the
            // previous one (one entry splited in two rows because of the long name).
            // Split the current entry with the name from previos entry.
            if(entry.name.empty()
            && entry.valid.addr
            && prevEntry.valid.name
            && !prevEntry.valid.addr){
                entry.name = prevEntry.name;
                entry.valid.name = 1;
            }

            // Add valid entry to map
            if(entry.valid.name && entry.valid.addr){
                //DBG_INFO(entry);
                entMap.push_back(entry);

                // Add object to set of objects
                if(entry.valid.obj){

                    // Insert object if not already in map
                    std::pair<std::map<string,ObjEntry>::iterator,bool> ret;
                    ObjEntry objEntry;
                    objEntry.name = entry.obj;
                    ret = objMap.insert(std::pair<string,ObjEntry>(objEntry.name, objEntry));

                    // Update object size fields
                    switch(entry.typ1){
                    case MapEntry::CODE:
                        ret.first->second.code += entry.size;
                        codeSize +=entry.size;
                        break;
                    case MapEntry::DATA:
                        ret.first->second.data += entry.size;
                        dataSize +=entry.size;
                        break;
                    default:
                        ret.first->second.unknown += entry.size;
                        unknownSize +=entry.size;
                        break;
                    }

                    // If new object, increment counter
                    if(ret.second)
                        objCnt++;
                }
            }

            prevEntry = entry;
        }

    }
    DBG_EXIT("IarMap::IarMap");
}

//******************************************************************************
IarMap::~IarMap(){
    DBG_ENTER("IarMap::~IarMap");

    DBG_EXIT("IarMap::~IarMap");
}
