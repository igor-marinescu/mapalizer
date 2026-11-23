//******************************************************************************
// ELF File (executable or object file, e.g. *.elf, *.axf, *.out, *.o):
// symbols are read from the symbol table (.symtab).
//
// Igor Marinescu
//******************************************************************************

//******************************************************************************
// Includes
//******************************************************************************
#include "elf_map.h"
#include <istream>
#include <iterator>
#include <algorithm>
#include <set>
#include "debug_cout.h"
#include "utils.h"

//******************************************************************************
// Defines (ELF specification)
//******************************************************************************
// e_ident
static const unsigned char ELF_MAGIC[] = { 0x7F, 'E', 'L', 'F' };
static const int EI_CLASS       = 4;
static const int EI_DATA        = 5;
static const int ELFCLASS32     = 1;
static const int ELFCLASS64     = 2;
static const int ELFDATA2LSB    = 1;
static const int ELFDATA2MSB    = 2;

// e_machine
static const unsigned long EM_ARM = 40;

// Section types
static const unsigned long SHT_SYMTAB       = 2;
static const unsigned long SHT_DYNSYM       = 11;
static const unsigned long SHT_SYMTAB_SHNDX = 18;

// Section flags
static const MapValue SHF_ALLOC     = 0x2;
static const MapValue SHF_EXECINSTR = 0x4;

// Special section indexes
static const unsigned long SHN_UNDEF     = 0;
static const unsigned long SHN_LORESERVE = 0xFF00;
static const unsigned long SHN_ABS       = 0xFFF1;
static const unsigned long SHN_XINDEX    = 0xFFFF;

// Symbol types
static const int STT_NOTYPE     = 0;
static const int STT_OBJECT     = 1;
static const int STT_FUNC       = 2;
static const int STT_COMMON     = 5;
static const int STT_TLS        = 6;
static const int STT_GNU_IFUNC  = 10;

// Symbol bindings
static const int STB_LOCAL      = 0;
static const int STB_GLOBAL     = 1;
static const int STB_WEAK       = 2;

//******************************************************************************
// true for ARM/AArch64 mapping symbols ($a, $t, $d, $x, optionally followed
// by ".<text>"): they mark the start of code/data, but are not objects
//******************************************************************************
static bool isMappingSymbol(const std::string & name){
    if((name.size() < 2) || (name[0] != '$'))
        return false;
    char c = name[1];
    if((c != 'a') && (c != 't') && (c != 'd') && (c != 'x'))
        return false;
    return (name.size() == 2) || (name[2] == '.');
}

//******************************************************************************
ElfMap::ElfMap() :
    m_is64(false),
    m_bigEndian(false){
}

//******************************************************************************
ElfMap::~ElfMap(){
}

//******************************************************************************
bool ElfMap::isElf(const std::string & content){
    return (content.size() >= sizeof(ELF_MAGIC))
        && (content.compare(0, sizeof(ELF_MAGIC),
                            (const char *)ELF_MAGIC, sizeof(ELF_MAGIC)) == 0);
}

//******************************************************************************
bool ElfMap::read(MapValue offset, int bytes, MapValue * out) const{

    MapValue fileSize = (MapValue)m_data.size();
    if((offset > fileSize) || ((MapValue)bytes > fileSize - offset))
        return false;

    const unsigned char * p = &m_data[(size_t)offset];
    MapValue val = 0;
    for(int i = 0; i < bytes; ++i){
        int idx = m_bigEndian ? i : (bytes - 1 - i);
        val = (val << 8) | p[idx];
    }
    *out = val;
    return true;
}

//******************************************************************************
std::string ElfMap::readString(const Section & strtab, MapValue offset) const{

    MapValue fileSize = (MapValue)m_data.size();
    if((offset >= strtab.size) || (strtab.offset > fileSize))
        return std::string();

    MapValue start = strtab.offset + offset;
    MapValue end = strtab.offset + strtab.size;
    if(end > fileSize)
        end = fileSize;

    std::string str;
    for(MapValue i = start; (i < end) && (m_data[(size_t)i] != 0); ++i)
        str += (char)m_data[(size_t)i];
    return str;
}

//******************************************************************************
// Read all section headers and their names
//******************************************************************************
bool ElfMap::readSections(std::vector<Section> & sections){

    MapValue shoff, shentsize, shnum, shstrndx;
    bool ok;
    if(m_is64){
        ok = read(0x28, 8, &shoff) && read(0x3A, 2, &shentsize)
          && read(0x3C, 2, &shnum) && read(0x3E, 2, &shstrndx);
    }else{
        ok = read(0x20, 4, &shoff) && read(0x2E, 2, &shentsize)
          && read(0x30, 2, &shnum) && read(0x32, 2, &shstrndx);
    }
    if(!ok || (shoff == 0))
        return false;

    MapValue minEntSize = m_is64 ? 64 : 40;
    if(shentsize < minEntSize)
        return false;

    // More than SHN_LORESERVE sections: the real number/index is stored in
    // the first section header (sh_size/sh_link)
    MapValue val;
    if(shnum == 0){
        if(!read(shoff + (m_is64 ? 32 : 20), m_is64 ? 8 : 4, &shnum))
            return false;
    }
    if(shstrndx == SHN_XINDEX){
        if(!read(shoff + (m_is64 ? 40 : 24), 4, &shstrndx))
            return false;
    }

    // all headers must be inside the file
    MapValue fileSize = (MapValue)m_data.size();
    if((shoff > fileSize) || (shnum > (fileSize - shoff) / shentsize))
        return false;

    std::vector<MapValue> nameOffsets;
    sections.resize((size_t)shnum);
    nameOffsets.resize((size_t)shnum);

    for(MapValue i = 0; i < shnum; ++i){
        MapValue hdr = shoff + i * shentsize;
        Section & sec = sections[(size_t)i];
        if(m_is64){
            ok = read(hdr + 0x00, 4, &nameOffsets[(size_t)i])
              && read(hdr + 0x04, 4, &val);
            sec.type = (unsigned long)val;
            ok = ok && read(hdr + 0x08, 8, &sec.flags)
                    && read(hdr + 0x18, 8, &sec.offset)
                    && read(hdr + 0x20, 8, &sec.size)
                    && read(hdr + 0x28, 4, &val);
            sec.link = (unsigned long)val;
            ok = ok && read(hdr + 0x38, 8, &sec.entsize);
        }else{
            ok = read(hdr + 0x00, 4, &nameOffsets[(size_t)i])
              && read(hdr + 0x04, 4, &val);
            sec.type = (unsigned long)val;
            ok = ok && read(hdr + 0x08, 4, &sec.flags)
                    && read(hdr + 0x10, 4, &sec.offset)
                    && read(hdr + 0x14, 4, &sec.size)
                    && read(hdr + 0x18, 4, &val);
            sec.link = (unsigned long)val;
            ok = ok && read(hdr + 0x24, 4, &sec.entsize);
        }
        if(!ok)
            return false;
    }

    // Section names
    if(shstrndx < shnum){
        Section shstrtab = sections[(size_t)shstrndx];
        for(size_t i = 0; i < sections.size(); ++i)
            sections[i].name = readString(shstrtab, nameOffsets[i]);
    }

    return true;
}

//******************************************************************************
// Convert the symbols of the symbol table 'symtabIdx' to map entries
//******************************************************************************
void ElfMap::readSymbols(const std::vector<Section> & sections, unsigned symtabIdx){

    const Section & symtab = sections[symtabIdx];
    if(symtab.link >= sections.size())
        return;
    const Section & strtab = sections[symtab.link];

    // Extended section indexes (more than SHN_LORESERVE sections)
    const Section * shndxTab = nullptr;
    for(size_t i = 0; i < sections.size(); ++i)
        if((sections[i].type == SHT_SYMTAB_SHNDX) && (sections[i].link == symtabIdx))
            shndxTab = &sections[i];

    MapValue machine = 0;
    read(0x12, 2, &machine);

    MapValue entSize = symtab.entsize;
    if(entSize < (MapValue)(m_is64 ? 24 : 16))
        entSize = m_is64 ? 24 : 16;
    MapValue count = symtab.size / entSize;

    // Symbols already counted in the sizes: (section, (address, size)).
    // Aliases (several names for the same object) are counted only once.
    std::set<std::pair<unsigned long, std::pair<MapValue, MapValue> > > counted;

    MapEntry entry;

    // symbol 0 is always the undefined symbol
    for(MapValue i = 1; i < count; ++i){

        MapValue sym = symtab.offset + i * entSize;
        MapValue nameOff, value, size, info, shndx;
        bool ok;
        if(m_is64){
            ok = read(sym + 0, 4, &nameOff) && read(sym + 4, 1, &info)
              && read(sym + 6, 2, &shndx) && read(sym + 8, 8, &value)
              && read(sym + 16, 8, &size);
        }else{
            ok = read(sym + 0, 4, &nameOff) && read(sym + 4, 4, &value)
              && read(sym + 8, 4, &size) && read(sym + 12, 1, &info)
              && read(sym + 14, 2, &shndx);
        }
        if(!ok)
            break;

        int type = (int)(info & 0x0F);
        int bind = (int)(info >> 4);

        // Only objects, functions and labels (no section, file, ... symbols)
        if((type != STT_NOTYPE) && (type != STT_OBJECT) && (type != STT_FUNC)
        && (type != STT_COMMON) && (type != STT_TLS) && (type != STT_GNU_IFUNC))
            continue;

        // Real section index stored in the extended section index table
        bool extended = (shndx == SHN_XINDEX);
        if(extended){
            if((shndxTab == nullptr) || !read(shndxTab->offset + i * 4, 4, &shndx))
                continue;
        }

        // Undefined symbols (defined in another file)
        if(shndx == SHN_UNDEF)
            continue;

        const Section * sec = nullptr;
        if(extended || (shndx < SHN_LORESERVE)){
            if(shndx >= sections.size())
                continue;
            sec = &sections[(size_t)shndx];
            // not loaded to target memory (e.g. debug info)
            if((sec->flags & SHF_ALLOC) == 0)
                continue;
        }else if((shndx != SHN_ABS) || (type == STT_NOTYPE)){
            // common symbols (object files only, no address yet) and absolute
            // labels (linker script constants like RAM_LENGTH = 0x80000)
            continue;
        }

        entry.clear();
        entry.name = readString(strtab, nameOff);
        if(entry.name.empty() || isMappingSymbol(entry.name))
            continue;
        entry.valid.name = true;

        // Code/data: symbol type, for labels the section flags
        if((type == STT_FUNC) || (type == STT_GNU_IFUNC))
            entry.typ1 = MapEntry::CODE;
        else if(type != STT_NOTYPE)
            entry.typ1 = MapEntry::DATA;
        else if((sec != nullptr) && ((sec->flags & SHF_EXECINSTR) != 0))
            entry.typ1 = MapEntry::CODE;
        else if(sec != nullptr)
            entry.typ1 = MapEntry::DATA;
        else
            entry.typ1 = MapEntry::UNKNOWN;
        entry.valid.typ1 = true;

        // ARM Thumb functions: bit 0 of the address marks the Thumb state
        if((machine == EM_ARM) && (entry.typ1 == MapEntry::CODE))
            value &= ~(MapValue)1;

        entry.addr = value;
        entry.valid.addr = true;
        entry.size = size;
        entry.valid.size = true;

        // Binding (same abbreviations as in IAR map files)
        if(bind == STB_LOCAL)       entry.typ2 = "Lc";
        else if(bind == STB_GLOBAL) entry.typ2 = "Gb";
        else if(bind == STB_WEAK)   entry.typ2 = "Wk";
        else                        entry.typ2 = "Gb";
        entry.valid.typ2 = true;

        // Object: the section containing the symbol
        entry.obj = (sec != nullptr) ? sec->name : std::string("*ABS*");
        entry.valid.obj = !entry.obj.empty();

        bool countSize = counted.insert(std::make_pair((unsigned long)shndx,
                                        std::make_pair(value, size))).second;
        addEntry(entry, countSize);
    }
}

//******************************************************************************
bool ElfMap::parse(std::istream & in){
    DBG_ENTER("ElfMap::parse");

    clear();

    m_data.assign(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());

    bool result = false;

    // ELF identification
    if((m_data.size() >= 16)
    && std::equal(ELF_MAGIC, ELF_MAGIC + sizeof(ELF_MAGIC), m_data.begin())
    && ((m_data[EI_CLASS] == ELFCLASS32) || (m_data[EI_CLASS] == ELFCLASS64))
    && ((m_data[EI_DATA] == ELFDATA2LSB) || (m_data[EI_DATA] == ELFDATA2MSB))){

        m_is64 = (m_data[EI_CLASS] == ELFCLASS64);
        m_bigEndian = (m_data[EI_DATA] == ELFDATA2MSB);

        std::vector<Section> sections;
        if(readSections(sections)){

            // Full symbol table, if stripped: dynamic symbols (shared objects)
            unsigned symtabIdx = 0;
            for(unsigned i = 0; (i < sections.size()) && (symtabIdx == 0); ++i)
                if(sections[i].type == SHT_SYMTAB)
                    symtabIdx = i;
            for(unsigned i = 0; (i < sections.size()) && (symtabIdx == 0); ++i)
                if(sections[i].type == SHT_DYNSYM)
                    symtabIdx = i;

            if(symtabIdx != 0){
                readSymbols(sections, symtabIdx);
                result = true;
            }
        }
    }

    // release the file content
    std::vector<unsigned char>().swap(m_data);

    DBG_EXIT("ElfMap::parse");
    return result;
}

//******************************************************************************
