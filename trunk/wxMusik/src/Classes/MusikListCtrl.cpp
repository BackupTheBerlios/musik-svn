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
#include "myprec.h"

#include "MusikUtils.h"
#include "MusikListCtrl.h"

#ifdef __WXMSW__
#include <commctrl.h>
#endif


CMusikListCtrl::CMusikListCtrl( wxWindow *parent, const wxWindowID id, const wxPoint& pos, const wxSize& size, long style)
	:	wxListCtrl		( parent, id, pos, size, wxLC_REPORT | wxLC_VIRTUAL | wxCLIP_CHILDREN | style)
#ifdef __WXMSW__
	,m_freezeCount(0)
    ,m_bHideHorzScrollbar(false)
    ,m_bHideVertScrollbar(false)
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
	EVT_MIDDLE_DOWN(  CMusikListCtrl::OnMiddleDown)
	EVT_RIGHT_DOWN(  CMusikListCtrl::OnRightDown)
    EVT_MOUSEWHEEL(CMusikListCtrl::OnMouseWheel)
#ifdef WXMUSIK_BUGWORKAROUND_LISTCTRL_CONTEXTMENU
	EVT_LIST_ITEM_RIGHT_CLICK(-1, CMusikListCtrl::ShowMenu)
#else
	EVT_CONTEXT_MENU( CMusikListCtrl::ShowMenu)

#endif	

#ifdef __WXMSW__
	EVT_ERASE_BACKGROUND	( CMusikListCtrl::OnEraseBackground )
	EVT_PAINT ( CMusikListCtrl::OnPaint )
#endif
	EVT_SIZE  (	CMusikListCtrl::OnSize )	
END_EVENT_TABLE()

void CMusikListCtrl::OnSize( wxSizeEvent& event )
{
	OnRescaleColumns();
    event.Skip(); 
}
// enable wheel scroll, no matter if the bug in wxWidgets/src/generic/listctrl is fixed or not
void CMusikListCtrl::OnMouseWheel(wxMouseEvent &event)
{
	event.Skip();
}
void CMusikListCtrl::OnRightDown(wxMouseEvent &event)
{
	SetFocus();
	int HitFlags = 0;
	long item = HitTest(event.GetPosition(),HitFlags);
	if(item != -1)
	{
		SelectClickedItem(event);
		wxListEvent myev(wxEVT_COMMAND_LIST_ITEM_RIGHT_CLICK, GetId());
		myev.SetEventObject(this);
		myev.m_itemIndex = item;
		myev.m_pointDrag = event.GetPosition();
		GetEventHandler()->ProcessEvent(myev);
	}
	return;

}

void CMusikListCtrl::OnMiddleDown(wxMouseEvent &event)
{
	SetFocus();
	int HitFlags = 0;
	long item = HitTest(event.GetPosition(),HitFlags);
	if(item != -1)
	{
		SelectClickedItem(event);
		wxListEvent myev(wxEVT_COMMAND_LIST_ITEM_MIDDLE_CLICK, GetId());
		myev.SetEventObject(this);
		myev.m_itemIndex = item;
		myev.m_pointDrag = event.GetPosition();
		GetEventHandler()->ProcessEvent(myev);
	}
	return;
}
void CMusikListCtrl::SelectClickedItem(wxMouseEvent &event)
{
	int HitFlags = 0;
	long item = HitTest(event.GetPosition(),HitFlags);
	if(item != -1)
	{
        if((wxLIST_STATE_SELECTED & GetItemState(item,wxLIST_STATE_SELECTED)) != wxLIST_STATE_SELECTED) // bug in MSW GetItemState (mask param is ignored)
        {    
            if(!event.CmdDown())
            {
                int i = -1;
                while ( -1 != (i = GetNextItem(i, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED)))
                {
                    if(i != item)
                        SetItemState( i, 0, wxLIST_STATE_SELECTED );
                }

            }
            SetItemState(item,wxLIST_STATE_SELECTED,wxLIST_STATE_SELECTED);
            wxTheApp->Yield(true);   // call yield to let the SetItemState changes go through the system.
        }
    }
}
bool CMusikListCtrl::OnRescaleColumns()	
{
	return true;
}

#ifdef WXMUSIK_BUGWORKAROUND_LISTCTRL_CONTEXTMENU
void CMusikListCtrl::ShowMenu( wxListEvent& WXUNUSED(event) )
#else
void CMusikListCtrl::ShowMenu( wxContextMenuEvent &WXUNUSED(event) )
#endif
{ 
	wxMenu *context_menu = CreateContextMenu();
    if(context_menu)
    {
	   PopupMenu( context_menu);
    	delete context_menu;
    }
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

	MSWDefWindowProc(WM_ERASEBKGND, (WPARAM) (HDC) dc.GetHDC(), 0);
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

#ifdef __WXMSW__
BOOL ModifyStyle(HWND hWnd, 
            DWORD dwRemove, DWORD dwAdd, UINT nFlags)
{
    int nStyleOffset = GWL_STYLE;
    DWORD dwStyle = ::GetWindowLong(hWnd, nStyleOffset);
    DWORD dwNewStyle = (dwStyle & ~dwRemove) | dwAdd;
    if (dwStyle == dwNewStyle)
        return FALSE;

    ::SetWindowLong(hWnd, nStyleOffset, dwNewStyle);
    return TRUE;

}

long CMusikListCtrl::MSWWindowProc(WXUINT message, WXWPARAM wParam, WXLPARAM lParam)
{
    if(message == WM_NCCALCSIZE)
    {
            if(m_bHideHorzScrollbar)
                ModifyStyle(((HWND)GetHWND()),WS_HSCROLL ,0,0);
            if(m_bHideVertScrollbar)
                ModifyStyle(((HWND)GetHWND()),WS_VSCROLL,0,0);
    }
    return wxListCtrl::MSWWindowProc(message,wParam,lParam);
}	
#endif
