//******************************************************************************
// IAR Map File (ILINK linker output)
//
// Igor Marinescu
//******************************************************************************

//******************************************************************************
// Includes
//******************************************************************************
#include "iar_map.h"
#include <istream>
#include <cstring>
#include "debug_cout.h"
#include "utils.h"

//******************************************************************************
// Defines
//******************************************************************************
// First word of the chapter title containing the symbols ("*** ENTRY LIST")
static const char ENTRY_CHAPTER[] = "ENTRY";

//******************************************************************************
IarMap::IarMap(){
}

//******************************************************************************
IarMap::~IarMap(){
}

//******************************************************************************
bool IarMap::parse(std::istream & in){
    DBG_ENTER("IarMap::parse");

    clear();

    std::string line;
    std::vector<char> cline;
    char * cptr0;
    char * cptr1;
    bool inEntryChapter = false;
    bool entryChapterFound = false;

    MapEntry entry, prevEntry;

    // Extract all variables/lines from file
    while(std::getline(in, line)){

        trimLineEnd(line);

        if(line.empty())
            continue;

        // remarks
        if(line[0] == '#')
            continue;

        // Convert to (modifiable) char array line
        cline.assign(line.begin(), line.end());
        cline.push_back('\0');
        cptr0 = &cline[0];

        // chapter: "*** CHAPTER NAME"
        if(*cptr0 == '*'){

            // First word: ***
            cptr0 = extractWord(cptr0, &cptr1);
            // Second word (first word of the chapter name). Lines without
            // chapter name ("***" or "*****...") don't change the chapter.
            cptr0 = extractWord(cptr0, &cptr1);
            if((cptr1 != nullptr) && (*cptr1 != '\0')){
                inEntryChapter = (strcmp(cptr1, ENTRY_CHAPTER) == 0);
                if(inEntryChapter)
                    entryChapterFound = true;
            }
            continue;
        }

        if(!inEntryChapter)
            continue;

        entry.clear();

        // Entity Name (a line starting with a blank has no name: it is the
        // continuation of the previous line, see below)
        if(!isBlank(*cptr0)){
            cptr0 = extractWord(cptr0, &cptr1);
            if((cptr1 != nullptr) && (*cptr1 != '\0')){
                entry.name = std::string(cptr1);
                entry.valid.name = isValidSymbolName(entry.name);
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
            entry.typ2 = std::string(cptr1);
            entry.valid.typ2 = true;
        }

        // Entity Object
        cptr1 = extractRest(cptr0);
        entry.valid.obj = ((cptr1 != nullptr) && (*cptr1 != '\0'));
        if(entry.valid.obj)
            entry.obj = std::string(cptr1);

        // In case the new entry has no name but a valid address check the
        // previous entry. If the previous has a valid name but all other
        // fields empty - current entry is the continuation of the previous
        // one (one entry split in two rows because of the long name).
        // Fill the current entry with the name from previous entry.
        if(entry.name.empty()
        && prevEntry.valid.name
        && !prevEntry.valid.addr){
            entry.name = prevEntry.name;
            entry.valid.name = true;
        }

        // Add valid entry to map
        if(entry.valid.name)
            addEntry(entry);

        prevEntry = entry;
    }

    DBG_EXIT("IarMap::parse");
    return entryChapterFound;
}

//******************************************************************************
