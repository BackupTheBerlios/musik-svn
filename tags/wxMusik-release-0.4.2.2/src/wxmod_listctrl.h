/////////////////////////////////////////////////////////////////////////////
// Name:        wx/generic/listctrl.h
// Purpose:     Generic list control
// Author:      Robert Roebling
// Created:     01/02/97
// RCS-ID:      $Id: listctrl.260.h,v 1.3 2005/04/25 19:58:35 xaignar Exp $
// Copyright:   (c) 1998 Robert Roebling and Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef LISTCTRL_260_H
#define LISTCTRL_260_H

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma interface "wxmod_listctrl.h"
#endif

#include <wx/defs.h>
#include <wx/object.h>
#include <wx/control.h>
#include <wx/timer.h>
#include <wx/dcclient.h>
#include <wx/scrolwin.h>
#include <wx/settings.h>
#include <wx/listbase.h>

#if wxUSE_DRAG_AND_DROP
class wxDropTarget;
#endif

#define wxLC_OWNERDRAW 0x10000

//-----------------------------------------------------------------------------
// classes
//-----------------------------------------------------------------------------

class wxListItem;
class wxListEvent;

class wxImageList;
typedef wxImageList wxImageListType;

namespace wxMod
{
//-----------------------------------------------------------------------------
// internal classes
//-----------------------------------------------------------------------------

class wxListHeaderData;
class wxListItemData;
class wxListLineData;

class wxListHeaderWindow;
class wxListMainWindow;

class wxListRenameTimer;
class wxListTextCtrl;

//-----------------------------------------------------------------------------
// wxListCtrl
//-----------------------------------------------------------------------------

class wxGenericListCtrl: public wxControl
{
public:
    wxGenericListCtrl();
    wxGenericListCtrl( wxWindow *parent,
                wxWindowID winid = wxID_ANY,
                const wxPoint &pos = wxDefaultPosition,
                const wxSize &size = wxDefaultSize,
                long style = wxLC_ICON,
                const wxValidator& validator = wxDefaultValidator,
                const wxString &name = wxT("listCtrl"))
    {
        Create(parent, winid, pos, size, style, validator, name);
    }
    ~wxGenericListCtrl();

    bool Create( wxWindow *parent,
                 wxWindowID winid = wxID_ANY,
                 const wxPoint &pos = wxDefaultPosition,
                 const wxSize &size = wxDefaultSize,
                 long style = wxLC_ICON,
                 const wxValidator& validator = wxDefaultValidator,
                 const wxString &name = wxT("listCtrl"));

    bool GetColumn( int col, wxListItem& item ) const;
    bool SetColumn( int col, wxListItem& item );
    int GetColumnWidth( int col ) const;
    bool SetColumnWidth( int col, int width);
    int GetCountPerPage() const; // not the same in wxGLC as in Windows, I think

	void GetVisibleLines(long* first, long* last);
    wxRect GetViewRect() const;

    bool GetItem( wxListItem& info ) const;
    bool SetItem( wxListItem& info ) ;
    long SetItem( long index, int col, const wxString& label, int imageId = -1 );
    int  GetItemState( long item, long stateMask ) const;
    bool SetItemState( long item, long state, long stateMask);
    bool SetItemImage( long item, int image, int selImage = -1 );
    wxString GetItemText( long item ) const;
    void SetItemText( long item, const wxString& str );
    wxUIntPtr GetItemData( long item ) const;
    bool SetItemData( long item, long data );
    bool GetItemRect( long item, wxRect& rect, int code = wxLIST_RECT_BOUNDS ) const;
    bool GetItemPosition( long item, wxPoint& pos ) const;
    bool SetItemPosition( long item, const wxPoint& pos ); // not supported in wxGLC
    int GetItemCount() const;
    int GetColumnCount() const;
    void SetItemSpacing( int spacing, bool isSmall = false );
    wxSize GetItemSpacing() const;
    void SetItemTextColour( long item, const wxColour& col);
    wxColour GetItemTextColour( long item ) const;
    void SetItemBackgroundColour( long item, const wxColour &col);
    wxColour GetItemBackgroundColour( long item ) const;
    int GetSelectedItemCount() const;
    wxColour GetTextColour() const;
    void SetTextColour(const wxColour& col);
    long GetTopItem() const;

    void SetSingleStyle( long style, bool add = true ) ;
    void SetWindowStyleFlag( long style );
    void RecreateWindow() {}
    long GetNextItem( long item, int geometry = wxLIST_NEXT_ALL, int state = wxLIST_STATE_DONTCARE ) const;
    wxImageListType *GetImageList( int which ) const;
    void SetImageList( wxImageListType *imageList, int which );
    void AssignImageList( wxImageListType *imageList, int which );
    bool Arrange( int flag = wxLIST_ALIGN_DEFAULT ); // always wxLIST_ALIGN_LEFT in wxGLC

    void ClearAll();
    bool DeleteItem( long item );
    bool DeleteAllItems();
    bool DeleteAllColumns();
    bool DeleteColumn( int col );

    void SetItemCount(long count);

    void EditLabel( long item ) { Edit(item); }
    void Edit( long item );

    bool EnsureVisible( long item );
    long FindItem( long start, const wxString& str, bool partial = false );
    long FindItem( long start, wxUIntPtr data );
    long FindItem( long start, const wxPoint& pt, int direction ); // not supported in wxGLC
    long HitTest( const wxPoint& point, int& flags);
    long InsertItem(wxListItem& info);
    long InsertItem( long index, const wxString& label );
    long InsertItem( long index, int imageIndex );
    long InsertItem( long index, const wxString& label, int imageIndex );
    long InsertColumn( long col, wxListItem& info );
    long InsertColumn( long col, const wxString& heading,
                       int format = wxLIST_FORMAT_LEFT, int width = -1 );
    bool ScrollList( int dx, int dy );
    bool SortItems( wxListCtrlCompare fn, long data );
    bool Update( long item );

    // are we in report mode?
    bool InReportView() const { return HasFlag(wxLC_REPORT); }

    // are we in virtual report mode?
    bool IsVirtual() const { return HasFlag(wxLC_VIRTUAL); }

    // do we have a header window?
    bool HasHeader() const
        { return InReportView() && !HasFlag(wxLC_NO_HEADER); }

    // refresh items selectively (only useful for virtual list controls)
    void RefreshItem(long item);
    void RefreshItems(long itemFrom, long itemTo);

    // obsolete, don't use
    wxDEPRECATED( int GetItemSpacing( bool isSmall ) const );


    virtual wxVisualAttributes GetDefaultAttributes() const
    {
        return GetClassDefaultAttributes(GetWindowVariant());
    }

    static wxVisualAttributes
    GetClassDefaultAttributes(wxWindowVariant variant = wxWINDOW_VARIANT_NORMAL);

    // implementation only from now on
    // -------------------------------

    void OnInternalIdle( );
    void OnSize( wxSizeEvent &event );

    // We have to hand down a few functions
    virtual void Refresh(bool eraseBackground = true,
                         const wxRect *rect = NULL);

    virtual void Freeze();
    virtual void Thaw();
	virtual void OnDrawItem(int item,wxDC* dc,const wxRect& rect,const wxRect& rectHL,bool highlighted);

    virtual bool SetBackgroundColour( const wxColour &colour );
    virtual bool SetForegroundColour( const wxColour &colour );
    virtual wxColour GetBackgroundColour() const;
    virtual wxColour GetForegroundColour() const;
    virtual bool SetFont( const wxFont &font );
    virtual bool SetCursor( const wxCursor &cursor );

#if wxUSE_DRAG_AND_DROP
    virtual void SetDropTarget( wxDropTarget *dropTarget );
    virtual wxDropTarget *GetDropTarget() const;
#endif

    virtual bool DoPopupMenu( wxMenu *menu, int x, int y );

    virtual bool ShouldInheritColours() const { return false; }
    virtual void SetFocus();
	virtual bool GetFocus();

    virtual wxSize DoGetBestSize() const;

    // implementation
    // --------------

    wxImageListType         *m_imageListNormal;
    wxImageListType         *m_imageListSmall;
    wxImageListType         *m_imageListState;  // what's that ?
    bool                 m_ownsImageListNormal,
                         m_ownsImageListSmall,
                         m_ownsImageListState;
    wxListHeaderWindow  *m_headerWin;
    wxListMainWindow    *m_mainWin;
    wxCoord              m_headerHeight;

protected:
    // return the text for the given column of the given item
    virtual wxString OnGetItemText(long item, long column) const;

    // return the icon for the given item
    virtual int OnGetItemImage(long item) const;

    // return the attribute for the item (may return NULL if none)
    virtual wxListItemAttr *OnGetItemAttr(long item) const;

    // it calls our OnGetXXX() functions
    friend class wxListMainWindow;

private:
    // Virtual function hiding supression
    virtual void Update() { wxWindow::Update(); }

    // create the header window
    void CreateHeaderWindow();

    // calculate and set height of the header
    void CalculateAndSetHeaderHeight();

    // reposition the header and the main window in the report view depending
    // on whether it should be shown or not
    void ResizeReportView(bool showHeader);

    DECLARE_EVENT_TABLE()
};

#if !defined(__WXMSW__) || defined(__WXUNIVERSAL__)
/*
 * wxListCtrl has to be a real class or we have problems with
 * the run-time information.
 */

class wxListCtrl: public wxGenericListCtrl
{
public:
    wxListCtrl() {}

    wxListCtrl(wxWindow *parent, wxWindowID winid = wxID_ANY,
               const wxPoint& pos = wxDefaultPosition,
               const wxSize& size = wxDefaultSize,
               long style = wxLC_ICON,
               const wxValidator &validator = wxDefaultValidator,
               const wxString &name = wxT("listCtrl"))
    : wxGenericListCtrl(parent, winid, pos, size, style, validator, name)
    {
    }
};

#endif // !__WXMSW__ || __WXUNIVERSAL__

}

#endif // __LISTCTRLH_G__
