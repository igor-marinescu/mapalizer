//******************************************************************************
// Util functions
//
// Igor Marinescu
//******************************************************************************

//******************************************************************************
// Includes
//******************************************************************************
#include "utils.h"

//******************************************************************************
// Extract (first) next word from string by populating 'pstring' pointer with
// first non-empty character and removing the trailing empty character (by
// inserting null). The pointer to remaining of the string is returned.
//
//            +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
// Input:     |   |   | H | e | l | l | o |   | W | o | r | l | d |   |\0 |
//            +-^-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
//              |
//             ptr0
//                                   null inserted
//                                          |
//            +---+---+---+---+---+---+---+-v-+---+---+---+---+---+---+---+
// Output:    |   |   | H | e | l | l | o |\0 | W | o | r | l | d |   |\0 |
//            +-^-+---+-^-+---+---+---+---+---+-^-+---+---+---+---+---+---+
//              |       |                       |
//             ptr0   pstart                  return
//
//
//            +---+---+---+---+---+---+---+
// Input:     | W | o | r | l | d |   |\0 |
//            +-^-+---+---+---+---+---+---+
//              |
//             ptr0
//                             null inserted
//                                  |
//            +---+---+---+---+---+-v-+---+
// Output:    | W | o | r | l | d |\0 |\0 |
//            +-^-+---+---+---+---+---+-^-+
//              |                       |
//             ptr0                   return
//            pstart
//
//******************************************************************************
char * extractWord(char * ptr0, char ** pstart){

    *pstart = ptr0;
    if(ptr0 == nullptr)
        return nullptr;

    if(*ptr0 == '\0')
        return ptr0;

    // remove leading spaces
    while((*ptr0 != '\0') && ((*ptr0 == ' ') || (*ptr0 == '\t'))) ptr0++;

    *pstart = ptr0;
    if(*ptr0 == '\0')
        return ptr0;

    // skip word (until space or null found)
    while((*ptr0 != '\0') && (*ptr0 != ' ') && (*ptr0 != '\t')) ptr0++;

    // insert null at the end of found word
    if(*ptr0 != '\0'){
        *ptr0 = '\0';
        ++ptr0;
    }

    return ptr0;
}

//******************************************************************************
// Extract the rest of the string, by returning the pointer to first non-empty
// character and removing the trailing empty characters (by inserting null).
//
//            +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
// Input:     |   |   | H | e | l | l | o |   | W | o | r | l | d |   |\0 |
//            +-^-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
//              |
//             ptr0
//                                                            null inserted
//                                                                  |
//            +---+---+---+---+---+---+---+---+---+---+---+---+---+-v-+---+
// Output:    |   |   | H | e | l | l | o |   | W | o | r | l | d |\0 |\0 |
//            +-^-+---+-^-+---+---+---+---+---+---+---+---+---+---+---+---+
//              |       |
//             ptr0   return
//
//******************************************************************************
char * extractRest(char * ptr0){

    if(ptr0 == nullptr)
        return nullptr;

    if(*ptr0 == '\0')
        return ptr0;

    // remove leading spaces
    while((*ptr0 != '\0') && ((*ptr0 == ' ') || (*ptr0 == '\t'))) ptr0++;

    if(*ptr0 == '\0')
        return ptr0;

    char * pstart = ptr0;

    // find end (null)
    while(*ptr0 != '\0') ptr0++;

    // go back and skip empty spaces
    do{
        *ptr0 = '\0';
        ptr0--;
    }while((ptr0 > pstart) && (*ptr0 == ' ') && (*ptr0 == '\t'));

    return pstart;
}

//******************************************************************************
// Return true - if string start (ignoring white spaces at begining)
//              with a valid number in hex format 0xFFFF.
//              Decoded number is written in out.
// Return false - if string doesn't start with a valid 0xFFFF format hex number
//              (out is not modified in this case).
//******************************************************************************
bool interpretHex(const char * cstr, unsigned long * out){

    bool special = false;

    if((cstr == nullptr) || (*cstr == '\0'))
        return false;

    // remove leading spaces
    while((*cstr != '\0') && ((*cstr == ' ') || (*cstr == '\t'))) cstr++;

    // check '0x'
    if((*cstr != '0') || (cstr[1] != 'x'))
        return false;

    special = true;
    cstr += 2;

    // extract value
    unsigned long val = 0;
    while((*cstr != '\0') && (*cstr != ' ') && (*cstr != '\t')){
        if((*cstr >= '0') && (*cstr <= '9')){
            val <<= 4;
            val |= (unsigned long)(*cstr - '0');
            special = false;
        }
        else if((*cstr >= 'A') && (*cstr <= 'F')){
            val <<= 4;
            val |= (unsigned long)(10 + (*cstr - 'A'));
            special = false;
        }
        else if((*cstr >= 'a') && (*cstr <= 'f')){
            val <<= 4;
            val |= (unsigned long)(10 + (*cstr - 'a'));
            special = false;
        }
        else if((*cstr == '\'') && !special)
            ;
        else
            return false;
        ++cstr;
    }

    if(special)
        return false;

    if(out != nullptr)
        *out = val;

    return true;
}

//******************************************************************************
// Return true - if string start (ignoring white spaces at begining)
//              with a valid number in hex format FFFFH.
//              Decoded number is written in out.
// Return false - if string doesn't start with a valid FFFFH format hex number
//              (out is not modified in this case).
//******************************************************************************
bool interpretHex2(const char * cstr, unsigned long * out){

    if((cstr == nullptr) || (*cstr == '\0'))
        return false;

    // remove leading spaces
    while((*cstr != '\0') && ((*cstr == ' ') || (*cstr == '\t'))) cstr++;

    // extract value
    unsigned long val = 0;
    while(*cstr != 'H'){
        val <<= 4;
        if((*cstr >= '0') && (*cstr <= '9'))
            val |= (unsigned long)(*cstr - '0');
        else if((*cstr >= 'A') && (*cstr <= 'F'))
            val |= (unsigned long)(10 + (*cstr - 'A'));
        else if((*cstr >= 'a') && (*cstr <= 'f'))
            val |= (unsigned long)(10 + (*cstr - 'a'));
        else
            return false;
        ++cstr;
    }

    if(out != nullptr)
        *out = val;

    return true;
}

//******************************************************************************

