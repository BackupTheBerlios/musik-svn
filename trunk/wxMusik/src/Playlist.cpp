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
        size_t nMoveFrom = arrToMove[i] + ( arrToMove.GetCount() - 1 - i);
        if(CurrentIndex() ==  nMoveFrom)
        {
            Insert(Detach(nMoveFrom),nMoveTo);
            CurrentIndex(nMoveTo);
        }
        else
            Insert(Detach(nMoveFrom),nMoveTo);
    }       
    nNewMoveToPos -= i + 1;
    // now move all entries which are left from nMoveTo
    for(int j = i; j >= 0; j--)
    {
        size_t nMoveFrom = arrToMove[j];
        size_t nRealMoveTo = nMoveTo - (i - j)-1;
        if(CurrentIndex() ==  nMoveFrom)
        {
            Insert(Detach(nMoveFrom),nRealMoveTo);
            CurrentIndex(nRealMoveTo);
        }
        else
            Insert(Detach(nMoveFrom),nRealMoveTo);
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


