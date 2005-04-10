//--- For compilers that support precompilation, includes "wx/wx.h" ---//
#include "wx/wxprec.h"
#include "Playlist.h"
#include "MusikApp.h"
#include <wx/arrimpl.cpp>
WX_DEFINE_OBJARRAY( CMusikSongIdArray );

wxLongLong CMusikSongArray::GetTotalFileSize() const
{
	return wxLongLong().Assign(wxGetApp().Library.GetSum(wxT("filesize"),*this));
}
int CMusikSongArray::GetTotalPlayingTimeInSeconds() const
{
	return (int) (wxGetApp().Library.GetSum(wxT("duration"),*this)/1000.0 + 0.5);
}


