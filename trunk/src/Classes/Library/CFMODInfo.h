#ifndef CFMODINFO_H
#define CFMODINFO_H
//--- For compilers that support precompilation, includes "wx/wx.h". ---//
#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif 
#include "CFMODInfo.h"
#include "CInfo.h"
//--- globals ---//
#include "../../MusikGlobals.h"


class CFMODInfo : public CInfoRead
{
public:
	CFMODInfo();
	bool ReadMetaData(CSongMetaData & MetaData) const;
protected:

private:
};
#endif
