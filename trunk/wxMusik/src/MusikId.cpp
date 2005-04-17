//--- For compilers that support precompilation, includes "wx/wx.h" ---//
#include "myprec.h"
#include <wx/arrimpl.cpp>
#include "MusikId.h"
#include "MusikUtils.h"

#include "MusikApp.h"

MusikSongId::~MusikSongId()
{
   delete m_pSong;
}
std::auto_ptr<CMusikSong> MusikSongId::Song() const
{
    if(m_Id >= 0)
    {
        CMusikSong * p = new CMusikSong();
        if(p)
            wxGetApp().Library.GetSongFromSongid(m_Id,p);
        return std::auto_ptr<CMusikSong>(p);
    }
    else
	{
		if(m_pSong)
			return std::auto_ptr<CMusikSong>(new CMusikSong(*m_pSong));
		else
			return std::auto_ptr<CMusikSong>(new CMusikSong());

	}
}
