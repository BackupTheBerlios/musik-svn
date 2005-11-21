#ifndef CMP3INFO_H
#define CMP3INFO_H
#include "CInfo.h"
//--- globals ---//
#include "../../MusikUtils.h"


class CMP3Info : public CInfoRead,public CInfoWrite
{
public:
	CMP3Info();
	bool ReadMetaData(CSongMetaData & MetaData) const;
	bool WriteMetaData(const CSongMetaData & MetaData,bool bClearAll = false);

protected:

private:
};
#endif
