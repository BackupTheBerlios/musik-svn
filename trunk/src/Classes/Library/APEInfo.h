/*
*  CAPEInfo.h
*
*  Helper functions to retrieve and write APE comments of a given file.
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
class CAPEInfo : public CInfoRead,public CInfoWrite
{
public:
	CAPEInfo(void);
	bool ReadMetaData(CSongMetaData & MetaData) const;
	bool WriteMetaData(const CSongMetaData & MetaData,bool bClearAll = false);
protected:
	bool ReadTagData(CSongMetaData & MetaData) const;
	virtual bool ReadFileData(CSongMetaData & MetaData) const;
	static bool GetFieldAsUtf8(wchar_t *pFieldName,CAPETag &tag, CSongMetaData::StringData &s);
};
#endif