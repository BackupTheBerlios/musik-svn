/*
*  CTagLibInfo.h
*
*  This calls encapuslates the Taglib guntions, for retrieving metadata for all  filetypes taglib supports
*
*  Copyright (c) 2004  Gunnar Roth
*
*  See the file "license.txt" for information on usage and redistribution
*  of this file, and for a DISCLAIMER OF ALL WARRANTIES.
*/
#ifndef APEINFO_H
#define APEINFO_H
#include "CInfo.h"
//--- globals ---//
#include "../../MusikGlobals.h"
class CAPETag;
class CTagLibInfo : public CInfoRead,public CInfoWrite
{
public:
	CTagLibInfo(void);
	bool ReadMetaData(CSongMetaData & MetaData) const;
	bool WriteMetaData(const CSongMetaData & MetaData,bool bClearAll = false);
};
#endif
