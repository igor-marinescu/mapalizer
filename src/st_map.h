//******************************************************************************
// ST Map File
//
// Igor Marinescu
//******************************************************************************
#ifndef ST_MAP_H
#define ST_MAP_H

//******************************************************************************
// Includes
//******************************************************************************
#include <iostream>
#include <vector>
#include <set>
#include <map>
#include "map_entry.h"

using namespace std;

//******************************************************************************
//******************************************************************************
class StMap : public XMap{

public:
    StMap(const char * filename);
    virtual ~StMap();
};

//******************************************************************************
#endif // ST_MAP_H
