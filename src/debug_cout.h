//******************************************************************************
// Description:     Class used for debugging.
//
// Notes:
//                  How to use it:
//                  --------------
//                  To debug some info use the DBG_xxx statements, ex:
//                  DBG_INFO("Hello world");
//                  DBG_INFO("x+y="<<std::dec<<(x+y));
//
//                  In order to activate the debugging, use DEBUGON define.
//                  #define DEBUGON
//                  or, for QT define the following in the .pro file:
//                      DEFINES += DEBUGON
//
//                  The log goes to output stream (std::cout).
//                  To redirect the output stream to a file (debug.txt)
//                  use DEBUGCOUT define and create one DebugCout object
//                  (e.g. at the beginning of main):
//                  #define DEBUGCOUT
//                  or, for QT define the following in the .pro file:
//                      DEFINES += DEBUGCOUT
//
// Marinescu Igor
// Date: 16.09.2013
//******************************************************************************
#ifndef DEBUG_COUT_H
#define DEBUG_COUT_H

//******************************************************************************
// Includes
//******************************************************************************
#include <iostream>     // std::streambuf, std::cout
#include <fstream>      // std::ofstream

//******************************************************************************
// Defines
//******************************************************************************
#ifdef DEBUGON
    #define DBG_ENTER(y)        do{ std::cout<<"[enter] "<<y<<std::endl; }while(0)
    #define DBG_EXIT(y)         do{ std::cout<<"[exit]  "<<y<<std::endl; }while(0)
    #define DBG_ERROR(y)        do{ std::cout<<"[error] "<<y<<std::endl; }while(0)
    #define DBG_INFO(y)         do{ std::cout<<"[info]  "<<y<<std::endl; }while(0)
#else
    #define DBG_ENTER(y)        do{ }while(0)
    #define DBG_EXIT(y)         do{ }while(0)
    #define DBG_ERROR(y)        do{ }while(0)
    #define DBG_INFO(y)         do{ }while(0)
#endif

//******************************************************************************
// Redirects std::cout to the file debug.txt for the lifetime of the object
//******************************************************************************
class DebugCout{
private:
    std::ofstream dfile;        // debug file
    std::streambuf *cout_buf;   // cout output buffer (nullptr if not redirected)

    // not copyable
    DebugCout(const DebugCout &);
    DebugCout & operator=(const DebugCout &);

public:
    DebugCout();
    virtual ~DebugCout();
};

//******************************************************************************
#endif // DEBUG_COUT_H
