//******************************************************************************
// Util functions
//
// Igor Marinescu
//******************************************************************************
#ifndef UTILS_H
#define UTILS_H

//******************************************************************************

char * extractWord(char * ptr0, char ** pstart);
char * extractRest(char * ptr0);
bool interpretHex(const char * cstr, unsigned long * out);
bool interpretHex2(const char * cstr, unsigned long * out);

//******************************************************************************
#endif // UTILS_H
