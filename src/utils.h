//******************************************************************************
// Util functions
//
// Igor Marinescu
//******************************************************************************
#ifndef UTILS_H
#define UTILS_H

//******************************************************************************
// Includes
//******************************************************************************
#include <string>

//******************************************************************************
// Numeric type used for addresses and sizes (64 bit on every platform,
// 'unsigned long' is only 32 bit on Windows).
//******************************************************************************
typedef unsigned long long MapValue;

//******************************************************************************

// true for space, tab, CR and LF
bool isBlank(char c);

// Remove trailing CR/LF characters (CRLF files read in binary mode)
void trimLineEnd(std::string & line);

// Remove leading and trailing blanks (see isBlank)
std::string trim(const std::string & str);

char * extractWord(char * ptr0, char ** pstart);
char * extractRest(char * ptr0);
bool interpretHex(const char * cstr, MapValue * out);
bool interpretHex2(const char * cstr, MapValue * out);

//******************************************************************************
#endif // UTILS_H
