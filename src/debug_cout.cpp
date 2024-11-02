//******************************************************************************
// Description:     Class used for debuging.
//
// Notes:
//                  How to use it:
//                  --------------
//                  To debug some info use LOG(x) statement, ex:
//                  DBG_INFO("Hello world");
//                  DBG_INFO("x+y="<<std::dec<<(x+y));
//
//                  In order to activate de debugging, use DEBUGON define.
//                  #define DEBUGON
//                  or, for QT define the following in the .pro file:
//                      DEFINES += DEBUGON
//
//                  The log goes to output stream (std::cout).
//                  To redirect the output stream to a file (debug.txt)
//                  use DEBUGCOUT define:
//                  #define DEBUGCOUT
//                  or, for QT define the following in the .pro file:
//                      DEFINES += DEBUGCOUT
//
// Marinescu Igor
// Data: 16.09.2013
//******************************************************************************
//******************************************************************************
// Includes
//******************************************************************************
#include "debug_cout.h"

//******************************************************************************
// Defines
//******************************************************************************
#ifdef DEBUGCOUT
DebugCout debugCout;
#endif

//******************************************************************************
// Class constructor
//******************************************************************************
DebugCout::DebugCout(){

    // Open debug file for writing
    // redirect std::cout to debugf file
    dfile.open("debug.txt");
    cout_buf = std::cout.rdbuf();   // back up cout's streambuf
    dout_buf = dfile.rdbuf();       // get debug file's streambuf
    std::cout.rdbuf(dout_buf);      // assign debug streambuf to cout
}

//******************************************************************************
// Class destructor
//******************************************************************************
DebugCout::~DebugCout(){

    // Close debug file
    // Restore std::cout
    std::cout.rdbuf(cout_buf);      // restore cout's original streambuf
    dfile.close();                  // close debuger file
}
