#ifndef CINFO_H
#define CINFO_H

//--- globals ---//
#include "../../MusikGlobals.h"


class CInfoRead
{
public:
	virtual bool ReadMetaData(CSongMetaData & MetaData) const =0;
protected:

private:
};
class CInfoWrite
{
public:
	virtual bool WriteMetaData(const CSongMetaData & MetaData,bool bClearAll = false)=0;
protected:

private:
};
#endif
