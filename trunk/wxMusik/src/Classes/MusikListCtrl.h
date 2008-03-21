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
#if defined(__WXMAC__) 
//#define  USE_GENERICLISTCTRL   
#endif
#define USE_NATIVELISTCTRL_DOUBLEBUFFERED_PAINTING 1

#ifdef USE_GENERICLISTCTRL
#include "wxmod_listctrl.h"
#else
#include "wx/listctrl.h"
#endif
#include "MusikDefines.h"

extern const wxEventType wxEVT_LISTSEL_CHANGED_COMMAND;
#define EVT_LISTSEL_CHANGED_COMMAND(id, fn) \
    DECLARE_EVENT_TABLE_ENTRY( \
    wxEVT_LISTSEL_CHANGED_COMMAND, id, wxID_ANY, \
    (wxObjectEventFunction)(wxEventFunction) wxStaticCastEvent( wxCommandEventFunction, &fn ), \
    (wxObject *) NULL \
    ),
#ifdef USE_GENERICLISTCTRL
#define MUSIKLISTCTRLBASE  wxMod::wxGenericListCtrl
#else
#define MUSIKLISTCTRLBASE  wxListCtrl
#endif

class SuppressListItemStateEventsWrapper;

class CMusikListCtrl;



class CMusikListCtrl : public MUSIKLISTCTRLBASE
{
public:
	//--------------------------------//
	//--- constructor / destructor ---//
	//--------------------------------//
	CMusikListCtrl( wxWindow *parent, const wxWindowID id, const wxPoint& pos, const wxSize& size ,long style);
	~CMusikListCtrl(){};

    void SelectAll	(  );
    void SelectNone	(  );

	//--------------//
	//--- events ---//
	//--------------//
#ifndef USE_GENERICLISTCTRL                                     
#ifdef __WXMSW__
#if USE_NATIVELISTCTRL_DOUBLEBUFFERED_PAINTING
    void OnEraseBackground(wxEraseEvent& event);
	void OnPaint(wxPaintEvent& event);
#endif //USE_NATIVELISTCTRL_DOUBLEBUFFERED_PAINTING
#endif
#ifdef __WXMSW__
	void SetItemCount(long count)
	{
		if(count != GetItemCount())
		{
			Freeze();
			EnsureVisible(0); // hack to circumvent bug, if SetItemCount is callec but the listview is scrolled down.
			MUSIKLISTCTRLBASE::SetItemCount(count);
			Thaw();
		}	
	}
#endif
#endif//  !USE_GENERICLISTCTRL
	DECLARE_EVENT_TABLE()
protected:
    void SuppressListItemStateEvents(bool suppress)
    {
        m_bSuppressListItemStateEvents = suppress;
    }

    void OnListItemSel          ( wxListEvent& event );
    void OnListItemDesel         ( wxListEvent& event );
    void OnListItemFocused      ( wxListEvent& event );
    void OnListSelChanged       (wxCommandEvent& event );

    void OnSize					( wxSizeEvent& event );

    void OnLeftDown(wxMouseEvent &event);
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
    void FireListSelChanged(wxListEvent &event);
#ifdef __WXMSW__
    bool m_bHideHorzScrollbar;
    bool m_bHideVertScrollbar;
protected:
#ifndef USE_GENERICLISTCTRL
    WXLRESULT MSWWindowProc(WXUINT message, WXWPARAM wParam, WXLPARAM lParam);
#endif
#endif

private:

    bool m_bSuppressListItemStateEvents;
    
    bool m_bLISTSEL_CHANGED_Fired;
    
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
