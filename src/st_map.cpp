//******************************************************************************
// ST Map File: Keil L166 linker/locater output (*.m66) for ST10/C166
// microcontrollers.
//
// Igor Marinescu
//******************************************************************************

//******************************************************************************
// Includes
//******************************************************************************
#include "st_map.h"
#include <istream>
#include <cstring>
#include "debug_cout.h"
#include "utils.h"

//******************************************************************************
// Defines
//******************************************************************************
// Start of the chapter containing the symbols
static const char SYMBOLS_CHAPTER[] = "PUBLIC SYMBOLS OF MODULE";
// Page header repeated inside the chapter
static const char PAGE_HEADER[] = "LINKER/LOCATER L166";
// Form feed (page break)
static const char FORM_FEED = 0x0C;

//******************************************************************************
// true if 'line' starts with the string literal 'prefix'
//******************************************************************************
template<size_t N>
static bool startsWith(const char * line, const char (&prefix)[N]){
    return strncmp(line, prefix, N - 1) == 0;
}

//******************************************************************************
StMap::StMap(){
}

//******************************************************************************
StMap::~StMap(){
}

//******************************************************************************
// Symbol lines of the chapter have the format (header lines and other lines
// starting with a blank are ignored since their first word is not an address):
//
//      VALUE       PUBLIC SYMBOL NAME     REP    TGR  CLASS   SECTION NAME
//      ====================================================================
//      00FA0000H   main                   LABEL  ---  FCODE   ?PR?MAIN
//
// The map file does not contain the symbol size.
//******************************************************************************
bool StMap::parse(std::istream & in){
    DBG_ENTER("StMap::parse");

    clear();

    std::string line;
    std::vector<char> cline;
    char * cptr0;
    char * cptr1;
    bool inSymbolsChapter = false;
    bool symbolsChapterFound = false;

    MapEntry entry;

    // Extract all variables/lines from file
    while(std::getline(in, line)){

        trimLineEnd(line);

        if(line.empty())
            continue;

        // Convert to (modifiable) char array line
        cline.assign(line.begin(), line.end());
        cline.push_back('\0');
        cptr0 = &cline[0];

        // chapter detection
        if(startsWith(cptr0, SYMBOLS_CHAPTER)){
            inSymbolsChapter = true;
            symbolsChapterFound = true;
            continue;
        }

        if(!inSymbolsChapter)
            continue;

        // filter FF (form feed) and page header
        if((cptr0[0] == FORM_FEED) || startsWith(cptr0, PAGE_HEADER))
            continue;

        // any line not starting with a blank ends the chapter
        if(!isBlank(cptr0[0])){
            inSymbolsChapter = false;
            continue;
        }

        // extract entity
        entry.clear();

        // Entity Address
        cptr0 = extractWord(cptr0, &cptr1);
        entry.valid.addr = interpretHex2(cptr1, &entry.addr);
        if(!entry.valid.addr)       // if address not valid (e.g. header)
            continue;               // ignore the rest of the line

        // Entity Name
        cptr0 = extractWord(cptr0, &cptr1);
        if((cptr1 != nullptr) && (*cptr1 != '\0')){
            entry.name = std::string(cptr1);
            entry.valid.name = isValidSymbolName(entry.name);
        }

        // Entity Size: not available in this map format
        entry.valid.size = false;
        entry.size = 0;

        // REP (Entity Type 2)
        cptr0 = extractWord(cptr0, &cptr1);
        if((cptr1 != nullptr) && (*cptr1 != '\0')){
            entry.typ2 = std::string(cptr1);
            entry.valid.typ2 = true;
        }

        // TGR (not used)
        cptr0 = extractWord(cptr0, &cptr1);

        // CLASS (Entity Type). The class name has an optional memory model
        // prefix (N=near, F=far, H=huge, X=xhuge, S=system) and an optional
        // suffix, e.g. FCODE, NDATA0, HCONST - therefore the class is
        // searched inside the name.
        cptr0 = extractWord(cptr0, &cptr1);
        if((cptr1 != nullptr) && (*cptr1 != '\0')){
            entry.valid.typ1 = true;
            if(strstr(cptr1, "CODE") != nullptr)
                entry.typ1 = MapEntry::CODE;
            else if((strstr(cptr1, "DATA") != nullptr)
                 || (strstr(cptr1, "CONST") != nullptr))
                entry.typ1 = MapEntry::DATA;
            else
                entry.typ1 = MapEntry::UNKNOWN;
        }

        // Entity Object (section name)
        cptr1 = extractRest(cptr0);
        entry.valid.obj = ((cptr1 != nullptr) && (*cptr1 != '\0'));
        if(entry.valid.obj)
            entry.obj = std::string(cptr1);

        // Add valid entry to map
        if(entry.valid.name)
            addEntry(entry);
    }

    DBG_EXIT("StMap::parse");
    return symbolsChapterFound;
}

//******************************************************************************
