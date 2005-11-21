/*
 *  ActivityBox.cpp
 *
 *  The infamous "Activity Box" control
 *	These controls are visible at the top of the main dialog.
 *	They contain lists of artists / albums / genres / etc.
 *
 *  Copyright (c) 2003 Casey Langen (casey@bak.rr.com)
 *	Contributors: Simon Windmill, Dustin Carter, Gunnar Roth, Wade Brainerd
 *
 *  See the file "license.txt" for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL WARRANTIES.
*/

// For compilers that support precompilation, includes "wx/wx.h".
#include "myprec.h"
#include "ActivityBox.h"
#include "ActivityAreaCtrl.h"
#include "Classes/PlaylistCtrl.h" //TODO: remove the dependancy
//--- globals ---//
#include "MusikUtils.h"
#include "MusikGlobals.h"

//--- frames ---//
#include "Frames/MusikFrame.h"

//--- threads ---//
#include "Threads/ActivityBoxThreads.h"

#include "DNDHelper.h"
//-----------------------//
//--- CActivityBoxEvt ---//
//-----------------------//
BEGIN_EVENT_TABLE(CActivityBoxEvt, wxEvtHandler)
	EVT_CHAR				( CActivityBoxEvt::TranslateKeys	)
END_EVENT_TABLE()

void CActivityBoxEvt::TranslateKeys( wxKeyEvent& event )
{
	if ( event.GetKeyCode() == WXK_F2 )
		pParent->EditBegin();
  else  
	  event.Skip();
}

//------------------------//
//--- CActivityEditEvt ---//
//------------------------//
BEGIN_EVENT_TABLE(CActivityEditEvt, wxEvtHandler)
	EVT_CHAR	( CActivityEditEvt::TranslateKeys )
END_EVENT_TABLE()

void CActivityEditEvt::TranslateKeys( wxKeyEvent& event )
{
	int nKey = event.GetKeyCode();
	if ( nKey == WXK_ESCAPE )
		pParent->EditCancel();
	else if ( nKey == WXK_RETURN )
		pParent->EditCommit();
	else  
  		event.Skip( TRUE );
}


//helpers
wxString wxStringRemovePrefix(const wxString &s)
{
    const wxChar* Prefix = BeginsWithPreposition(s);
    if(Prefix)
    {
        wxString r = s.Right(s.size() - wxStrlen(Prefix));
        return r;
    }
    return s;
}
// NOTE: wxStrcoll sorts by using the current locale. This means they are sorted lexically.
// case is ignored in th lexically order of most locales ( don't know why MS has a separate stricoll function)
int wxCMPFUNC_CONV wxStringSortAscendingLocaleRemovePrefix(const wxString& s1, const wxString& s2)
{
    return wxStrcoll(wxStringRemovePrefix(s1).c_str(), wxStringRemovePrefix(s2).c_str());
}

int wxCMPFUNC_CONV wxStringSortAscendingLocale(const wxString& s1,const  wxString& s2)
{
    return wxStrcoll(s1.c_str(), s2.c_str());
}

//------------------------//
//--- CActivityListBox ---//
//------------------------//
CActivityListBox::CActivityListBox( CActivityBox *parent,  wxWindowID id )
	: CMusikListCtrl( parent, id, wxPoint( -1, -1 ), wxSize( -1, -1 ),wxNO_BORDER|wxLC_NO_SORT_HEADER)
{
	m_Related = 0;
	m_pParent = parent;
#ifdef __WXMSW__
    m_bHideHorzScrollbar = true;
#endif
    InsertColumn( 0, wxT(""), wxLIST_FORMAT_LEFT, 0 );
//	InsertColumn( 1, wxT(""), wxLIST_FORMAT_LEFT, 0 );
}
BEGIN_EVENT_TABLE(CActivityListBox, CMusikListCtrl)
	EVT_CHAR	( CActivityListBox::OnChar )
    EVT_LIST_COL_BEGIN_DRAG		(wxID_ANY,	CActivityListBox::OnColSize		)
END_EVENT_TABLE()

wxMenu * CActivityListBox::CreateContextMenu()
{ 
    return m_pParent->CreateContextMenu();
}	
void CActivityListBox::OnChar(wxKeyEvent& event)
{
#ifdef __WXMSW__
#if wxUSE_UNICODE
	wxChar keycode=event.GetUnicodeKey();
#else
	int keycode=event.GetKeyCode();
#endif
#else
	int keycode=event.GetKeyCode();
#endif
	if ((event.GetKeyCode() >=WXK_SPACE) && (event.GetKeyCode() <= 255))
	{
		if(m_OnCharStopWatch.Time()>1000)
		{ // More than 1000 ms have passed since the last character was entered.
			// Reset the search string.
			m_sSearch.Empty();
		}
		m_sSearch+=keycode;
		ScrollToItem(m_sSearch);
		m_OnCharStopWatch.Start();
	} 
	else 
		event.Skip();
}
void CActivityListBox::ScrollToItem(const wxString & sItem, bool bCenter,bool bSelectItem)
{
    bool bSortNoCaseNoPrefix = wxGetApp().Prefs.bSortArtistWithoutPrefix && g_PlaylistColumn[m_pParent->ColId()].SortOrder  == PlaylistColumn::SortNoCaseNoPrefix;
	for (int i=HasShowAllRow()?1:0;i<GetItemCount();++i)
	{
        wxString rt = GetRowText(i,false).Left(sItem.Len());
        if ((bSortNoCaseNoPrefix ? wxStringSortAscendingLocaleRemovePrefix (rt, sItem): wxStringSortAscendingLocale(rt, sItem)) >= 0)
		{ // Scroll this item to right position in the list.
            ScrollToItem(i,bCenter,bSelectItem);
			break;
		}
	}

}
void CActivityListBox::ScrollToItem(long nItem, bool bCenter,bool bSelectItem)
{
    int showitem=0;
    int offset = GetCountPerPage() - 1;// show item on top of list
    if(bCenter)
    {
        offset = (GetCountPerPage() - 1) / 2;// show item in center of list
    }
    if (nItem >= GetTopItem()) 
    {
        // We need to ensure visibility of an item further down the list to
        // move the matching item to the offset.
        showitem = wxMin(nItem + offset, GetItemCount()-1);
    } 
    else
    {
        // nItem < GetTopItem() , so EnsureVisible(m) will scroll backwards, 
        // this will bring the item m automatically to offset
        showitem = wxMax(0, nItem - offset);
    }
    EnsureVisible(showitem);
    if(bSelectItem)
    {
        SetItemState(nItem,wxLIST_STATE_SELECTED,wxLIST_STATE_SELECTED);
    }
    else
    {
        SuppressListItemStateEventsWrapper(*this);
        // Move the focus (*not* the selection) to the matching item.
        SetItemState(nItem,wxLIST_STATE_FOCUSED,wxLIST_STATE_FOCUSED);
    }
}
void CActivityListBox::RescaleColumns( )
{
	int nWidth, nHeight;
	GetClientSize	( &nWidth, &nHeight );
    const int main_col = 0;
	if ( GetColumnWidth( main_col ) != nWidth )
	{
		#if defined(__WXMSW__) && !defined(USE_GENERICLISTCTRL)
			SetColumnWidth	( main_col, nWidth );
		#else
			SetColumnWidth( main_col, nWidth - wxSystemSettings::GetMetric(wxSYS_HSCROLL_Y) /*- GetColumnWidth( 0 )*/ - 1 );			
		#endif 
	}
}

void CActivityListBox::SetCaption( const wxString & sCaption )
{
	m_sCaption = sCaption;
	RefreshCaption();
}
void CActivityListBox::RefreshCaption()
{
	wxListItem item;
	item.SetId( 1 );
	item.SetMask( wxLIST_MASK_TEXT );
	item.SetText( wxString::Format(wxT("%s (%d)"), m_sCaption.c_str(),m_Items.GetCount()));
	SetColumn( 0, item );
}

void CActivityListBox::SetList( const wxArrayString &  aList ,enum eResetContentMode rcm)
{
    
    long  nTopItem = HasShowAllRow() && GetTopItem() > 0 ? GetTopItem() - 1 : GetTopItem();
    wxString sCurrentTopItem;
    if(m_Items.GetCount() && nTopItem >= 0 && nTopItem < (long)m_Items.GetCount())
    {
        sCurrentTopItem = m_Items[nTopItem];
    }

    wxArrayString sSelectedItems;
    wxString sFirstVisibleSelectedItem;
    if(rcm == RCM_PreserveSelectedItems)
    {
        long nFirstVisibleSelectedItem =  GetNextItem( nTopItem, wxLIST_NEXT_ALL , wxLIST_STATE_SELECTED );
        nFirstVisibleSelectedItem = HasShowAllRow() && nFirstVisibleSelectedItem > 0 ? nFirstVisibleSelectedItem - 1 : nFirstVisibleSelectedItem;
        if(m_Items.GetCount() && nFirstVisibleSelectedItem >= 0 && nFirstVisibleSelectedItem < (long)m_Items.GetCount())
        {
            sFirstVisibleSelectedItem = m_Items[nFirstVisibleSelectedItem];
        }
  
        GetSelected(sSelectedItems);
    }
    m_Items = aList;
    Update( rcm == RCM_DeselectAll );
    if(sSelectedItems.GetCount() == 0 && rcm == RCM_EnsureVisibilityOfCurrentTopItem && !sCurrentTopItem.IsEmpty())
    {
            ScrollToItem(sCurrentTopItem,false);
    }
    else if(sSelectedItems.GetCount())
    {
        // we have to preserve the selection
        // in fact this means we have to restore the selection
        {
            // restore 1-(n-1) sel items, sel events will be suppressed
            SuppressListItemStateEventsWrapper x(*this);
            SelectNone( );
            for ( size_t i = 1; i < sSelectedItems.GetCount(); i++ )
            {
                SetSel(sSelectedItems.Item( i ), false,false);
            }
        }
        // select the previously first selected item
        SetSel(sSelectedItems.Item( 0 ), false,false);// NOTE:this triggers a sel change event
        ScrollToItem(sFirstVisibleSelectedItem,true);
    }
}

void CActivityListBox::Update( bool selnone )
{
	Freeze();
	//----------------------------------------------------------------------------------//
	//---        note that the activity box control is now virtual, so we don't      ---//
	//---    add items directly to it.  Instead, we have the object pItems a		 ---//
	//---  *wxArrayString, that the virtual listctrl then references via callbacks   ---//
	//----------------------------------------------------------------------------------//

	//--- update colors from prefs ---//
	m_LightAttr		= wxListItemAttr( wxSystemSettings::GetColour( wxSYS_COLOUR_WINDOWTEXT), wxSystemSettings::GetColour( wxSYS_COLOUR_BTNHIGHLIGHT ), wxNullFont );
	m_DarkAttr		= wxListItemAttr( wxSystemSettings::GetColour( wxSYS_COLOUR_WINDOWTEXT), StringToColour( wxGetApp().Prefs.sActStripeColour ), wxNullFont );
	m_ActiveAttr	= wxListItemAttr( wxSystemSettings::GetColour( wxSYS_COLOUR_HIGHLIGHTTEXT ), wxSystemSettings::GetColour( wxSYS_COLOUR_HIGHLIGHT ), wxNullFont );
	m_AllReset		= wxListItemAttr( wxSystemSettings::GetColour( wxSYS_COLOUR_WINDOWTEXT), wxSystemSettings::GetColour( wxSYS_COLOUR_BTNHIGHLIGHT ), g_fntListBold );

	SetItemCount( GetRowCount() );
	RescaleColumns();
	if ( selnone )
    {
        SuppressListItemStateEventsWrapper x(*this);
		SelectNone( );
    }
	RefreshCaption();
	Thaw();
}


//---------------------------------------------------//
//--- virtual functions. careful with these ones, ---// 
//---        boys. they drive everything          ---//
//---------------------------------------------------//
wxString CActivityListBox::OnGetItemText(long item, long column) const
{
	switch ( column )
	{
//	case 0:
//		break;

	case 0:
		{
			wxString text(GetRowText( item,false ));
			if(text.IsEmpty())
				return _( "<unknown>" );
			else
				return text;
		}
		break;
	}
	return wxT("");
}
bool CActivityListBox::HasShowAllRow() const 
{ 
	return (wxGetApp().Prefs.eSelStyle == MUSIK_SELECTION_TYPE_STANDARD || wxGetApp().Prefs.eSelStyle == MUSIK_SELECTION_TYPE_SLOPPY);
}
wxString CActivityListBox::GetRowText( long row, bool bPure ) const
{
		if( row == 0 && HasShowAllRow())
			return wxString::Format(wxT("%s%s"),_("Show all "), m_sCaption.c_str());
		if(HasShowAllRow())
			row--;
        if(!bPure && g_PlaylistColumn[m_pParent->ColId()].Type == PlaylistColumn::Date)
        {
            double dbl = CharStringToDouble(ConvW2A(m_Items.Item( row ))); //CharStringToDouble always uses "." as decimal point no matter what the locale is.
            
            return  JDN2LocalTimeString(dbl,wxT("%x"));
        }
        else if(!bPure && g_PlaylistColumn[m_pParent->ColId()].Type == PlaylistColumn::Duration)
        {
            long l=0;
            m_Items.Item( row ).ToLong(&l); 
            return MStoStr(l*60000,true);
        }
        else if(wxGetApp().Prefs.bSortArtistWithoutPrefix && !bPure && g_PlaylistColumn[m_pParent->ColId()].SortOrder  == PlaylistColumn::SortNoCaseNoPrefix)
			return MoveArtistPrefixToEnd(SanitizedString( m_Items.Item( row ) ));
		else
			return SanitizedString( m_Items.Item( row ) );
}
wxListItemAttr* CActivityListBox::OnGetItemAttr(long item) const
{
#ifdef __WXMAC__
	wxListItemAttr *pDefAttr = 	NULL;
#else
	wxListItemAttr *pDefAttr = 	(wxListItemAttr*)&m_LightAttr;
#endif

	if ( wxGetApp().Prefs.eSelStyle == MUSIK_SELECTION_TYPE_HIGHLIGHT && item < m_Related )
		return ( wxListItemAttr* )&m_AllReset;

	else if ( item == 0 && HasShowAllRow())
		return ( wxListItemAttr* )&m_AllReset;

	if ( wxGetApp().Prefs.bActStripes == 1 )
		return item % 2 ? (wxListItemAttr *)&m_DarkAttr : pDefAttr;
	return pDefAttr;
}

CActivityListBox::~CActivityListBox()
{
}

bool CActivityListBox::IsSelected( int n )
{
	if ( GetItemState( n, wxLIST_STATE_SELECTED ) > 2 )
		return true;
	else
		return false;
}


void CActivityListBox::SetSel( const  wxArrayString & aList )
{
	SelectNone();
	for ( size_t i = 0; i < aList.GetCount(); i++ )
	{
		SetSel(aList.Item( i ), false,false);
	}
}

void CActivityListBox::SetSel( const wxString & sel,bool bEnsureVisible , bool bDeselectAllFirst )
{
	if(bDeselectAllFirst)
		SelectNone();
	bool bFound = false;
	for ( size_t i = 0; i < GetRowCount(); i++ )
	{
		if (GetRowText(i) == sel )
		{
			if(bEnsureVisible && !bFound)
			{
				EnsureVisible(i);
				SetItemState( i, wxLIST_STATE_SELECTED|wxLIST_STATE_FOCUSED, wxLIST_STATE_SELECTED|wxLIST_STATE_FOCUSED );
            }
            else
            {
            	SetItemState( i, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
            }
			bFound = true;
		}
		else if(bFound)
		{  // not what we search for, but we have found one before
			// so break here.
			break;
		}
	}
}

void CActivityListBox::GetSelected(wxArrayString & aReturn)
{
	aReturn.Clear();
	int nIndex = -1;
	for( ;; )
	{
		nIndex = GetNextItem( nIndex, wxLIST_NEXT_ALL , wxLIST_STATE_SELECTED );
		if ( nIndex == -1 )
			break;
		aReturn.Add( GetRowText( nIndex ) );
	}
	return;
}

wxString CActivityListBox::GetFirstSel()
{
	int nIndex = -1;
	nIndex = GetNextItem( nIndex, wxLIST_NEXT_ALL , wxLIST_STATE_SELECTED );
	if ( nIndex > -1 )
	{
			return( GetRowText( nIndex ) );
	}
	return wxT("");
}

void CActivityListBox::SetRelated( int n )
{
	if((n == -1) && (m_Related > 0)	)
	{
		//		int nIndex = GetNextItem( -1, wxLIST_NEXT_ALL , wxLIST_STATE_SELECTED );
		m_pParent->ResetContents();
	}
	m_Related = n; 
}

void CActivityBox::SetRelated( int n )
{
	pListBox->SetRelated( n ); 
}
//--------------------//
//--- CActivityBox ---//
//--------------------//

BEGIN_EVENT_TABLE(CActivityBox, wxPanel)
	//---------------------------------------------------------//
	//--- threading events.. we use EVT_MENU becuase its	---//
	//--- nice and simple, and gets the job done. this may	---//
	//--- become a little prettier later, but it works.		---//
	//---------------------------------------------------------//
    EVT_MENU			( MUSIK_ACTIVITY_RENAME_THREAD_START,	CActivityBox::OnRenameThreadStart	)
	EVT_MENU			( MUSIK_ACTIVITY_RENAME_THREAD_END,		CActivityBox::OnRenameThreadEnd		)
	EVT_MENU			( MUSIK_ACTIVITY_RENAME_THREAD_PROG,	CActivityBox::OnRenameThreadProg	)

	EVT_LIST_BEGIN_DRAG			( -1, CActivityBox::OnActivityBoxSelDrag	)

	EVT_MENU					( MUSIK_PLAYLIST_CONTEXT_PLAY_INSTANTLY,			CActivityBox::OnPlayInstantly		)
	EVT_MENU					( MUSIK_PLAYLIST_CONTEXT_PLAY_ASNEXT,				CActivityBox::OnPlayAsNext			)
	EVT_MENU					( MUSIK_PLAYLIST_CONTEXT_PLAY_ENQUEUED,				CActivityBox::OnPlayEnqueued		)
	EVT_MENU					( MUSIK_PLAYLIST_CONTEXT_PLAY_REPLACE_PLAYERLIST_WITH_SELECTION,CActivityBox::OnPlayReplaceWithSel		)
	EVT_MENU					( MUSIK_SOURCE_CONTEXT_RENAME,						CActivityBox::OnRename				)	
    EVT_LIST_ITEM_MIDDLE_CLICK      (-1, CActivityBox::OnListItemMiddleClick)
END_EVENT_TABLE()
#if 0 // old code
//-----------------//
//--- dnd stuff ---//
//-----------------//
bool ActivityDropTarget::OnDropText( wxCoord x, wxCoord y, const wxString &text )
{
	wxString sFiles = text;
	long n;
	if ( sFiles != wxT( "" ) )
	{
		//--- separate values from type. "s\n" for Sources, "p\n" for playlist, "a\n" for activity box ---//
		wxString sType = sFiles.Left( 2 );
		sFiles = sFiles.Right( sFiles.Length() - 2 );

		//--- where did we land? ---//
		const wxPoint& pt = wxPoint( x, y );
		int nFlags;
		n = pList->HitTest( pt, nFlags );

		//--- hit nothing, do nothing ---//
		if ( n < 0 )
			return FALSE;

		//--- drag originated from playlist or activity area (others are ignored) ---//
		if ( sType == wxT( "p\n" ) )	
		{
			//--- drag over an existing item, convert id3 tags ---//
			wxArrayString files;
			wxString file = sFiles.BeforeFirst( wxT('\n') );
			while ( !file.IsEmpty() )
			{

				files.Add( file );

				sFiles = sFiles.AfterFirst( wxT('\n') );

				file = sFiles.BeforeFirst( wxT('\n') );

			}
			pActivityCtrl->StartRenameThread( 
				ACTIVITY_RENAME_SONGS,
				files, 
				SanitizedString( pList->GetList().Item( n ) ) );
		}
	}

	return TRUE;
}

wxDragResult ActivityDropTarget::OnDragOver(wxCoord x, wxCoord y, wxDragResult def)
{
	if(def == wxDragNone)
		return wxDragNone;
	// Disallow further drag and drop if a thread is happening.
	if ( g_MusikFrame->GetActiveThread() )
		return wxDragNone;

	const wxPoint& pt = wxPoint( x, y );
	HighlightSel( pt );
	return wxDragMove;
}

void ActivityDropTarget::HighlightSel( wxPoint pPos )
{
	int nFlags;
	long n = pList->HitTest( pPos, nFlags );
	if ( ( n > 0 ) && ( n != nLastHit ) )
	{
		pList->SetItemState( n, wxLIST_STATE_FOCUSED, wxLIST_STATE_FOCUSED );
	}
	nLastHit = n;
}
#endif // 0 old code
CActivityBox::CActivityBox( wxWindow *parent, wxWindowID id, PlaylistColumn::eId ColId )
	:  wxPanel( parent, -1, wxPoint( -1, -1 ), wxSize( -1, -1 ),wxTAB_TRAVERSAL | wxNO_BORDER | wxCLIP_CHILDREN )
{
    m_ParentBox = NULL;
	m_EditVisible = false;
	m_ColId = ColId;
	//--- CActivityListBox ---//
	pListBox	= new CActivityListBox	( this, id );
	pListBox->SetCaption(DisplayName());
	//--- drag and drop handler ---//
	// what is the drag and drop handler for in this case? it just disturbs dragging
	// from playlist to sources box, if you cross the listbox area. after the dragging the playlist display changes to 
	// according to the selected entry in one of the activity boxes. bad!
	// and i see no use in dragging with the activity boxes. 
	// its not very handy in renaming files and it is not intuitive too.
	// furthermore it crashes very often, because both threads access the db.
	// the renaming thread  because he renames, and the main thread because he an entry of the activity box was
	// selected and therefore refreshes the playlist.
	//pListBox->SetDropTarget( new ActivityDropTarget( this ) );

	//--- text control to change val ---//
	pEdit		= new wxTextCtrl( this, MUSIK_ACT_TEXT, wxT(""), wxPoint( -1, -1 ), wxSize( -1, -1 ), wxSIMPLE_BORDER | wxTE_PROCESS_ENTER );

	//--- top sizer ---//
	pSizer = new wxBoxSizer( wxVERTICAL );
	pSizer->Add( pListBox, 1, wxEXPAND, 0 );
	pSizer->Add( pEdit, 0, wxEXPAND, 0 );
	pSizer->Show( pEdit, FALSE );

	pEdit->Enable( FALSE );

	SetSizerAndFit( pSizer );
	Layout();

	//--- push event handlers ---//
	pActivityBoxEvt = new CActivityBoxEvt( this, pListBox );
	pListBox->PushEventHandler( pActivityBoxEvt );

	pActivityEditEvt = new CActivityEditEvt( this, pEdit );
	pEdit->PushEventHandler( pActivityEditEvt );

}


CActivityBox::~CActivityBox()
{
	delete pListBox;
	delete pEdit;
	delete pActivityBoxEvt;
	delete pActivityEditEvt;
}


wxString CActivityBox::DisplayName()
{
    return wxGetTranslation(g_PlaylistColumn[ColId()].Label);
}




void CActivityBox::GetRelatedList( CActivityBox *pParentBox, wxArrayString & aReturn )
{
    m_ParentBox = pParentBox;

	aReturn.Clear();
    const PlaylistColumn & ColumnOut =  g_PlaylistColumn[ColId()];
    bool bLetDBSort = ColumnOut.Type != PlaylistColumn::Textual; // texts cannot be sorted by the db right now, 
                                                                 //because it does not sort utf-8 strings correctly.
                                                                 // this will change if sqlite 3 is used.
    if(m_ParentBox)
    {
        const PlaylistColumn & ColumnIn	= g_PlaylistColumn[m_ParentBox->ColId()];
        wxArrayString sel;
        m_ParentBox->GetSelected( sel );
        wxGetApp().Library.GetInfo( sel, ColumnIn, ColumnOut, aReturn ,bLetDBSort);
    }
    else
    {
        GetFullList(aReturn,bLetDBSort);
    }
    if(!bLetDBSort)
    {
        if(wxGetApp().Prefs.bSortArtistWithoutPrefix && g_PlaylistColumn[ColId()].SortOrder  == PlaylistColumn::SortNoCaseNoPrefix)
            aReturn.Sort(wxStringSortAscendingLocaleRemovePrefix);
        else
            aReturn.Sort(wxStringSortAscendingLocale);
    }
}


void CActivityBox::ResetContents(enum eResetContentMode rcm)
{
    m_ParentBox = NULL;
	wxArrayString list;
	GetRelatedList(NULL,list);
 	SetContents( list , rcm);
}
void CActivityBox::ReloadContents()
{
    if ( m_ParentBox == NULL)
    {
        ResetContents(RCM_PreserveSelectedItems);
        return;
    }
    wxArrayString list;
    GetRelatedList( m_ParentBox , list );
    SetContents( list , RCM_PreserveSelectedItems);
}

void CActivityBox::GetFullList( wxArrayString & aReturn ,bool bSorted)
{
	aReturn.Clear();
	wxGetApp().Library.GetAllOfColumn( g_PlaylistColumn[ColId()], aReturn ,bSorted);
}

void CActivityBox::GetSelectedSongs( MusikSongIdArray& array )
{
	//-----------------------------------------------------//
	//--- if we have "highlight" entries, or this is	---//
	//--- the parent box, select all the related		---//
	//--- material without special regard.				---//
	//-----------------------------------------------------//
	if ( g_ActivityAreaCtrl->GetParentBox() == this || g_ActivityAreaCtrl->GetParentBox() == NULL )
	{
	  wxArrayString list;
	  GetSelected( list );
      if(list.GetCount())
	    wxGetApp().Library.GetSongs( list, g_PlaylistColumn[ColId()], array );
	  return;
	}
 
	//---------------------------------------------------------//
	//--- standard or sloppy style. this routine			---//
	//--- helps assure that, for example, if two artists	---//
	//--- share a common album name (such as "unplugged").	---//
	//--- only the correct, selected artist's album songs	---//
	//--- get displayed										---//
	//---------------------------------------------------------//
	else if ( wxGetApp().Prefs.eSelStyle == MUSIK_SELECTION_TYPE_STANDARD || wxGetApp().Prefs.eSelStyle == MUSIK_SELECTION_TYPE_SLOPPY )
	{
		CActivityBox *pParentBox = g_ActivityAreaCtrl->GetParentBox();
		if ( pParentBox != NULL )
		{
			//-------------------------------------------------//
			//--- what type of box is this?					---//
			//-------------------------------------------------//
			wxString sThisType = wxString::Format(g_PlaylistColumn[ColId()].ColQueryMask,g_PlaylistColumn[ColId()].DBName.c_str());

			//-------------------------------------------------//
            //--- what type of box is the parent?			---//
			//-------------------------------------------------//
			wxString sParentType = wxString::Format(g_PlaylistColumn[pParentBox->ColId()].ColQueryMask,g_PlaylistColumn[pParentBox->ColId()].DBName.c_str());

			//-------------------------------------------------//
            //--- return if there is an invalid type		---//
			//-------------------------------------------------//
			if ( sThisType.IsEmpty() || sParentType.IsEmpty() )
				return;

			//-------------------------------------------------//
			//--- get selected items from both boxes.		---//
			//------------------------------------------------//
			wxArrayString aParentSel, aThisSel;
			pParentBox->GetSelected( aParentSel );
			GetSelected( aThisSel );

			//-------------------------------------------------//
			//--- preallocate the strings for optimization	---//
			//-------------------------------------------------//
			wxString sThis, sParent;
			sThis.Alloc( aThisSel.GetCount() * 40 );
			sParent.Alloc( aParentSel.GetCount() * ( 50 + sThis.Length() + sParentType.Length() ) );

			//-------------------------------------------------//
			//--- this is the current box'es portion of the	---//
			//--- query. will be something like "select		---//
			//--- [all this box'es artists]" from...		---//
			//-------------------------------------------------//
			sThis += sThisType + wxT(" = ");
			for ( size_t i = 0; i < aThisSel.GetCount(); i++ )
			{
				aThisSel.Item( i ).Replace( wxT( "'" ), wxT( "''" ), true );
				sThis += wxT("'");
				sThis += aThisSel.Item( i );
				
				if ( i == ( aThisSel.GetCount() - 1 ) )
				      sThis += wxT("'");				
				else
				{
				      sThis += wxT("' or ");
				      sThis += sThisType;
				      sThis += wxT(" = ");
				}
			}

			//-------------------------------------------------//
			//--- parent box'es portion of the query. which	---//
			//--- corresponding entries are selected in the	---//
			//--- other box?								---//
			//-------------------------------------------------//
			if(aParentSel.GetCount() > 0)
			{
				sParent = sParentType + wxT(" = "); 
				for ( size_t i = 0; i < aParentSel.GetCount(); i++ )
				{
					aParentSel.Item( i ).Replace( wxT( "'" ), wxT( "''" ), true );
					sParent += wxT("'");
					sParent += aParentSel.Item( i );
					sParent += wxT("' and " );
					sParent += sThis;
					if ( i != ( aParentSel.GetCount() - 1 ) )
					//--- not last item, so format string for another ---//
					{
						sParent += wxT(" or ");
						sParent += sParentType;
						sParent += wxT(" = ");
					}
				}
			}
			else
			  sParent = sThis;

			wxGetApp().Library.QuerySongsWhere( sParent, array ,true);
			return;
		}
	}
}

void CActivityBox::SetPlaylist()
{
	GetSelectedSongs( g_thePlaylist );
	g_PlaylistBox->SetPlaylist( &g_thePlaylist );
}

void CActivityBox::SetContents( const wxArrayString & aList , enum eResetContentMode rcm)
{

	pListBox->SetList( aList, rcm);
	
}
//------------------------//
//--- tag info editing ---//
//------------------------//
void CActivityBox::EditBegin()
{
    if(g_PlaylistColumn[ColId()].bEditableInActivityBox == false)
        return;
	//-- disallow further renaming until the current job is finished --//
	if ( g_MusikFrame->GetActiveThread() )
		return;

	wxString sVal = GetFirstSel();
	pSizer->Show( pEdit, TRUE );
	pEdit->Enable( TRUE );
	Layout();
	pEdit->SetValue( sVal );
	pEdit->SetFocus( );
	pEdit->SetSelection( -1, -1 );
	pListBox->Enable( false );
	m_EditVisible = true;
}

void CActivityBox::EditCancel()
{
	EditDone();
}

void CActivityBox::EditCommit()
{
	//-- disallow further renaming until the current job is finished --//
	if ( g_MusikFrame->GetActiveThread() )
		return;

	pEdit->Enable( FALSE );

	wxArrayString sel;
	GetSelected(sel);
	StartRenameThread( ACTIVITY_RENAME_ACTIVITY, sel, pEdit->GetValue() );
}

void CActivityBox::EnableProgress( bool enable )
{
	pSizer->Show( pEdit, !enable );
	pListBox->Enable( !enable );
	if ( !enable )
		EditDone();
	Layout();
}

void CActivityBox::EditDone()
{
	pSizer->Show( pEdit, FALSE );
	pListBox->Enable( true );
	Layout();
	m_EditVisible = false;
}

//-------------------//
//--- drag 'n drop---//
//-------------------//
void CActivityBox::DNDBegin()
{
	wxString sDrop = DNDGetList();

	//-------------------------------------------------------//
	//--- initialize drag and drop                        ---//
	//--- Playlist / Sources should take care of the rest ---//
	//-------------------------------------------------------//
	wxDropSource dragSource( this );
	CMusikSonglistDataObject song_data( sDrop );
	dragSource.SetData( song_data );
	dragSource.DoDragDrop( TRUE );
}

wxString CActivityBox::DNDGetList()
{
	//--- get selected items ---//
	MusikSongIdArray songs;
	GetSelectedSongs( songs );

	//--- add songs to dnd string ---//
	wxString sRet;
	sRet.Alloc(255 * songs.GetCount());
	for ( size_t i = 0; i < songs.GetCount(); i++ )
	{
		sRet += wxString() << (int)songs.Item( i );
		if	( i != ( songs.GetCount() - 1 ) )
			sRet += wxT("\n");
	}
	return sRet;
}


wxMenu * CActivityBox::CreateContextMenu()
{
	//Play menu
	wxMenu * context_play_menu = new wxMenu;
	context_play_menu->Append( MUSIK_PLAYLIST_CONTEXT_PLAY_ASNEXT , _( "Next" ), wxT( "" ) );
	context_play_menu->Append( MUSIK_PLAYLIST_CONTEXT_PLAY_ENQUEUED, _( "Enqueue" ), wxT( "" ) );
	context_play_menu->Append( MUSIK_PLAYLIST_CONTEXT_PLAY_INSTANTLY , _( "Instantly" ), wxT( "" ));
	context_play_menu->Append( MUSIK_PLAYLIST_CONTEXT_PLAY_REPLACE_PLAYERLIST_WITH_SELECTION, _( "Replace current playlist with selection" ), wxT( "" ) );
	//--- main context menu ---//
	wxMenu *context_menu = new wxMenu;
	context_menu->Append( MUSIK_PLAYLIST_CONTEXT_PLAYNODE,	_( "&Play" ),	context_play_menu );
	context_menu->Append( MUSIK_SOURCE_CONTEXT_RENAME, _( "&Rename\t F2" ) );

	return context_menu;
}

void CActivityBox::OnListItemMiddleClick( wxListEvent& WXUNUSED(event) )
{
    wxCommandEvent ev(0);
    if(wxGetKeyState(WXK_SHIFT))
    {
        OnPlayAsNext(ev);    
    }
    else if(wxGetKeyState(WXK_CONTROL))
    {
        OnPlayEnqueued(ev);    
    }
    else
    {
        OnPlayInstantly(ev);
    }
}

void CActivityBox::OnPlayInstantly( wxCommandEvent& WXUNUSED(event) )
{
	MusikSongIdArray aResult;
	GetSelectedSongs(aResult);
	wxGetApp().Player.InsertToPlaylist(aResult);

}
void CActivityBox::OnPlayAsNext ( wxCommandEvent& WXUNUSED(event) )
{
	MusikSongIdArray aResult;
	GetSelectedSongs(aResult);
	wxGetApp().Player.InsertToPlaylist(aResult,wxGetApp().Player.IsPlaying() ? false : true);

}
void CActivityBox::OnPlayEnqueued	( wxCommandEvent& WXUNUSED(event) )
{
	MusikSongIdArray aResult;
	GetSelectedSongs(aResult);
	wxGetApp().Player.AddToPlaylist(aResult,wxGetApp().Player.IsPlaying() ? false : true);
}
void CActivityBox::OnPlayReplaceWithSel	( wxCommandEvent& WXUNUSED(event) )
{
	MusikSongIdArray aResult;
	GetSelectedSongs(aResult);
	wxGetApp().Player.PlayReplaceList(0,aResult);
}

//-----------------------------//
//--- rename thread control ---//
//-----------------------------//
void CActivityBox::StartRenameThread( int mode, const wxArrayString &WXUNUSED(sel), wxString newvalue )
{
	
	if ( m_ActiveThreadController.IsAlive() == false )
     {
		m_ActiveThreadController.AttachAndRun( new MusikActivityRenameThread( this, mode, newvalue ) );
    }
	else
		InternalErrorMessageBox(wxT("Previous thread not terminated correctly."));
}


//----------------------------------//
//--- activity box rename thread ---//
//----------------------------------//
void CActivityBox::OnRenameThreadStart( wxCommandEvent& WXUNUSED(event) )
{
	//--- update locally ---//
	SetProgressType	( MUSIK_ACTIVITY_RENAME_THREAD );
	SetProgress		( 0 );


	//--- post the event. we're up and running now! ---//
	wxCommandEvent MusikStartProgEvt( wxEVT_COMMAND_MENU_SELECTED, MUSIK_FRAME_THREAD_START );
	wxPostEvent( g_MusikFrame, MusikStartProgEvt );
}
	
void CActivityBox::OnRenameThreadProg( wxCommandEvent& WXUNUSED(event) )
{
	//--- relay thread progress message to g_MusikFrame ---//
	g_MusikFrame->SetProgress( GetProgress() );
	wxCommandEvent MusikEndProgEvt( wxEVT_COMMAND_MENU_SELECTED, MUSIK_FRAME_THREAD_PROG );
	wxPostEvent( g_MusikFrame, MusikEndProgEvt );
}

void CActivityBox::OnRenameThreadEnd( wxCommandEvent& WXUNUSED(event) )
{
	m_ActiveThreadController.Join();// waits until threads really ends
	ReloadContents();
	EnableProgress( false );

	//--- update locally ---//
	SetProgressType	( 0 );
	SetProgress		( 0 );

    //--- relay thread end message to g_MusikFrame ---//
	wxCommandEvent MusikEndProgEvt( wxEVT_COMMAND_MENU_SELECTED, MUSIK_FRAME_THREAD_END );
	wxPostEvent( g_MusikFrame, MusikEndProgEvt );
}

