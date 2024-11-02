//******************************************************************************
// ST Map File
//
// Igor Marinescu
//******************************************************************************
//******************************************************************************
// Includes
//******************************************************************************
#include "st_map.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include "debug_cout.h"
#include "utils.h"

//******************************************************************************
StMap::StMap(const char * filename){
    DBG_ENTER("StMap::StMap");

    objCnt = 0;
    codeSize = 0;
    dataSize = 0;
    unknownSize = 0;

    string line;
    char cline[256];
    char * cptr0 = cline;
    char * cptr1;
    char chapter;
    int ch_line_idx;

    MapEntry entry;
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

        // Convert to char array line
        strncpy_s(cline, 256, line.c_str(), line.length());
        cptr0 = cline;

        // chapter detection
        if(strncmp(cline, "PUBLIC SYMBOLS OF MODULE", 24) == 0){
            chapter = 'E';
            ch_line_idx = 0;
            continue;
        }

        // process inside chapter
        if(chapter == 'E'){

            // entity (always starts with 6 spaces "      ")
            if(strncmp(cline, "      ", 6) == 0){
                // ignore first two lines:
                //      VALUE     PUBLIC SYMBOL NAME               REP   TGR  CLASS   SECTION NAME
                //      ==========================================================================
                ch_line_idx++;
                if(ch_line_idx <= 2)
                    continue;

                // extract entity
                entry.clear();

                // Entity Address
                cptr0 = extractWord(cptr0, &cptr1);
                entry.valid.addr = interpretHex2(cptr1, &entry.addr);
                if(!entry.valid.addr){      // if address not valid
                    continue;               // ignore the rest of the line
                }

                // Entity Name
                cptr0 = extractWord(cptr0, &cptr1);
                if((cptr1 != nullptr) && (*cptr1 != '\0')){
                    entry.name = string(cptr1);
                    entry.valid.name = true;
                }

                // Entity Size (not available)
                entry.valid.size = true;
                entry.size = 0;

                // REP (Entity Type 2)
                cptr0 = extractWord(cptr0, &cptr1);
                if((cptr1 != nullptr) && (*cptr1 != '\0')){
                    entry.typ2 = string(cptr1);
                    entry.valid.typ2 = true;
                }

                // TGR (not used)
                cptr0 = extractWord(cptr0, &cptr1);

                // Entity Type
                cptr0 = extractWord(cptr0, &cptr1);
                if((cptr1 != nullptr) && (*cptr1 != '\0')){
                    entry.valid.typ1 = true;
                    if(!strncmp(&cptr1[1], "CODE", 4))
                        entry.typ1 = MapEntry::CODE;
                    else
                    if((!strncmp(&cptr1[1], "DATA", 4))
                    || (!strncmp(&cptr1[1], "CONST", 5)))
                        entry.typ1 = MapEntry::DATA;
                    else
                        entry.typ1 = MapEntry::UNKNOWN;
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
                         || (entry.name[0] == '?')
                         ||((entry.name[0] >= 'a') && (entry.name[0] <= 'z'))
                         ||((entry.name[0] >= 'A') && (entry.name[0] <= 'Z')))
                            ; // correct name
                    else
                        entry.valid.name = 0;
                }

                // Add valid entry to map
                if(entry.valid.name && entry.valid.addr){
                    DBG_INFO(entry);
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
            }
            // filter FF (form feed) 0x0C
            else if(cline[0] == 0x0C)
                continue;
            // filter "LINKER/LOCATER"
            else if(strncmp(cline, "LINKER/LOCATER L166", 19) == 0)
                continue;
            // exit chapter
            else
                chapter = '.';
        }

    }
    DBG_EXIT("StMap::StMap");
}

//******************************************************************************
StMap::~StMap(){
    DBG_ENTER("StMap::~StMap");

    DBG_EXIT("StMap::~StMap");
}
