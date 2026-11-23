//******************************************************************************
// GCC Map File (GNU ld linker output, generated with -Wl,-Map=<file>)
//
// Igor Marinescu
//******************************************************************************

//******************************************************************************
// Includes
//******************************************************************************
#include "gcc_map.h"
#include <istream>
#include <algorithm>
#include <cstring>
#include "debug_cout.h"
#include "utils.h"

//******************************************************************************
// Defines
//******************************************************************************
// Start of the chapter containing sections and symbols
static const char MEMORY_MAP_CHAPTER[] = "Linker script and memory map";
// Optional chapter following the memory map (-Wl,--cref)
static const char CROSS_REF_CHAPTER[] = "Cross Reference Table";

// Input sections containing code. A section matches if its name is equal to
// the prefix or continues with '.' (e.g. ".text.main", but not ".init_array").
static const char * const CODE_SECTIONS[] = {
    ".text", ".init", ".fini", ".plt", ".iplt", ".plt.got", ".plt.sec",
    ".vectors", ".isr_vector", ".reset", ".time_critical", ".ramfunc",
    ".ramtext", ".boot2", ".glue_7", ".glue_7t", ".vfp11_veneer", ".v4_bx",
    ".gnu.linkonce.t", ".itcm", ".iram", ".flashtext", nullptr
};

// Input sections containing data. A section matches if its name starts with
// the prefix (e.g. ".bss", ".bss.buf" and ".bss_dummy").
static const char * const DATA_SECTIONS[] = {
    ".data", ".bss", ".rodata", ".sdata", ".sbss", ".srodata", ".tdata",
    ".tbss", ".ldata", ".lbss", ".lrodata", "COMMON", ".heap", ".stack",
    ".noinit", ".uninitialized", ".persistent", ".init_array", ".fini_array",
    ".preinit_array", ".ctors", ".dtors", ".got", ".igot", ".eh_frame",
    ".gcc_except_table", ".ARM.extab", ".ARM.exidx", ".tm_clone_table",
    ".jcr", ".gnu.linkonce.d", ".gnu.linkonce.r", ".gnu.linkonce.b",
    ".binary_info", ".mutex_array", ".scratch", ".dtcm", ".ccmram",
    ".ram_vector_table", nullptr
};

// Output sections not loaded to target memory (debug info, attributes, ...)
static const char * const NON_ALLOC_SECTIONS[] = {
    ".debug", ".zdebug", ".stab", ".comment", ".line", ".ARM.attributes",
    ".riscv.attributes", ".gnu.attributes", ".note.GNU-stack",
    ".gnu_debuglink", ".gnu_debugaltlink", ".gnu.build.attributes",
    ".gnu.lto", nullptr
};

//******************************************************************************
// true if 'str' starts with 'prefix'
//******************************************************************************
static bool startsWith(const std::string & str, const char * prefix){
    return str.compare(0, strlen(prefix), prefix) == 0;
}

//******************************************************************************
// true if 'name' is equal to 'prefix' or continues with '.'
//******************************************************************************
static bool isSectionOrSubsection(const std::string & name, const char * prefix){
    size_t len = strlen(prefix);
    return (name.compare(0, len, prefix) == 0)
        && ((name.size() == len) || (name[len] == '.'));
}

//******************************************************************************
// Sort symbols by address (stable: aliases keep the map file order)
//******************************************************************************
template<class T>
static bool lessAddr(const T & a, const T & b){
    return a.addr < b.addr;
}

//******************************************************************************
// true if the rest of a symbol line is a linker script assignment, e.g.
//      __bss_start__ = .
//      . = ALIGN (0x4)
//      PROVIDE (__data_end__ = .)
//******************************************************************************
static bool isAssignment(const std::string & rest){

    if(startsWith(rest, "PROVIDE") || startsWith(rest, "HIDDEN")
    || startsWith(rest, "ASSERT") || startsWith(rest, "["))
        return true;

    // second word is an assignment operator (=, +=, -=, <<=, ...)
    std::string::size_type p = rest.find(' ');
    if(p == std::string::npos)
        return false;
    p = rest.find_first_not_of(' ', p);
    if(p == std::string::npos)
        return false;
    std::string::size_type e = rest.find(' ', p);
    std::string op = rest.substr(p, (e == std::string::npos) ? std::string::npos : e - p);
    return !op.empty() && (op.size() <= 3) && (op[op.size() - 1] == '=');
}

//******************************************************************************
void GccMap::InSection::clear(){
    active = false;
    name.clear();
    outName.clear();
    addr = 0;
    size = 0;
    obj.clear();
    symbols.clear();
}

//******************************************************************************
GccMap::GccMap(){
}

//******************************************************************************
GccMap::~GccMap(){
}

//******************************************************************************
// Only the chapter "Linker script and memory map" is evaluated. It lists the
// output sections, the input sections they contain and the global symbols
// defined in each input section:
//
//  .text           0x10000000     0xa198                      <- output section
//   *(.text*)                                                 <- ignored
//   .text.uart_drv_init                                       <- input section
//                  0x10000638       0xa0 CMakeFiles/x.dir/uart_drv.c.o
//                  0x10000638                uart_drv_init    <- symbol
//   .text          0x20000258       0xa0 libgcc.a(_aeabi_ldivmod.o)
//                  0x20000258                __aeabi_ldivmod  <- symbol
//   *fill*         0x20000689        0x3                      <- ignored
//                  0x20000e10                PROVIDE (__x = .)  <- ignored
//
// Long section names are followed by a line break (the address, size and
// object are printed on the next line).
//
// The map file does not contain the symbol size: it is calculated as the
// distance to the next symbol of the same input section (or to the end of the
// section). Input sections without symbols (static functions/variables) or
// the part of an input section before its first symbol are added as entries
// named after the input section (e.g. ".text.my_static_function", ".bss").
//******************************************************************************
bool GccMap::parse(std::istream & in){
    DBG_ENTER("GccMap::parse");

    clear();

    std::string line;
    std::vector<char> cline;
    char * cptr0;
    char * cptr1;
    bool inMemoryMap = false;
    bool memoryMapFound = false;
    bool skipOutSection = false;    // non-allocated output section
    std::string outName;            // current output section
    std::string pendingIn;          // input section name waiting for address
    InSection sec;

    // Extract all variables/lines from file
    while(std::getline(in, line)){

        trimLineEnd(line);

        // chapter detection
        if(!inMemoryMap){
            if(startsWith(line, MEMORY_MAP_CHAPTER)){
                inMemoryMap = true;
                memoryMapFound = true;
            }
            continue;
        }

        if(startsWith(line, CROSS_REF_CHAPTER))
            break;

        // empty line: end of output section
        if(trim(line).empty()){
            flushSection(sec);
            pendingIn.clear();
            continue;
        }

        // Convert to (modifiable) char array line
        cline.assign(line.begin(), line.end());
        cline.push_back('\0');
        cptr0 = &cline[0];

        // Line starting in column 0: output section ("name [addr size ...]")
        // or linker command (LOAD, START GROUP, OUTPUT(...), ...)
        if(!isBlank(*cptr0)){
            flushSection(sec);
            pendingIn.clear();

            std::string name;
            cptr0 = extractWord(cptr0, &cptr1);
            name = cptr1;
            cptr0 = extractWord(cptr0, &cptr1);
            if((*cptr1 == '\0') || interpretHex(cptr1, nullptr)){
                outName = name;
                skipOutSection = isNonAllocSection(outName);
            }
            continue;
        }

        // First word
        cptr0 = extractWord(cptr0, &cptr1);
        if(*cptr1 == '\0')
            continue;

        MapValue addr, size;

        if(interpretHex(cptr1, &addr)){

            // Text after the address (symbol name or assignment), extracted
            // before the line is split into words
            std::string symName = trim(std::string(cptr0));

            cptr0 = extractWord(cptr0, &cptr1);
            if(interpretHex(cptr1, &size)){
                // "addr size [object]": continuation of a long section name
                if(!pendingIn.empty()){
                    flushSection(sec);
                    sec.active  = !skipOutSection;
                    sec.name    = pendingIn;
                    sec.outName = outName;
                    sec.addr    = addr;
                    sec.size    = size;
                    sec.obj     = objectName(extractRest(cptr0));
                }
                pendingIn.clear();
                continue;
            }
            pendingIn.clear();

            // "addr name": symbol of the current input section. Ignored:
            // assignments and remarks like "0x8 (size before relaxing)".
            if(sec.active && !symName.empty() && (symName[0] != '(')
            && !isAssignment(symName)){
                Symbol sym;
                sym.name = symName;
                sym.addr = addr;
                sec.symbols.push_back(sym);
            }
            continue;
        }

        pendingIn.clear();
        flushSection(sec);

        // Input section patterns "*(.text*)" and "*fill*"
        if(*cptr1 == '*')
            continue;

        std::string name(cptr1);

        // Input section name alone: address and size on the next line
        cptr0 = extractWord(cptr0, &cptr1);
        if(*cptr1 == '\0'){
            pendingIn = name;
            continue;
        }

        // Input section: "name addr size object"
        if(!interpretHex(cptr1, &addr))
            continue;
        cptr0 = extractWord(cptr0, &cptr1);
        if(!interpretHex(cptr1, &size))
            continue;

        sec.active  = !skipOutSection;
        sec.name    = name;
        sec.outName = outName;
        sec.addr    = addr;
        sec.size    = size;
        sec.obj     = objectName(extractRest(cptr0));
    }

    flushSection(sec);

    DBG_EXIT("GccMap::parse");
    return memoryMapFound;
}

//******************************************************************************
void GccMap::flushSection(InSection & sec){

    if(!sec.active){
        sec.clear();
        return;
    }

    std::vector<Symbol> & syms = sec.symbols;
    std::stable_sort(syms.begin(), syms.end(), lessAddr<Symbol>);

    MapValue secEnd = sec.addr + sec.size;

    MapEntry entry;
    entry.typ1 = sectionType(sec.name, sec.outName);
    entry.valid.typ1 = true;
    entry.typ2 = sec.name;
    entry.valid.typ2 = true;
    entry.obj = sec.obj;
    entry.valid.obj = !sec.obj.empty();
    entry.valid.name = true;
    entry.valid.addr = true;
    entry.valid.size = true;

    // Part of the section before the first symbol (or the whole section)
    MapValue firstAddr = secEnd;
    for(std::vector<Symbol>::size_type i = 0; i < syms.size(); ++i){
        if(syms[i].addr >= sec.addr){
            firstAddr = std::min(syms[i].addr, secEnd);
            break;
        }
    }
    if(firstAddr > sec.addr){
        entry.name = sec.name;
        entry.addr = sec.addr;
        entry.size = firstAddr - sec.addr;
        addEntry(entry);
    }

    // Symbols: size up to the next symbol with a different address
    for(std::vector<Symbol>::size_type i = 0; i < syms.size(); ++i){

        std::vector<Symbol>::size_type next = i + 1;
        while((next < syms.size()) && (syms[next].addr == syms[i].addr))
            ++next;
        MapValue nextAddr = (next < syms.size()) ? syms[next].addr : secEnd;

        entry.name = syms[i].name;
        entry.addr = syms[i].addr;

        // Symbols outside of the section (should not happen): no size
        entry.valid.size = (entry.addr >= sec.addr) && (entry.addr <= secEnd);
        entry.size = entry.valid.size ? (std::min(nextAddr, secEnd) - entry.addr) : 0;

        // Aliases (same address as the previous symbol) are counted only once
        bool alias = (i > 0) && (syms[i - 1].addr == syms[i].addr);
        addEntry(entry, !alias);
    }

    sec.clear();
}

//******************************************************************************
MapEntry::Type GccMap::sectionType(const std::string & inName, const std::string & outName){

    // input section first, then the output section it is placed in
    const std::string * names[] = { &inName, &outName };

    for(int n = 0; n < 2; ++n){
        const std::string & name = *names[n];
        for(int i = 0; CODE_SECTIONS[i] != nullptr; ++i)
            if(isSectionOrSubsection(name, CODE_SECTIONS[i]))
                return MapEntry::CODE;
        for(int i = 0; DATA_SECTIONS[i] != nullptr; ++i)
            if(startsWith(name, DATA_SECTIONS[i]))
                return MapEntry::DATA;
    }

    return MapEntry::UNKNOWN;
}

//******************************************************************************
bool GccMap::isNonAllocSection(const std::string & outName){

    for(int i = 0; NON_ALLOC_SECTIONS[i] != nullptr; ++i)
        if(startsWith(outName, NON_ALLOC_SECTIONS[i]))
            return true;

    return false;
}

//******************************************************************************
std::string GccMap::objectName(const std::string & path){

    // Archive member "dir/libc.a(abort.o)": the path ends before '('
    std::string::size_type end = path.find('(');
    if(end == std::string::npos)
        end = path.size();

    std::string::size_type sep = path.find_last_of("/\\", end == 0 ? 0 : end - 1);
    if((sep == std::string::npos) || (sep >= end))
        return path;

    return path.substr(sep + 1);
}

//******************************************************************************
