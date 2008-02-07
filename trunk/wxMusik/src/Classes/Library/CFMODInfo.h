#ifndef CFMODINFO_H
#define CFMODINFO_H
//--- For compilers that support precompilation, includes "wx/wx.h". ---//
#include "myprec.h"
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif 
#include "CInfo.h"
//--- globals ---//
#include "../../MusikGlobals.h"

#ifdef USE_FMOD3
class CFMODInfo : public CInfoRead
{
public:
	CFMODInfo();
	bool ReadMetaData(CSongMetaData & MetaData) const;
protected:

private:
};
#elif defined USE_FMODEX
namespace FMOD
{
    class System;
}

class CFMODInfo : public CInfoRead
{
public:
    CFMODInfo();
    ~CFMODInfo();
    bool ReadMetaData(CSongMetaData & MetaData) const;
protected:

private:
    FMOD::System *m_pSystem;
};

#endif
#endif
