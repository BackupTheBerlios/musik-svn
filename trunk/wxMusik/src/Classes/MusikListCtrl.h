/*
 *  MusikListCtrl.h
 *
 *  The "MusikListCtrl" control
 *	Base class for our ListCtrls
 *
 *  Copyright (c) 2003 Casey Langen (casey@bak.rr.com)
 *	Contributors: Simon Windmill, Dustin Carter, Gunnar Roth, Wade Brainerd
 *
 *  See the file "license.txt" for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL WARRANTIES.
*/
#ifndef MUSIKLISTCTRL_H
#define MUSIKLISTCTRL_H

#include "myprec.h"


#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif 
#include "wx/listctrl.h"
#include "MusikDefines.h"

extern const wxEventType wxEVT_LISTSEL_CHANGED_COMMAND;
#define EVT_LISTSEL_CHANGED_COMMAND(id, fn) \
    DECLARE_EVENT_TABLE_ENTRY( \
    wxEVT_LISTSEL_CHANGED_COMMAND, id, wxID_ANY, \
    (wxObjectEventFunction)(wxEventFunction) wxStaticCastEvent( wxCommandEventFunction, &fn ), \
    (wxObject *) NULL \
    ),


class SuppressListItemStateEventsWrapper;

class CMusikListCtrl : public wxListCtrl
{
public:
	//--------------------------------//
	//--- constructor / destructor ---//
	//--------------------------------//
	CMusikListCtrl( wxWindow *parent, const wxWindowID id, const wxPoint& pos, const wxSize& size ,long style);
	~CMusikListCtrl(){};


	//--------------//
	//--- events ---//
	//--------------//
                                     
#ifdef __WXMSW__
	void OnEraseBackground(wxEraseEvent& event);
	void OnPaint(wxPaintEvent& event);
	void Freeze();
	void Thaw();
#endif
#ifdef __WXMSW__
	void SetItemCount(long count)
	{
		if(count != GetItemCount())
		{
			Freeze();
			EnsureVisible(0); // hack to circumvent bug, if SetItemCount is callec but the listview is scrolled down.
			wxListCtrl::SetItemCount(count);
			Thaw();
		}	
	}
#endif
	DECLARE_EVENT_TABLE()
protected:
    void SuppressListItemStateEvents(bool suppress)
    {
        m_bSuppressListItemStateEvents = suppress;
    }

    void OnListItemSel          ( wxListEvent& event );
    void OnListItemFocused      ( wxListEvent& event );
    void OnListSelChanged       (wxCommandEvent& event );

    void OnSize					( wxSizeEvent& event );

    void OnMouseWheel(wxMouseEvent & event);
	void OnMiddleDown(wxMouseEvent & event);
	void OnRightDown(wxMouseEvent & event);
	
#ifdef WXMUSIK_BUGWORKAROUND_LISTCTRL_CONTEXTMENU
	void ShowMenu				( wxListEvent& event );
#else	
	void ShowMenu				( wxContextMenuEvent& event );
#endif	

	//virtual void DoSetSize(int x, int y,int width, int height,int sizeFlags);
	virtual bool OnRescaleColumns();
	virtual wxMenu * CreateContextMenu(){ return NULL;}

	void SelectClickedItem(wxMouseEvent &event);
#ifdef __WXMSW__
    bool m_bHideHorzScrollbar;
    bool m_bHideVertScrollbar;
protected:
    long MSWWindowProc(WXUINT message, WXWPARAM wParam, WXLPARAM lParam);
#endif

private:
#ifdef __WXMSW__
	int m_freezeCount;
#endif
    bool m_bSuppressListItemStateEvents;

    
   friend class SuppressListItemStateEventsWrapper;
};

class SuppressListItemStateEventsWrapper
{
public:
    SuppressListItemStateEventsWrapper(CMusikListCtrl &lc)
        :m_lc(lc)
    {
        m_lc.SuppressListItemStateEvents(true);
    }
    ~SuppressListItemStateEventsWrapper()
    {
        m_lc.SuppressListItemStateEvents(false);
    }
private:
    CMusikListCtrl & m_lc; 
};

#endif
