//******************************************************************************
// Map File Loader: format detection and parser creation
//
// Igor Marinescu
//******************************************************************************
#ifndef MAP_LOADER_H
#define MAP_LOADER_H

//******************************************************************************
// Includes
//******************************************************************************
#include <string>
#include "map_entry.h"

//******************************************************************************
enum MapFormat {
    MAP_FORMAT_IAR,         // IAR ILINK (*.map)
    MAP_FORMAT_ST,          // Keil L166 for ST10/C166 (*.m66)
    MAP_FORMAT_GCC          // GNU ld (*.map)
};

// Detect the map format from the file content. If the content is not
// recognized the file extension (without dot, e.g. "m66") decides.
MapFormat detectMapFormat(const std::string & content, const std::string & extension);

// Create a parser for the given format (to be deleted by the caller)
XMap * createMap(MapFormat format);

//******************************************************************************
#endif // MAP_LOADER_H
