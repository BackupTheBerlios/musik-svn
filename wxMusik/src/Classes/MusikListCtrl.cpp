/*
 *  MusikListCtrl.cpp
 *
 *  
 *  Inherited from a wxListCtrl
 *  
 *  Copyright (c) 2003 Casey Langen (casey@bak.rr.com)
 *	Contributors: Simon Windmill, Dustin Carter, Gunnar Roth, Wade Brainerd
 *
 *  See the file "license.txt" for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL WARRANTIES.
*/

//--- For compilers that support precompilation, includes "wx/wx.h". ---//
#include "wx/wxprec.h"
#include "MusikListCtrl.h"

#ifdef __WXMSW__
#include <commctrl.h>
#endif

CMusikListCtrl::CMusikListCtrl( wxWindow *parent, const wxWindowID id, const wxPoint& pos, const wxSize& size, long style)
	:	wxListCtrl		( parent, id, pos, size, wxLC_REPORT | wxLC_VIRTUAL | wxCLIP_CHILDREN | style)
#ifdef __WXMSW__
	,m_freezeCount(0)
#endif
{
#ifdef __WXMSW__
#ifdef LVS_EX_LABELTIP
	HWND hwnd = (HWND)GetHandle();
	ListView_SetExtendedListViewStyleEx(hwnd,LVS_EX_LABELTIP,LVS_EX_LABELTIP);
#endif
#endif
}


BEGIN_EVENT_TABLE(CMusikListCtrl, wxListCtrl)

#ifdef __WXMSW__
	EVT_ERASE_BACKGROUND	( CMusikListCtrl::OnEraseBackground )
	EVT_PAINT ( CMusikListCtrl::OnPaint )
#endif
END_EVENT_TABLE()

void CMusikListCtrl::DoSetSize(int x, int y,int width, int height,int sizeFlags)
{
	Freeze();
	wxListCtrl::DoSetSize(x,y,width,height,sizeFlags);
	OnRescaleColumns();
	Thaw();

}



bool CMusikListCtrl::OnRescaleColumns()	
{
	return true;
}
#ifdef __WXMSW__
#include "wx/dcbuffer.h"
void CMusikListCtrl::Freeze()
{
	if(!m_freezeCount++)
		wxListCtrl::Freeze();
}
void CMusikListCtrl::Thaw()
{
    wxCHECK_RET( m_freezeCount > 0, _T("thawing unfrozen list control?") );

    if ( !--m_freezeCount )
    {
        wxListCtrl::Thaw();
    }

}

void CMusikListCtrl::OnEraseBackground(wxEraseEvent& WXUNUSED(event))
{
	// empty => no background erasing to avoid flicker
}
void CMusikListCtrl::OnPaint(wxPaintEvent& event)
{
	if(m_freezeCount > 0)
	{
		wxControl::OnPaint(event);
		return;
	}
	wxBufferedPaintDC dc(this);

	wxEraseEvent erase_event(m_windowId, &dc);
	wxControl::OnEraseBackground(erase_event);

	MSWDefWindowProc(WM_PAINT, (WPARAM) (HDC) dc.GetHDC(), 0);

	/* copied from original wxwindows code */
    // Reset the device origin since it may have been set
	dc.SetDeviceOrigin(0, 0);
    bool drawHRules = ((GetWindowStyle() & wxLC_HRULES) != 0);
    bool drawVRules = ((GetWindowStyle() & wxLC_VRULES) != 0);

    if (!drawHRules && !drawVRules)
        return;
    if ((GetWindowStyle() & wxLC_REPORT) == 0)
        return;
    

    wxPen pen(wxSystemSettings::GetColour(wxSYS_COLOUR_3DLIGHT), 1, wxSOLID);
    dc.SetPen(pen);
    dc.SetBrush(* wxTRANSPARENT_BRUSH);

    wxSize clientSize = GetClientSize();
    wxRect itemRect;
    int cy=0;

    int itemCount = GetItemCount();
    int i;
    if (drawHRules)
    {
        long top = GetTopItem();
        for (i = top; i < top + GetCountPerPage() + 1; i++)
        {
            if (GetItemRect(i, itemRect))
            {
                cy = itemRect.GetTop();
                if (i != 0) // Don't draw the first one
                {
                    dc.DrawLine(0, cy, clientSize.x, cy);
                }
                // Draw last line
                if (i == itemCount - 1)
                {
                    cy = itemRect.GetBottom();
                    dc.DrawLine(0, cy, clientSize.x, cy);
                }
            }
        }
    }
    i = itemCount - 1;
    if (drawVRules && (i > -1))
    {
        wxRect firstItemRect;
        GetItemRect(0, firstItemRect);

        if (GetItemRect(i, itemRect))
        {
            int col;
            int x = itemRect.GetX();
            for (col = 0; col < GetColumnCount(); col++)
            {
                int colWidth = GetColumnWidth(col);
                x += colWidth ;
                dc.DrawLine(x-1, firstItemRect.GetY() - 2, x-1, itemRect.GetBottom());
            }
        }
    }
}
#endif
