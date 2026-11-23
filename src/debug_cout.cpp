//******************************************************************************
// Description:     Class used for debugging (see debug_cout.h).
//
// Marinescu Igor
// Date: 16.09.2013
//******************************************************************************

//******************************************************************************
// Includes
//******************************************************************************
#include "debug_cout.h"

//******************************************************************************
// Class constructor
//******************************************************************************
DebugCout::DebugCout() :
    cout_buf(nullptr){

    // Open debug file for writing and redirect std::cout to it.
    // If the file cannot be created std::cout is left unchanged.
    dfile.open("debug.txt");
    if(!dfile.is_open())
        return;

    cout_buf = std::cout.rdbuf(dfile.rdbuf());  // back up cout's streambuf
}

//******************************************************************************
// Class destructor
//******************************************************************************
DebugCout::~DebugCout(){

    // Restore std::cout and close debug file
    if(cout_buf != nullptr){
        std::cout.flush();
        std::cout.rdbuf(cout_buf);      // restore cout's original streambuf
        std::cout.clear();              // reset possible error flags
    }
    dfile.close();
}
