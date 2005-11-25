//--- For compilers that support precompilation, includes "wx/wx.h" ---//
#include "myprec.h"
#include "Playlist.h"
#include "MusikApp.h"
#include <wx/arrimpl.cpp>
WX_DEFINE_OBJARRAY( MusikSongIdArrayBase );

int MusikSongIdArray::MoveEntrys(int nMoveTo ,const wxArrayInt &arrToMove)
{
    int nNewMoveToPos = nMoveTo;
    wxASSERT(nMoveTo >= 0 && nMoveTo <= (int)GetCount()); 
    int i = arrToMove.GetCount() - 1;
    // first move all entries which are right from nMoveTo position
    for(;i >= 0 ; i--)
    {
        if(nMoveTo > arrToMove[i])
            break;
        Insert(Detach(arrToMove[i] + ( arrToMove.GetCount() - 1 - i)),nMoveTo);
    }
    nNewMoveToPos -= i + 1;
    // now move all entries which are left from nMoveTo
    for(int j = i; j >= 0; j--)
    {
        Insert(Detach(arrToMove[j]),nMoveTo - (i - j)-1);
    }
    return nNewMoveToPos;
}

wxLongLong MusikSongIdArray::GetTotalFileSize() const
{
	return wxLongLong().Assign(wxGetApp().Library.GetSum(wxT("filesize"),*this));
}
int MusikSongIdArray::GetTotalPlayingTimeInSeconds() const
{
	return (int) (wxGetApp().Library.GetSum(wxT("duration"),*this)/1000.0 + 0.5);
}


