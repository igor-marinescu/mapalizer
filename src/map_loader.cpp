//******************************************************************************
// Map File Loader: format detection and parser creation
//
// Igor Marinescu
//******************************************************************************

//******************************************************************************
// Includes
//******************************************************************************
#include "map_loader.h"
#include "iar_map.h"
#include "st_map.h"

//******************************************************************************
MapFormat detectMapFormat(const std::string & content, const std::string & extension){

    // Keil L166 signatures
    if((content.find("LINKER/LOCATER L166") != std::string::npos)
    || (content.find("PUBLIC SYMBOLS OF MODULE") != std::string::npos))
        return MAP_FORMAT_ST;

    // IAR signatures
    if((content.find("*** ENTRY LIST") != std::string::npos)
    || (content.find("IAR ELF Linker") != std::string::npos))
        return MAP_FORMAT_IAR;

    // Not recognized: use the file extension
    std::string ext = extension;
    for(std::string::size_type i = 0; i < ext.size(); ++i)
        if((ext[i] >= 'A') && (ext[i] <= 'Z'))
            ext[i] = (char)(ext[i] - 'A' + 'a');

    if(ext == "m66")
        return MAP_FORMAT_ST;

    return MAP_FORMAT_IAR;
}

//******************************************************************************
XMap * createMap(MapFormat format){

    switch(format){
    case MAP_FORMAT_ST:
        return new StMap();
    case MAP_FORMAT_IAR:
    default:
        return new IarMap();
    }
}

//******************************************************************************
