#ifndef	 CMETADATAHANDLER_H
#define CMETADATAHANDLER_H

#include "../../MusikDefines.h"
#include "CInfo.h"
struct tSongClass
{
	const wxChar * szExtension;
	const wxChar * szDescription;
	EMUSIK_FORMAT_TYPE eFormat;
	CInfoRead * pInfoRead;
	CInfoWrite * pInfoWrite;
};

class CMetaDataHandler
{
public:
	enum RetCode{fail,success ,notsupported };
	CMetaDataHandler(){}
	~CMetaDataHandler(){}
	
	static const tSongClass * GetSongClass(const wxString &ext);
	static const tSongClass * GetSongClass(EMUSIK_FORMAT_TYPE eFormat);
	static RetCode GetMetaData		( CSongMetaData & MetaData  );
	static RetCode WriteMetaData	(const CSongMetaData &MetaData,bool bClearAll);
protected:
};
#endif
