/*
*  CAPEInfo.h
*
*  Helper functions to retrieve  Musepack (ID3V1 APE1/2) comments of a given file.
*   Writing is done by the APEInfo base class
*  Copyright (c) 2004  Gunnar Roth
*
*  See the file "license.txt" for information on usage and redistribution
*  of this file, and for a DISCLAIMER OF ALL WARRANTIES.
*/
#ifndef MPCINFO_H
#define MPCINFO_H
#include "APEInfo.h"
//--- globals ---//
#include "../../MusikGlobals.h"

class CMPCInfo : public CAPEInfo
{
  bool ReadMetaData(CSongMetaData & MetaData) const;
};
#endif