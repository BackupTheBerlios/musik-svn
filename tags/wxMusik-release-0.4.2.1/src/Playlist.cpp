//--- For compilers that support precompilation, includes "wx/wx.h" ---//
#include "myprec.h"
#include "Playlist.h"
#include "MusikApp.h"
#include <wx/arrimpl.cpp>
WX_DEFINE_OBJARRAY( MusikSongIdArrayBase );

wxLongLong MusikSongIdArray::GetTotalFileSize() const
{
	return wxLongLong().Assign(wxGetApp().Library.GetSum(wxT("filesize"),*this));
}
int MusikSongIdArray::GetTotalPlayingTimeInSeconds() const
{
	return (int) (wxGetApp().Library.GetSum(wxT("duration"),*this)/1000.0 + 0.5);
}


