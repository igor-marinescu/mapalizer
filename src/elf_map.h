//******************************************************************************
// ELF File (executable or object file, e.g. *.elf, *.axf, *.out, *.o):
// symbols are read from the symbol table (.symtab).
//
// Igor Marinescu
//******************************************************************************
#ifndef ELF_MAP_H
#define ELF_MAP_H

//******************************************************************************
// Includes
//******************************************************************************
#include "map_entry.h"

//******************************************************************************
//******************************************************************************
class ElfMap : public XMap{

public:
    ElfMap();
    virtual ~ElfMap();

    virtual bool parse(std::istream & in);

    // true if 'content' starts with the ELF magic number
    static bool isElf(const std::string & content);

private:

    // One section header (only the fields needed)
    struct Section{
        std::string     name;
        unsigned long   type;
        MapValue        flags;
        MapValue        offset;
        MapValue        size;
        unsigned long   link;
        MapValue        entsize;
    };

    std::vector<unsigned char>  m_data;     // whole file
    bool                        m_is64;
    bool                        m_bigEndian;

    // Read an unsigned value of 'bytes' (1, 2, 4 or 8) at 'offset'.
    // Returns false if the value is outside of the file.
    bool read(MapValue offset, int bytes, MapValue * out) const;

    // Null terminated string at 'offset' of the string table 'strtab'
    std::string readString(const Section & strtab, MapValue offset) const;

    bool readSections(std::vector<Section> & sections);
    void readSymbols(const std::vector<Section> & sections, unsigned symtabIdx);
};

//******************************************************************************
#endif // ELF_MAP_H
