//******************************************************************************
// GCC Map File (GNU ld linker output, generated with -Wl,-Map=<file>)
//
// Igor Marinescu
//******************************************************************************
#ifndef GCC_MAP_H
#define GCC_MAP_H

//******************************************************************************
// Includes
//******************************************************************************
#include "map_entry.h"

//******************************************************************************
//******************************************************************************
class GccMap : public XMap{

public:
    GccMap();
    virtual ~GccMap();

    virtual bool parse(std::istream & in);

private:

    // One symbol listed below an input section
    struct Symbol{
        std::string     name;
        MapValue        addr;
    };

    // Input section currently being collected (symbols follow the section)
    struct InSection{
        bool                active;
        std::string         name;       // input section name, e.g. .text.main
        std::string         outName;    // output section name, e.g. .text
        MapValue            addr;
        MapValue            size;
        std::string         obj;
        std::vector<Symbol> symbols;

        InSection(){ clear(); }
        void clear();
    };

    // Convert the collected input section and its symbols to map entries
    void flushSection(InSection & sec);

    // Classify an input/output section as code or data
    static MapEntry::Type sectionType(const std::string & inName, const std::string & outName);

    // true for output sections not loaded to target memory (debug info, ...)
    static bool isNonAllocSection(const std::string & outName);

    // Object file name without path, e.g. "libc.a(abort.o)" or "main.c.o"
    static std::string objectName(const std::string & path);
};

//******************************************************************************
#endif // GCC_MAP_H
