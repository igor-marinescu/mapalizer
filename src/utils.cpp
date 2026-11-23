//******************************************************************************
// Util functions
//
// Igor Marinescu
//******************************************************************************

//******************************************************************************
// Includes
//******************************************************************************
#include "utils.h"
#include <cstring>

//******************************************************************************
// Defines
//******************************************************************************
// Maximum number of hex digits that fit into MapValue
static const int MAX_HEX_DIGITS = (int)(sizeof(MapValue) * 2);

//******************************************************************************
bool isBlank(char c){
    return (c == ' ') || (c == '\t') || (c == '\r') || (c == '\n');
}

//******************************************************************************
void trimLineEnd(std::string & line){
    while(!line.empty()
       && ((line[line.size() - 1] == '\r') || (line[line.size() - 1] == '\n')))
        line.erase(line.size() - 1);
}

//******************************************************************************
std::string trim(const std::string & str){
    std::string::size_type first = 0;
    std::string::size_type last = str.size();
    while((first < last) && isBlank(str[first])) ++first;
    while((last > first) && isBlank(str[last - 1])) --last;
    return str.substr(first, last - first);
}

//******************************************************************************
// Extract (first) next word from string by populating 'pstart' pointer with
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

    // remove leading blanks
    while((*ptr0 != '\0') && isBlank(*ptr0)) ptr0++;

    *pstart = ptr0;
    if(*ptr0 == '\0')
        return ptr0;

    // skip word (until blank or null found)
    while((*ptr0 != '\0') && !isBlank(*ptr0)) ptr0++;

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

    // remove leading blanks
    while((*ptr0 != '\0') && isBlank(*ptr0)) ptr0++;

    char * pstart = ptr0;

    // find end (null) and go back over trailing blanks
    char * pend = pstart + strlen(pstart);
    while((pend > pstart) && isBlank(pend[-1])) --pend;
    *pend = '\0';

    return pstart;
}

//******************************************************************************
// Decode one hex digit. Returns -1 if 'c' is not a hex digit.
//******************************************************************************
static int hexDigit(char c){
    if((c >= '0') && (c <= '9')) return c - '0';
    if((c >= 'A') && (c <= 'F')) return 10 + (c - 'A');
    if((c >= 'a') && (c <= 'f')) return 10 + (c - 'a');
    return -1;
}

//******************************************************************************
// Return true - if string starts (ignoring blanks at beginning) with a valid
//              number in hex format 0xFFFF (or 0XFFFF), terminated by a blank
//              or the end of the string. A digit separator ' is allowed
//              between digits (IAR 64 bit format: 0x0000'0000'0800'0000).
//              Decoded number is written in out.
// Return false - if string doesn't start with a valid 0xFFFF format hex number
//              or the number does not fit into MapValue
//              (out is not modified in this case).
//******************************************************************************
bool interpretHex(const char * cstr, MapValue * out){

    if(cstr == nullptr)
        return false;

    // remove leading blanks
    while((*cstr != '\0') && isBlank(*cstr)) cstr++;

    // check '0x'
    if((cstr[0] != '0') || ((cstr[1] != 'x') && (cstr[1] != 'X')))
        return false;
    cstr += 2;

    // extract value
    MapValue val = 0;
    int digits = 0;             // number of significant digits
    bool lastWasDigit = false;  // separator is only allowed after a digit
    while((*cstr != '\0') && !isBlank(*cstr)){
        int d = hexDigit(*cstr);
        if(d >= 0){
            if((val != 0) || (d != 0))
                ++digits;
            if(digits > MAX_HEX_DIGITS)
                return false;   // overflow
            val = (val << 4) | (MapValue)d;
            lastWasDigit = true;
        }
        else if((*cstr == '\'') && lastWasDigit)
            lastWasDigit = false;
        else
            return false;
        ++cstr;
    }

    // at least one digit, no trailing separator
    if(!lastWasDigit)
        return false;

    if(out != nullptr)
        *out = val;

    return true;
}

//******************************************************************************
// Return true - if string starts (ignoring blanks at beginning) with a valid
//              number in hex format FFFFH (or FFFFh), terminated by a blank
//              or the end of the string.
//              Decoded number is written in out.
// Return false - if string doesn't start with a valid FFFFH format hex number
//              or the number does not fit into MapValue
//              (out is not modified in this case).
//******************************************************************************
bool interpretHex2(const char * cstr, MapValue * out){

    if(cstr == nullptr)
        return false;

    // remove leading blanks
    while((*cstr != '\0') && isBlank(*cstr)) cstr++;

    // extract value
    MapValue val = 0;
    int digits = 0;         // all digits
    int significant = 0;    // digits without leading zeros
    while((*cstr != 'H') && (*cstr != 'h')){
        int d = hexDigit(*cstr);
        if(d < 0)
            return false;   // also covers end of string without 'H'
        ++digits;
        if((val != 0) || (d != 0))
            ++significant;
        if(significant > MAX_HEX_DIGITS)
            return false;   // overflow
        val = (val << 4) | (MapValue)d;
        ++cstr;
    }
    ++cstr; // skip 'H'

    // at least one digit, 'H' must terminate the word
    if((digits == 0) || ((*cstr != '\0') && !isBlank(*cstr)))
        return false;

    if(out != nullptr)
        *out = val;

    return true;
}

//******************************************************************************
