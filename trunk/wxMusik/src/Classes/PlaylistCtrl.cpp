/*
 *  PlaylistCtrl.cpp
 *
 *  A control that acts as a playlist.
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

#include "wx/file.h"

#include "PlaylistCtrl.h"
#include "Classes/ActivityAreaCtrl.h"
#include "Classes/SourcesBox.h"
//--- globals ---//
#include "MusikGlobals.h"
#include "MusikUtils.h"

//--- frames ---//
#include "Frames/MusikFrame.h"
#include "Frames/MusikTagFrame.h"
#include "Frames/MusikAutoTaggerFrame.h"
//--- threads ---//
#include "Threads/PlaylistCtrlThreads.h"

// other
#include "DataObjectCompositeEx.h"
#include "DNDHelper.h"


class PlaylistDropTarget : public wxDropTarget
{
public:
	//-------------------//
	//--- constructor ---//
	//-------------------//
	PlaylistDropTarget( CPlaylistCtrl *pPList )	
	{ 
		m_pPlaylistCtrl = pPList;	
        nLastHit = n = -1;
		wxDataObjectCompositeEx * dobj = new wxDataObjectCompositeEx;
		dobj->Add(m_pSonglistDObj = new CMusikSonglistDataObject(),true);
		dobj->Add(m_pFileDObj = new wxFileDataObject());
		SetDataObject(dobj);
	}
	virtual wxDragResult OnData(wxCoord x, wxCoord y, wxDragResult def);
	//-------------------------//
	//--- virtual functions ---//
	//-------------------------//
	virtual wxDragResult OnDropFiles(wxCoord x, wxCoord y, const wxArrayString& filenames,wxDragResult def);

	virtual wxDragResult OnDropSonglist(wxCoord x, wxCoord y, const wxString& text,wxDragResult def);
	virtual wxDragResult OnDragOver(wxCoord x, wxCoord y, wxDragResult def);

	//-------------------------//
	//--- utility functions ---//
	//-------------------------//
	bool  HighlightSel( const  wxPoint &pPos );
private:
	CMusikSonglistDataObject * m_pSonglistDObj;
	wxFileDataObject * m_pFileDObj;

	CPlaylistCtrl *m_pPlaylistCtrl;	//--- pointer to the playlist ---//
	int nLastHit;					//--- last item hit           ---//
	long n;							//--- new pos                 ---//
};

BEGIN_EVENT_TABLE(CPlaylistBox,wxPanel)
//EVT_ERASE_BACKGROUND(CPlaylistBox::OnEraseBackground)
END_EVENT_TABLE()

CPlaylistBox::CPlaylistBox( wxWindow *parent )
	: wxPanel( parent, -1, wxPoint( -1, -1 ), wxSize( -1, -1 ),wxNO_FULL_REPAINT_ON_RESIZE| wxTAB_TRAVERSAL|wxCLIP_CHILDREN|wxSUNKEN_BORDER )
	
{
#ifdef __WXMAC__
	int nBorder1 = 0;
	int nBorder2 = 0;
#else	
	int nBorder1 = 2;
	int nBorder2 = 5;
#endif	
	//--- CSourcesListBox ---//
	m_pPlaylistCtrl	= new CPlaylistCtrl( this, MUSIK_PLAYLIST, wxPoint( -1, -1 ), wxSize( -1, -1 ) );
	m_pPlaylistInfoCtrl = new CPlaylistInfoCtrl( this ,  m_pPlaylistCtrl);
	m_pSearchBox = new CSearchBox(this);
	m_pInfoSearchSizer = new wxBoxSizer( wxVERTICAL );
	m_pInfoSearchSizer->Add( m_pSearchBox, 0, wxEXPAND|wxALL,nBorder1 );
	m_pInfoSearchSizer->Add( m_pPlaylistInfoCtrl, 0, wxEXPAND );
	//--- top sizer ---//
	m_pMainSizer = new wxBoxSizer( wxVERTICAL );
	m_pMainSizer->Add( m_pInfoSearchSizer, 0, wxADJUST_MINSIZE|wxEXPAND|wxRIGHT|wxLEFT|wxBOTTOM|wxTOP  , nBorder1 );
	
	m_pMainSizer->Add(  m_pPlaylistCtrl, 1, wxEXPAND|wxRIGHT|wxLEFT|wxBOTTOM,nBorder2);
	SetSizerAndFit( m_pMainSizer );

	Layout();
}
void CPlaylistBox::ShowPlaylistInfo()
{
    if( wxGetApp().Prefs.bShowPLInfo )
       m_pPlaylistInfoCtrl->Update();
	m_pInfoSearchSizer->Show( m_pPlaylistInfoCtrl, ( bool )wxGetApp().Prefs.bShowPLInfo );
	Layout();
}
void CPlaylistBox::Update( bool bSelFirstItem )
{
#ifndef __WXMAC__ 
	SetOwnBackgroundColour(wxGetApp().Prefs.bPlaylistBorder ?  
										StringToColour(wxGetApp().Prefs.sPlaylistBorderColour)
										:wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
#endif										
	m_pPlaylistCtrl->Update(bSelFirstItem);
	if ( wxGetApp().Prefs.bShowPLInfo )
	{
		m_pPlaylistInfoCtrl->Update();
	}
	Layout();

}
void CPlaylistBox::OnEraseBackground( wxEraseEvent& event )
{	
	// empty => no background erasing to avoid flicker

	wxDC * TheDC = event.GetDC();
	wxColour BGColor =  GetBackgroundColour();
	wxBrush MyBrush(BGColor ,wxSOLID);
	TheDC->SetBackground(MyBrush);

	wxCoord x,y,w,h;
	TheDC->GetClippingBox(&x,&y,&w,&h); 

	// Now  declare the Clipping Region which is
	// what needs to be repainted
	wxRegion MyRegion(x,y,w,h); 

	//Get all the windows(controls)  rect's on the dialog
	wxWindowList & children = GetChildren();
	for ( wxWindowList::Node *node = children.GetFirst(); node; node = node->GetNext() )
	{
		wxWindow *current = (wxWindow *)node->GetData();

		// now subtract out the controls rect from the
		//clipping region
		MyRegion.Subtract(current->GetRect());
	}

	// now destroy the old clipping region
	TheDC->DestroyClippingRegion();

	//and set the new one
	TheDC->SetClippingRegion(MyRegion);
	TheDC->Clear();
}
CPlaylistBox::~CPlaylistBox()
{
	
}


BEGIN_EVENT_TABLE(CPlaylistCtrl, CMusikListCtrl)
	EVT_LIST_ITEM_ACTIVATED		( -1,														CPlaylistCtrl::OnItemActivate		)	
	EVT_LIST_BEGIN_DRAG			( -1,														CPlaylistCtrl::BeginDrag			)
	EVT_LIST_ITEM_SELECTED		( -1,														CPlaylistCtrl::UpdateSel			)
	EVT_LIST_COL_BEGIN_DRAG		( -1,														CPlaylistCtrl::BeginDragCol			)
	EVT_LIST_COL_END_DRAG		( -1,														CPlaylistCtrl::EndDragCol			)
	EVT_MENU					( MUSIK_PLAYLIST_CONTEXT_PLAY_INSTANTLY,								CPlaylistCtrl::OnPlayInstantly		)
	EVT_MENU					( MUSIK_PLAYLIST_CONTEXT_PLAY_ASNEXT,									CPlaylistCtrl::OnPlayAsNext			)
	EVT_MENU					( MUSIK_PLAYLIST_CONTEXT_PLAY_ENQUEUED,									CPlaylistCtrl::OnPlayEnqueued		)
	EVT_MENU					( MUSIK_PLAYLIST_CONTEXT_PLAY_REPLACE_PLAYERLIST,						CPlaylistCtrl::OnPlayReplace		)
	EVT_MENU					( MUSIK_PLAYLIST_CONTEXT_PLAY_REPLACE_PLAYERLIST_WITH_SELECTION,		CPlaylistCtrl::OnPlayReplaceWithSel	)
	EVT_MENU					( MUSIK_PLAYLIST_CONTEXT_OPEN_FOLDER_IN_FILEMANAGER,							CPlaylistCtrl::OnOpenFolderInFileManager	)
	EVT_MENU_RANGE				( MUSIK_PLAYLIST_CONTEXT_SHOW_IN_LIBRARY_FIRST_ENTRY,MUSIK_PLAYLIST_CONTEXT_SHOW_IN_LIBRARY_LAST_ENTRY, CPlaylistCtrl::OnShowInLibrary)
	EVT_MENU					( MUSIK_PLAYLIST_DELETE_CONTEXT_DELETE_FROM_PLAYLIST,					CPlaylistCtrl::OnDelSel				)
	EVT_MENU					( MUSIK_PLAYLIST_DELETE_CONTEXT_DELETE_FILES,							CPlaylistCtrl::OnDelFiles			)
	EVT_MENU					( MUSIK_PLAYLIST_DELETE_CONTEXT_DELETE_FROM_DB,							CPlaylistCtrl::OnDelFilesDB			)
	EVT_UPDATE_UI_RANGE			( MUSIK_PLAYLIST_DELETE_CONTEXT_DELETE_FROM_PLAYLIST, MUSIK_PLAYLIST_DELETE_CONTEXT_DELETE_FROM_DB,	CPlaylistCtrl::OnUpdateUIDelete	)
	EVT_MENU					( MUSIK_PLAYLIST_CONTEXT_RENAME_FILES,									CPlaylistCtrl::OnRenameFiles		)
	EVT_MENU					( MUSIK_PLAYLIST_CONTEXT_RETAG_FILES,									CPlaylistCtrl::OnRetagFiles			)
	EVT_MENU					( MUSIK_PLAYLIST_CONTEXT_REBUILDTAG,									CPlaylistCtrl::OnRebuildTag			)

	EVT_MENU_RANGE				( MUSIK_PLAYLIST_CONTEXT_RATING, MUSIK_PLAYLIST_CONTEXT_RATING + (MUSIK_MAX_RATING - MUSIK_MIN_RATING) + 1,			CPlaylistCtrl::OnRateSel			) 	
	EVT_UPDATE_UI_RANGE			( MUSIK_PLAYLIST_CONTEXT_RATING, MUSIK_PLAYLIST_CONTEXT_RATING + (MUSIK_MAX_RATING - MUSIK_MIN_RATING) + 1,			CPlaylistCtrl::OnUpdateUIRateSel	)
	EVT_MENU_RANGE				( MUSIK_PLAYLIST_CONTEXT_TAG_TITLE,	MUSIK_PLAYLIST_CONTEXT_TAG_NOTES,	CPlaylistCtrl::OnClickEditTag		)
	EVT_MENU					( MUSIK_PLAYLIST_DISPLAY_SMART,											CPlaylistCtrl::OnDisplaySmart		)
	EVT_UPDATE_UI				( MUSIK_PLAYLIST_DISPLAY_SMART,											CPlaylistCtrl::OnUpdateUIDisplaySmart)
	EVT_MENU					( MUSIK_PLAYLIST_DISPLAY_FIT,											CPlaylistCtrl::OnDisplayFit			)
	EVT_MENU					( MUSIK_PLAYLIST_CLEARPLAYERLIST,										CPlaylistCtrl::OnClearPlayerlist	)	
	EVT_KEY_DOWN                (																		CPlaylistCtrl::OnKeyDown			)// we use up event because using down leads to problems with deleting songs    
	EVT_LIST_COL_CLICK			( -1,																	CPlaylistCtrl::OnColumnClick		)

	//---------------------------------------------------------//
	//--- column on off stuff.								---//
	//-------------  		--------------------------------------------//
	EVT_MENU_RANGE				( MUSIK_PLAYLIST_DISPLAY_FIRST, MUSIK_PLAYLIST_DISPLAY_LAST,	CPlaylistCtrl::OnDisplayMenu				)
	EVT_UPDATE_UI_RANGE		( MUSIK_PLAYLIST_DISPLAY_FIRST, MUSIK_PLAYLIST_DISPLAY_LAST,		CPlaylistCtrl::OnUpdateUIDisplayMenu		)
	//---------------------------------------------------------//
	//--- threading events.. we use EVT_MENU becuase its	---//
	//--- nice and simple, and gets the job done. this may	---//
	//--- become a little prettier later, but it works.		---//
	//---------------------------------------------------------//
    EVT_MENU			( MUSIK_PLAYLIST_THREAD_START,	CPlaylistCtrl::OnThreadStart)
	EVT_MENU			( MUSIK_PLAYLIST_THREAD_END,	CPlaylistCtrl::OnThreadEnd	)
	EVT_MENU			( MUSIK_PLAYLIST_THREAD_PROG,	CPlaylistCtrl::OnThreadProg	)

END_EVENT_TABLE()

//-----------------//
//--- dnd stuff ---//
//-----------------//

wxDragResult PlaylistDropTarget::OnData(wxCoord x, wxCoord y, wxDragResult def)
{
	
	if (GetData() )
	{
			
		wxDataObjectSimple *dobj = ((wxDataObjectCompositeEx *)GetDataObject())->GetActualDataObject();
		
		if( dobj == (wxDataObjectSimple *)m_pSonglistDObj )
			def  = OnDropSonglist(x, y, m_pSonglistDObj->GetText(), def );
		else if( dobj == (wxDataObjectSimple *)m_pFileDObj )
			def  = OnDropFiles(x, y, m_pFileDObj->GetFilenames(), def);
	}
	return def;
}

wxDragResult PlaylistDropTarget::OnDropFiles(wxCoord x, wxCoord y, const wxArrayString& filenames,wxDragResult def)
{
	return m_pPlaylistCtrl->OnDropFiles( x,  y,  filenames ,def);
}
wxDragResult PlaylistDropTarget::OnDropSonglist( wxCoord x, wxCoord y, const wxString &sFiles,wxDragResult def )
{
	//--- make sure we have something ---//
	if ( !sFiles.IsEmpty())
	{

		//--- where did we land? ---//
		const wxPoint& pt = wxPoint( x, y );
		int nFlags;
		n = m_pPlaylistCtrl->HitTest( pt, nFlags );


		//--- make sure we havn't dragged on a selected item and aCurSel is not empty---//
		if ( m_pPlaylistCtrl->DNDIsSel( n ) || m_pPlaylistCtrl->aCurSel.GetCount() == 0)
		{
			m_pPlaylistCtrl->aCurSel.Clear();
			return wxDragNone;
		}

		//--- first lets make sure user intended to drop files in the playlist ---//
		//--- otherwise we may have songs removed that shouldn't be. that would be bad ---//
		int nLastSelItem = m_pPlaylistCtrl->aCurSel.Last();
		if ( n == -1 || n < nLastSelItem || n > nLastSelItem )
		{
			//--- not dragged over anything, push to bottom ---//
			if ( n == -1 )
			{
				n = m_pPlaylistCtrl->GetItemCount() - 1;
			}
			//--- find where we need to drop ---//
			if ( n == -2 )
			{	//--- error, couldn't locate our position ---//
				InternalErrorMessageBox(wxT("Could not find previous item's position."));
				return wxDragNone;
			}
			if(n >= nLastSelItem)
				n++;
			m_pPlaylistCtrl->MovePlaylistEntrys( n ,m_pPlaylistCtrl->aCurSel );
			
		}
	}
	m_pPlaylistCtrl->DNDDone();
	return def;
}

wxDragResult PlaylistDropTarget::OnDragOver(wxCoord x, wxCoord y, wxDragResult def)
{
	if(def == wxDragNone)
		return wxDragNone;

	//--- calls HighlightSel() to highlight the item the mouse is over ---//
	//m_pPlaylistCtrl->SetFocus();
	const wxPoint& pt = wxPoint( x, y );
	if(m_pPlaylistCtrl->m_bInternalDragInProcess)
	{	 // check if dnd makes sense
		if((g_SourcesCtrl->GetSelType() == MUSIK_SOURCES_LIBRARY) 
			|| (g_SourcesCtrl->GetSelType() == MUSIK_SOURCES_NETSTREAM)
			|| (g_SourcesCtrl->GetSelType() == MUSIK_SOURCES_PLAYLIST_DYNAMIC)
			)
			return wxDragNone;
	}
	return HighlightSel( pt ) ? wxDragCopy : wxDragNone;// return wxDragCopy because def is not set correctly by wxwidgets(it should be the operation which is proposed by windows, but wxwidgets uses the keystate for determining the drop effect)
	// wxDragCopy because CPlaylistCtrl uses wxDrag_CopyOnly( to prohibit moving files by explorer when files are dropped there.)
}

bool PlaylistDropTarget::HighlightSel( const  wxPoint & pPos )
{
	//--- gotta set this so stuff doesn't keep updating ---//
	int nFlags;
	long n = m_pPlaylistCtrl->HitTest( pPos, nFlags );
	m_pPlaylistCtrl->SetFocus();
	long topitem = m_pPlaylistCtrl->GetTopItem();
	long countperpage = m_pPlaylistCtrl->GetCountPerPage();
	if( n == topitem && n > 0)
		m_pPlaylistCtrl->EnsureVisible(n - 1);
	else if((n == topitem + countperpage - 1) &&  (n < m_pPlaylistCtrl->GetItemCount() - 1))
		m_pPlaylistCtrl->EnsureVisible(n + 1);
	//--- highlight what we're over, deselect old ---//
	if ( n != nLastHit && n!= -1 )
	{
		if ( nLastHit != -1 )
			m_pPlaylistCtrl->SetItemState( nLastHit, 0, wxLIST_STATE_FOCUSED );

		m_pPlaylistCtrl->SetItemState( n, wxLIST_STATE_FOCUSED, wxLIST_STATE_FOCUSED );
	}

	//--- this is a quick way to check if we need to update ---//
	nLastHit = n;
	return true;
}

//----------------------------//
//--- construct / destruct ---//
//----------------------------//
CPlaylistCtrl::CPlaylistCtrl( CPlaylistBox *parent, const wxWindowID id, const wxPoint& pos, const wxSize& size )
	:	CMusikListCtrl		( parent, id, pos, size,wxNO_BORDER)
	,m_bInternalDragInProcess(false)
	,m_pParent(parent)
    ,m_pPlaylist(NULL)	
{
	//--- setup headers ---//
	m_ColSaveNeeded = false;
	ResetColumns();


	//--- setup drop target ---//
	SetDropTarget( new PlaylistDropTarget( this ) );

	//--- not dragging, no selections ---//
	m_nCurSel = -1;
	m_Overflow = 0;
	m_bColDragging = false;

}

CPlaylistCtrl::~CPlaylistCtrl()
{
}

///////////////////////////////////////////////////////////////////////////////
wxMenu * CPlaylistCtrl::CreateContextMenu()
{

	//--- rating menu ---//
	wxMenu *playlist_context_rating_menu = new wxMenu;
	for(int i = MUSIK_MIN_RATING; i <= MUSIK_MAX_RATING ;i++)
	{
		playlist_context_rating_menu->Append(  MUSIK_PLAYLIST_CONTEXT_RATING  + (i - MUSIK_MIN_RATING), (i == 0) ? wxString(_( "Unrated" )) : wxString() << i, wxT( "" ), wxITEM_CHECK );
	}
	//--- tag edit menu ---//
	wxMenu *playlist_context_edit_tag_menu = new wxMenu;
	playlist_context_edit_tag_menu->Append( MUSIK_PLAYLIST_CONTEXT_TAG_TITLE,		_( "Edit Title\tF2" ) );
	playlist_context_edit_tag_menu->Append( MUSIK_PLAYLIST_CONTEXT_TAG_TRACKNUM,	_( "Edit Track Number\tF3" ) );
	playlist_context_edit_tag_menu->Append( MUSIK_PLAYLIST_CONTEXT_TAG_ARTIST,		_( "Edit Artist\tF4" ) );
	playlist_context_edit_tag_menu->Append( MUSIK_PLAYLIST_CONTEXT_TAG_ALBUM,		_( "Edit Album\tF5" ) );
	playlist_context_edit_tag_menu->Append( MUSIK_PLAYLIST_CONTEXT_TAG_GENRE,		_( "Edit Genre\tF6" ) );
	playlist_context_edit_tag_menu->Append( MUSIK_PLAYLIST_CONTEXT_TAG_YEAR,		_( "Edit Year\tF7" ) );
	playlist_context_edit_tag_menu->Append( MUSIK_PLAYLIST_CONTEXT_TAG_NOTES,		_( "Edit Notes\tF8" ) );
	playlist_context_edit_tag_menu->AppendSeparator();
	playlist_context_edit_tag_menu->Append( MUSIK_PLAYLIST_CONTEXT_RENAME_FILES,	wxString(_( "&Auto Rename" )) + wxT("\tCTRL+SHIFT+R"));
	playlist_context_edit_tag_menu->Append( MUSIK_PLAYLIST_CONTEXT_RETAG_FILES,		wxString(_( "A&uto Retag...")) + wxT("\tCTRL+T") );
	playlist_context_edit_tag_menu->Append( MUSIK_PLAYLIST_CONTEXT_REBUILDTAG,		wxString(_( "&Rebuild Tag"))+ wxT("\tCTRL+R") );

	//--- delete menu ---//
	wxMenu *playlist_context_delete_menu = new wxMenu;
	playlist_context_delete_menu->Append( MUSIK_PLAYLIST_DELETE_CONTEXT_DELETE_FROM_PLAYLIST,	_( "From Playlist\tDel" ) );
	playlist_context_delete_menu->Append( MUSIK_PLAYLIST_DELETE_CONTEXT_DELETE_FROM_DB,			_( "From Database\tAlt+Del" ) );
	playlist_context_delete_menu->Append( MUSIK_PLAYLIST_DELETE_CONTEXT_DELETE_FILES,			_( "From Computer\tCtrl+Del" ) );

	//--- columns context menu ---//
	wxMenu *playlist_context_display_menu = new wxMenu;
	for(size_t i = 0 ; i < PlaylistColumn::NCOLUMNS;i ++)
	{
		playlist_context_display_menu->AppendCheckItem( MUSIK_PLAYLIST_DISPLAY_FIRST + i,wxGetTranslation(g_PlaylistColumn[i].Label) );
	}
	playlist_context_display_menu->AppendSeparator();
	playlist_context_display_menu->AppendCheckItem( MUSIK_PLAYLIST_DISPLAY_FIT,				_( "Fit Columns" ) );
	playlist_context_display_menu->AppendCheckItem( MUSIK_PLAYLIST_DISPLAY_SMART,			_( "No Horizontal Scroll" ) );

	bool bNetStreamSel = false;
	if ( GetSelectedItemCount() > 0 )
	{
		int nFirstIndex = GetNextItem( -1, wxLIST_NEXT_ALL , wxLIST_STATE_SELECTED );
		bNetStreamSel  =  m_pPlaylist && m_pPlaylist->Item ( nFirstIndex ).IsFormat(MUSIK_FORMAT_NETSTREAM);
	}
	bool bIsNowPlayingSelected = (g_SourcesCtrl->GetSelType() == MUSIK_SOURCES_NOW_PLAYING);

	//--- main context menu ---//
	wxMenu *playlist_context_menu = new wxMenu;
	if(!bIsNowPlayingSelected)
	{
		//Play menu
		wxMenu * playlist_context_play_menu = new wxMenu;
		playlist_context_play_menu->Append( MUSIK_PLAYLIST_CONTEXT_PLAY_ASNEXT , _( "Next" ), wxT( "" ) );
		playlist_context_play_menu->Append( MUSIK_PLAYLIST_CONTEXT_PLAY_ENQUEUED, _( "Enqueue" ), wxT( "" ) );
		playlist_context_play_menu->Append( MUSIK_PLAYLIST_CONTEXT_PLAY_INSTANTLY , _( "Instantly" ), wxT( "" ));
		playlist_context_play_menu->Append( MUSIK_PLAYLIST_CONTEXT_PLAY_REPLACE_PLAYERLIST_WITH_SELECTION, _( "Replace current playlist with selection" ), wxT( "" ) );
		playlist_context_play_menu->Append( MUSIK_PLAYLIST_CONTEXT_PLAY_REPLACE_PLAYERLIST, _( "Replace current playlist with all" ), wxT( "" ) );

		playlist_context_menu->Append( MUSIK_PLAYLIST_CONTEXT_PLAYNODE,			_( "&Play" ),					playlist_context_play_menu );
	}
	else
		playlist_context_menu->Append( MUSIK_PLAYLIST_CLEARPLAYERLIST,			_( "&Clear List" ),					wxT("") );

	wxMenu *  playlist_context_show_in_library_menu = new wxMenu;
    size_t cnt = g_ActivityAreaCtrl->GetActivityBoxCount();
    for(size_t i = 0; i < cnt; i ++)
    {
        playlist_context_show_in_library_menu->Append( MUSIK_PLAYLIST_CONTEXT_SHOW_IN_LIBRARY_FIRST_ENTRY + i,
                                                        _( "This ") + g_ActivityAreaCtrl->GetActivityBox(i)->DisplayName()
                                                        ,	wxT("") );
    }
	playlist_context_menu->Append( MUSIK_PLAYLIST_CONTEXT_SHOW_IN_LIBRARY_NODE,_( "&Show in Library" ),	playlist_context_show_in_library_menu );
	playlist_context_menu->Append( MUSIK_PLAYLIST_CONTEXT_OPEN_FOLDER_IN_FILEMANAGER,_( "&Open Folder in File Manager" ),wxT(""));
	

	playlist_context_menu->Append( MUSIK_PLAYLIST_CONTEXT_RATENODE,	_( "&Rating" ),	playlist_context_rating_menu );
	playlist_context_menu->Append( MUSIK_PLAYLIST_CONTEXT_DISPLAYNODE,	_( "Display" ),	playlist_context_display_menu );
	playlist_context_menu->AppendSeparator();
	playlist_context_menu->Append( MUSIK_PLAYLIST_CONTEXT_DELETENODE,	_( "Delete" ),	playlist_context_delete_menu );
	playlist_context_menu->AppendSeparator();
	playlist_context_menu->Append( MUSIK_PLAYLIST_CONTEXT_TAGNODE,	_( "Edit &Tag" ),playlist_context_edit_tag_menu );

	//if(!bIsNowPlayingSelected)
	//	playlist_context_menu->Enable( MUSIK_PLAYLIST_CONTEXT_PLAYNODE,		!bNetStreamSel );
	if(!bIsNowPlayingSelected)	
		playlist_context_menu->Enable( MUSIK_PLAYLIST_CONTEXT_DELETENODE,	!bNetStreamSel );
	playlist_context_menu->Enable( MUSIK_PLAYLIST_CONTEXT_RENAME_FILES, !bNetStreamSel );
	playlist_context_menu->Enable( MUSIK_PLAYLIST_CONTEXT_RETAG_FILES,	!bNetStreamSel );
	playlist_context_menu->Enable( MUSIK_PLAYLIST_CONTEXT_REBUILDTAG,	!bNetStreamSel );
	playlist_context_menu->Enable( MUSIK_PLAYLIST_CONTEXT_RATENODE,		!bNetStreamSel );
	playlist_context_menu->Enable( MUSIK_PLAYLIST_CONTEXT_TAGNODE,		!bNetStreamSel );
	playlist_context_menu->Enable( MUSIK_PLAYLIST_CONTEXT_SHOW_IN_LIBRARY_NODE,	!bNetStreamSel );
	playlist_context_menu->Enable( MUSIK_PLAYLIST_CONTEXT_OPEN_FOLDER_IN_FILEMANAGER,	!bNetStreamSel );

	return playlist_context_menu;
}
void CPlaylistCtrl::OnColumnClick( wxListEvent& event )
{
	if(m_pPlaylist == NULL)
		return;
	if(g_SourcesCtrl->GetSelType() != MUSIK_SOURCES_LIBRARY) // only sort in library view, else we will destroy the user playlist
		return;
	int ActualColumn = m_ColumnOrder.Item( event.GetColumn() );
	
	int currentsortorder = m_aColumnSorting.Item( ActualColumn );
	for(size_t i = 0; i < m_aColumnSorting.GetCount();i++)
    	m_aColumnSorting.Item( i ) = 0;	   // set all columns to unsorted

	m_aColumnSorting.Item( ActualColumn ) = currentsortorder > 0 ? -1 : 1;   // toggle sort order

	bool desc = ( m_aColumnSorting.Item( ActualColumn ) < 0 );

	wxGetApp().Library.SetSortOrderColumn( g_PlaylistColumn[ActualColumn], desc );
	wxGetApp().Library.RedoLastQuerySongsWhere( *m_pPlaylist ,true);//sorted
	Update();
}

///////////////////////////////////////////////////////////////////////////////

void CPlaylistCtrl::SaveColumns()
{
	//---------------------------------------------------------//
	//--- get the total width of all the columns in pixels.	---//
	//--- this value will be used to calculate dynamic		---//
	//--- columns.											---//
	//---------------------------------------------------------//
	m_ColSaveNeeded = false;
	wxSize client_size = GetClientSize();

	size_t nCurrCol;
	float f_Pos;
	int n_Pos;

	//---------------------------------------------------------//
	//--- sum up all the columns with a static size. the	---//
	//--- remaining size will be used to calculate dynamic	---//
	//--- column percentages.								---//
	//---------------------------------------------------------//
	size_t nStaticWidth = 0;
	for ( size_t i = 0; i < m_ColumnOrder.GetCount(); i++ )
	{
		nCurrCol = m_ColumnOrder.Item( i );

		if ( wxGetApp().Prefs.bPlaylistColumnDynamic[nCurrCol] == 0 )
		{
			wxGetApp().Prefs.PlaylistColumnSize[nCurrCol] = GetColumnWidth( i );
			nStaticWidth += GetColumnWidth( i );
		}
	}

	//---------------------------------------------------------//
	//--- calculate the dynamic size columns. these are		---//
	//--- these are the percent of the window the current	---//
	//--- column takes up.									---//
	//---------------------------------------------------------//
	int nRemaining = client_size.GetWidth() - nStaticWidth;
    if( nRemaining < 0 || (nRemaining / m_ColumnOrder.GetCount() < 50))
       nRemaining = 50*m_ColumnOrder.GetCount();
	for ( size_t i = 0; i < m_ColumnOrder.GetCount(); i++ )
	{
		nCurrCol = m_ColumnOrder.Item( i );

		if ( wxGetApp().Prefs.bPlaylistColumnDynamic[nCurrCol] == 1 )
		{
			f_Pos = (float)GetColumnWidth( i ) / (float)nRemaining * 100.0f;
			n_Pos = MusikRound( f_Pos );

			if ( n_Pos < 1 )
				n_Pos = 1;

			wxGetApp().Prefs.PlaylistColumnSize[nCurrCol] = n_Pos;
		}
	}
}

//--------------//
//--- events ---//
//--------------//

int CPlaylistCtrl::DisplayEventId2ColumnId( int evid)
{
	return  evid - MUSIK_PLAYLIST_DISPLAY_FIRST;

}
void CPlaylistCtrl::OnRateSel( wxCommandEvent& event )
{
	RateSel( (event.GetId() - MUSIK_PLAYLIST_CONTEXT_RATING) + MUSIK_MIN_RATING );
}
void CPlaylistCtrl::OnClickEditTag( wxCommandEvent& event )
{
	EditTag( event.GetId() - MUSIK_PLAYLIST_CONTEXT_TAG_TITLE );
}
void CPlaylistCtrl::OnDisplayMenu( wxCommandEvent& event )
{
	int nColumn = -1;
	nColumn = DisplayEventId2ColumnId(event.GetId());
	if( nColumn > -1)
	{
		wxGetApp().Prefs.bPlaylistColumnEnable[nColumn] = !wxGetApp().Prefs.bPlaylistColumnEnable[nColumn];
		ResetColumns( false, true );
	}
}
void CPlaylistCtrl::OnDisplayFit( wxCommandEvent& WXUNUSED(event) )
{
	RescaleColumns( true, false, true );
}
void CPlaylistCtrl::OnClearPlayerlist( wxCommandEvent& WXUNUSED(event) )
{

	wxGetApp().Player.ClearPlaylist();
	Update(false);
}
void CPlaylistCtrl::OnUpdateUIDisplayMenu ( wxUpdateUIEvent &event)
{
	int nColumn = DisplayEventId2ColumnId(event.GetId());
	if(nColumn > -1)
		event.Check(wxGetApp().Prefs.bPlaylistColumnEnable[nColumn]);	
}

void CPlaylistCtrl::OnUpdateUIDelete ( wxUpdateUIEvent &event)
{
	bool bEnable = true;
			
	int nFirstIndex = GetNextItem( -1, wxLIST_NEXT_ALL , wxLIST_STATE_SELECTED );
	if(nFirstIndex >= 0 && m_pPlaylist && m_pPlaylist->Item ( nFirstIndex ).IsFormat(MUSIK_FORMAT_NETSTREAM))
	{// first selected item is a netstream
		if((event.GetId() == MUSIK_PLAYLIST_DELETE_CONTEXT_DELETE_FROM_PLAYLIST) 
			&&(g_SourcesCtrl->GetSelType() == MUSIK_SOURCES_NOW_PLAYING))
			bEnable = true;//enable "delete from playlist" for netsreams only if "now playing" is active.
		else
			bEnable = false;

	}
	event.Enable(bEnable);
}

void CPlaylistCtrl::OnUpdateUIRateSel ( wxUpdateUIEvent &event)
{
	//--- get rating for first sel ---//
	int item = GetNextItem( -1, wxLIST_NEXT_ALL , wxLIST_STATE_SELECTED );
	
	if(item > -1 && m_pPlaylist)
	{
		int nRating = m_pPlaylist->Item( item ).Song()->Rating;
		wxASSERT(nRating >= MUSIK_MIN_RATING && nRating <= MUSIK_MAX_RATING);
		event.Check( event.GetId() == (MUSIK_PLAYLIST_CONTEXT_RATING + (nRating  - MUSIK_MIN_RATING)));
	}
}

void CPlaylistCtrl::OnDisplaySmart( wxCommandEvent& WXUNUSED(event) )
{
	wxGetApp().Prefs.bPlaylistSmartColumns = !wxGetApp().Prefs.bPlaylistSmartColumns;
	RescaleColumns();
}

void CPlaylistCtrl::OnUpdateUIDisplaySmart ( wxUpdateUIEvent &event)
{
	event.Check( wxGetApp().Prefs.bPlaylistSmartColumns );
}
void CPlaylistCtrl::BeginDrag( wxListEvent& WXUNUSED(event) )
{
	//--- pass selected items ---//
	wxString sValidSelFiles = GetSelSongIds();
	if(!sValidSelFiles.IsEmpty() && m_pPlaylist)
	{
		wxString sDrop (sValidSelFiles);
		DNDSetCurSel();
		  
		//--- initialize drag and drop... SourcesDropTarget / PlaylistDropTarget should take care of the rest ---//
		wxDropSource dragSource( this );
		CMusikSonglistDataObject *psong_data = new CMusikSonglistDataObject(sDrop);
		wxFileDataObject *pfile_data = new wxFileDataObject;
		for (size_t i = 0; i < aCurSel.GetCount();i++)
		{
			pfile_data->AddFile(m_pPlaylist->Item(aCurSel[i]).Song()->MetaData.Filename.GetFullPath());
		}
		wxDataObjectComposite dobj;
		dobj.Add(psong_data,true);
		dobj.Add(pfile_data);

		dragSource.SetData( dobj );
		m_bInternalDragInProcess = true;
		dragSource.DoDragDrop( wxDrag_CopyOnly );
		m_bInternalDragInProcess = false;
	}
}
void CPlaylistCtrl::BeginDragCol(wxListEvent& WXUNUSED(event))
{
	m_bColDragging = true;
}
void CPlaylistCtrl::EndDragCol( wxListEvent& WXUNUSED(event) )
{
	m_bColDragging = false;
	m_ColSaveNeeded = true;
}


void CPlaylistCtrl::OnItemActivate( wxListEvent& (event) )
{
	if(g_SourcesCtrl->GetSelType() == MUSIK_SOURCES_NOW_PLAYING)
		wxGetApp().Player.PlayByUser(event.GetIndex());
	else if (wxGetApp().Prefs.bDoubleClickReplacesPlaylist)
	{
		wxCommandEvent dummy;
		OnPlayReplace(dummy);
	}
	else
	{
		OnPlayInstantly(event);
	}
}

void CPlaylistCtrl::OnKeyDown( wxKeyEvent& event )
{	 
	int nKeyCode = event.GetKeyCode();
	if ( event.AltDown())
	{
		switch( nKeyCode )
		{
			case WXK_DELETE:
			case WXK_BACK:
				DelSelSongs(true);// delete songs from db
				return;
			default:
				event.Skip();
				return;
		}
	}
	else if ( event.CmdDown())
	{
        if( GetSelectedItemCount() > 0 )
        {
            wxCommandEvent dummy;
            switch( nKeyCode )
            {
            case 'R':
                if(event.ShiftDown())
                    OnRenameFiles(dummy);
                else
                    OnRebuildTag(dummy);
                return;
            case 'T':
                OnRetagFiles(dummy);
                return;
            case 'D':	//--- d / D ---//
                SelectNone( );
                return;
            case WXK_DELETE:
            case WXK_BACK:
                DelSelSongs(true,true);// delete from db and computer
                return;
            default:
                break;
            }
        }
        switch( nKeyCode )
        {
        case 'A':	//--- a / A ---//
            SelectAll( );
            return;
        default:
            event.Skip();
            return;
		}
	}
	else 
	{
		if( GetSelectedItemCount() > 0 )
		{
 			switch( nKeyCode )
			{
				case WXK_F2:
				case WXK_F3:
				case WXK_F4:
				case WXK_F5:
				case WXK_F6:
				case WXK_F7:
				case WXK_F8:
					EditTag( nKeyCode -  WXK_F2);
					return;
				case WXK_DELETE:
				case WXK_BACK:
					DelSelSongs();
					return;
				case '0':
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
				case '8':
				case '9':
					if(event.ShiftDown())
						RateSel(-(nKeyCode - '0'));
					else
						RateSel(nKeyCode - '0');
					return;
				case 'I':
					{
						wxCommandEvent dummy;
						OnPlayInstantly( dummy );
					}
					return;
				case 'E':
					{
						wxCommandEvent dummy;
						OnPlayEnqueued( dummy );
					}
					return;
				case 'N':
					{
						wxCommandEvent dummy;
						OnPlayAsNext( dummy );
					}
					return;
			}
		}
        switch( nKeyCode )
        {
        case 'C':
            {
                wxCommandEvent dummy;
                OnClearPlayerlist( dummy );
            }
            return;
       default:
            event.Skip();
            return;
        }
        return;
	}
}

//---------------------------------------------------//
//--- virtual functions. careful with these ones, ---// 
//---        boys. they drive everything          ---//
//---------------------------------------------------//
wxString CPlaylistCtrl::OnGetItemText(long item, long column) const
{
	//-----------------------------------------------------//
	//--- FindColumnOrder() should already have fired	---//
	//--- so go ahead and add the items.				---//
	//-----------------------------------------------------//

    PlaylistColumn::eId eCurrType = (PlaylistColumn::eId)m_ColumnOrder.Item( column );
	return GetItemText(item,eCurrType);

}

wxString CPlaylistCtrl::EmptyColumnString()	const
{
	return wxGetApp().Prefs.bDisplayEmptyPlaylistColumnAsUnkown == true ? _( "<unknown>" ):wxEmptyString;
}

wxString CPlaylistCtrl::GetItemText(long item, PlaylistColumn::eId eColumnId) const
{
	if(m_pPlaylist && item >= (long)m_pPlaylist->GetCount())
		return wxT( "" );
    const std::auto_ptr<CMusikSong> pSong = m_pPlaylist->Item ( item ).Song();
	switch ( eColumnId )
	{
	case PlaylistColumn::RATING:
		break;

	case PlaylistColumn::TRACK:
		if ( pSong->MetaData.nTracknum > 0 )
			return wxString::Format( wxT( "%d" ), pSong->MetaData.nTracknum );
		else
			return wxT( "-" );
		break;

	case PlaylistColumn::TITLE:
		return SanitizedString(ConvFromUTF8( pSong->MetaData.Title ));		
		break;

	case PlaylistColumn::ARTIST:
		if ( pSong->MetaData.Artist.IsEmpty() )
			return EmptyColumnString();
			
		else 
		{
			return SanitizedString(ConvFromUTF8( pSong->MetaData.Artist ));
		}
		break;

	case PlaylistColumn::ALBUM:
		if ( pSong->MetaData.Album.IsEmpty() )
			return  EmptyColumnString();
		else
			return SanitizedString(ConvFromUTF8( pSong->MetaData.Album ));
		break;

	case PlaylistColumn::YEAR:
		if ( pSong->MetaData.Year.IsEmpty() )
			return  EmptyColumnString();
		else
			return ConvFromUTF8(pSong->MetaData.Year);
		break;

	case PlaylistColumn::GENRE:
		if ( pSong->MetaData.Genre.IsEmpty() )
			return  EmptyColumnString();
		else
			return SanitizedString(ConvFromUTF8( pSong->MetaData.Genre ));
		break;

	case PlaylistColumn::TIMES_PLAYED:
		{

			return (pSong->TimesPlayed > 0) ? wxString() << pSong->TimesPlayed : wxString(wxT("-"));
		}
		break;

	case PlaylistColumn::LAST_PLAYED:
		if ( pSong->LastPlayed != 0.0 )
		{
			return JDN2LocalTimeString( pSong->LastPlayed );
		}
		else
			return _("Never");
		break;

	case PlaylistColumn::TIME:
		return MStoStr( pSong->MetaData.nDuration_ms );
		break;

	case PlaylistColumn::BITRATE:
		return IntTowxString( pSong->MetaData.nBitrate );
		break;

	case PlaylistColumn::FILENAME:
		{
			return pSong->MetaData.Filename.GetFullPath();
		}
		break;
	case PlaylistColumn::NOTES:
		return ConvFromUTF8(pSong->MetaData.Notes);
	case PlaylistColumn::TIMEADDED:
		{
		    return JDN2LocalTimeString( pSong->TimeAdded );
		}
		break;
	
	default:
		wxASSERT(false);
		break;
	}

	return wxT( "" );

}
void CPlaylistCtrl::FindColumnOrder()
{
	m_ColumnOrder.Clear();
	m_aColumnSorting.Clear();
	for ( int i = 0; i < PlaylistColumn::NCOLUMNS; i++ )
	{
		if ( wxGetApp().Prefs.bPlaylistColumnEnable[i] == 1 )
		{
			m_ColumnOrder.Add( i );
		}
		m_aColumnSorting.Add( 0 );
	}
}

int CPlaylistCtrl::OnGetItemImage(long item) const
{
	if(m_pPlaylist && item >= (long)m_pPlaylist->GetCount())
		return 0;  // this actually did happen on macos.dont know why.
	return m_pPlaylist->Item ( item ).Song()->Rating - MUSIK_MIN_RATING;
}

wxListItemAttr* CPlaylistCtrl::OnGetItemAttr(long item) const
{
#ifdef __WXMAC__
	wxListItemAttr *pDefAttr = 	NULL;
#else
	wxListItemAttr *pDefAttr = 	(wxListItemAttr *)&m_LightAttr;
#endif
	if(m_pPlaylist && item >= (long)m_pPlaylist->GetCount())
		return pDefAttr;
    const std::auto_ptr<CMusikSong> pSong = m_pPlaylist->Item ( item ).Song();
	
	if(wxGetApp().Player.IsPlaying() && (g_SourcesCtrl->GetSelType() == MUSIK_SOURCES_NOW_PLAYING) 
		&& (wxGetApp().Player.GetCurIndex() == (size_t)item ) && (pSong->songid == wxGetApp().Player.GetCurrentSongid()))
	{
		if ( wxGetApp().Prefs.bPLStripes == 1 )
			return item % 2 ? (wxListItemAttr *)&m_SelectedDarkAttr : (wxListItemAttr *)&m_SelectedLightAttr;
		else
			return (wxListItemAttr *)&m_SelectedLightAttr;
	}
	else if ( wxGetApp().Player.IsPlaying() && (g_SourcesCtrl->GetSelType() != MUSIK_SOURCES_NOW_PLAYING) 
		&& pSong->songid == wxGetApp().Player.GetCurrentSongid() && pSong->MetaData.Filename.GetFullPath() == wxGetApp().Player.GetCurrentFilename() )
	{
		if ( wxGetApp().Prefs.bPLStripes == 1 )
			return item % 2 ? (wxListItemAttr *)&m_SelectedDarkAttr : (wxListItemAttr *)&m_SelectedLightAttr;
		else
			return (wxListItemAttr *)&m_SelectedLightAttr;
	}
	else
	{
		if ( wxGetApp().Prefs.bPLStripes == 1 )
			return item % 2 ? (wxListItemAttr *)&m_DarkAttr : pDefAttr;
		else
			return pDefAttr;
	}
}

//--------------------------------------------------//
//--- functions to get information from the list ---//
//--------------------------------------------------//

wxString CPlaylistCtrl::GetAllFiles()
{
	wxString sResult;
	sResult.Alloc( GetItemCount() * 50 ); // optimization ( the 50 is a wild guess)
	for ( int i = 0; i < GetItemCount(); i++ )
	{
		sResult +=  GetFilename( i );
		if( i <  GetItemCount() - 1 )
			sResult += '\n';
	}
	return sResult;
}

wxString CPlaylistCtrl::GetSelSongIds()
{
	wxString sResult;
	int nIndex = -1;
	int itemcount = m_pPlaylist ? GetSelectedItemCount():0;
	sResult.Alloc(itemcount * 50); // 50 is just a for the average file name length
	for ( int i = 0; i < itemcount; i++ )
	{
		nIndex = GetNextItem( nIndex, wxLIST_NEXT_ALL , wxLIST_STATE_SELECTED );
		if( nIndex == -1)
			break;
		if(m_pPlaylist->Item( nIndex ).IsFormat(MUSIK_FORMAT_NETSTREAM) == false)
		{
			sResult += wxString() << (int)m_pPlaylist->Item( nIndex );
			sResult+= wxT( "\n" ); // only add \n if it is not the last name
		}
	}

	//--- we don't need the last \n ---//
	return sResult.IsEmpty() ? sResult : sResult.Truncate( sResult.Length() - 1 );	
}

void  CPlaylistCtrl::GetSelItems(wxArrayInt & aResult)
{
	aResult.Clear();
	int nIndex = -1;
	for ( int i = 0; i < GetSelectedItemCount(); i++ )
	{
		nIndex = GetNextItem( nIndex, wxLIST_NEXT_ALL , wxLIST_STATE_SELECTED );
		if ( nIndex == -1 )
			break;
		aResult.Add( nIndex );
	}
	return;	
}

void CPlaylistCtrl::GetSelFilesList( wxArrayString & aResult )
{
	aResult.Clear();
	int nIndex = -1;
	for ( int i = 0; i < GetSelectedItemCount(); i++ )
	{
		nIndex = GetNextItem( nIndex, wxLIST_NEXT_ALL , wxLIST_STATE_SELECTED );
		if ( nIndex == -1 )
			break;
		aResult.Add( GetFilename( nIndex ) );
	}

	return;
}

void CPlaylistCtrl::GetSelectedSongs(MusikSongIdArray & aResult)
{
	aResult.Clear();
	int nIndex = -1;
	
	if( m_pPlaylist && GetSelectedItemCount() > 0 )
	{
		for( int i = 0; i < GetSelectedItemCount(); i++ )
		{
			nIndex = GetNextItem( nIndex, wxLIST_NEXT_ALL , wxLIST_STATE_SELECTED );
			if ( nIndex == -1 )
				break;
			aResult.Add( m_pPlaylist->Item ( nIndex ) ); 
			
		}
	}
	return;
}

void CPlaylistCtrl::RefreshSelectedSongs()
{

	int nIndex = -1;

	if( GetSelectedItemCount() > 0 )
	{
		while((nIndex = GetNextItem( nIndex, wxLIST_NEXT_ALL , wxLIST_STATE_SELECTED )) != -1)
		{
			 RefreshItem(nIndex);
		}	
	}
}
wxLongLong CPlaylistCtrl::GetTotalFilesize()
{
	return m_pPlaylist ? m_pPlaylist->GetTotalFileSize(): 0;
}

int CPlaylistCtrl::GetTotalPlayingTimeInSeconds()
{
	return m_pPlaylist ? m_pPlaylist->GetTotalPlayingTimeInSeconds():0;
}

wxString CPlaylistCtrl::GetFilename( int nItem )
{
	if ( m_pPlaylist && nItem >= 0 )
	{
		return m_pPlaylist->Item ( nItem ).Song()->MetaData.Filename.GetFullPath();
	}
	//--- not found? return a null string ---//
	return m_EmptyString;
}



//----------------------------------------//
//--- various other functions we need. ---//
//----------------------------------------//
void CPlaylistCtrl::ShowIcons()
{
	if ( wxGetApp().Prefs.bPlaylistColumnEnable[PlaylistColumn::RATING] == 0 )
		SetImageList( g_NullImageList, wxIMAGE_LIST_SMALL );
	else
		SetImageList( g_RatingImages, wxIMAGE_LIST_SMALL );
}

void CPlaylistCtrl::UpdateSel( wxListEvent& event )
{
	m_nCurSel = event.GetIndex();
}


void CPlaylistCtrl::ResynchItem( int listindex, const MusikSongId & songid)
{
	if(m_pPlaylist && m_pPlaylist->GetCount() && listindex >= 0 &&(listindex < (int)m_pPlaylist->GetCount()) && (g_SourcesCtrl->GetSelType() == MUSIK_SOURCES_NOW_PLAYING))
	{
		m_pPlaylist->Item( listindex ) = songid;
		RefreshItem( listindex );
	}
}

void CPlaylistCtrl::Update( bool bSelFirst)
{

	//--- setup listbox colours from prefs	---//
	wxColour bk_col_light_active( wxGetApp().Prefs.bPlaylistBorder ? StringToColour( wxGetApp().Prefs.sPlaylistBorderColour ) : wxSystemSettings::GetColour( wxSYS_COLOUR_BTNHIGHLIGHT ) );
	wxColour bk_col_dark_active( wxGetApp().Prefs.bPlaylistBorder ? StringToColour( wxGetApp().Prefs.sPlaylistBorderColour ) : StringToColour( wxGetApp().Prefs.sPLStripeColour ) );

	m_LightAttr			= wxListItemAttr( wxSystemSettings::GetColour( wxSYS_COLOUR_WINDOWTEXT), wxSystemSettings::GetColour( wxSYS_COLOUR_BTNHIGHLIGHT ), wxNullFont );
	m_SelectedLightAttr	= wxListItemAttr( wxSystemSettings::GetColour( wxSYS_COLOUR_WINDOWTEXT), bk_col_light_active, g_fntListBold );
	m_DarkAttr			= wxListItemAttr( wxSystemSettings::GetColour( wxSYS_COLOUR_WINDOWTEXT), StringToColour( wxGetApp().Prefs.sPLStripeColour ), wxNullFont );
	m_SelectedDarkAttr	= wxListItemAttr( wxSystemSettings::GetColour( wxSYS_COLOUR_WINDOWTEXT), bk_col_dark_active, g_fntListBold );

	//--- SetItemCount() kinda tells the virtual list control to udpate ---//
	// no Freeze() here , because RescaleColumns(); will not work correctly then
	SetItemCount( ( long ) m_pPlaylist ? m_pPlaylist->GetCount():0 );
	RescaleColumns();

	SelectNone( );

	//--- sel first item, if we're supposed to ---//
	if ( bSelFirst && GetItemCount() )
		SetItemState( 0, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED );	
}

void CPlaylistCtrl::RescaleColumns( bool bFreeze, bool WXUNUSED(bSave), bool bAutoFit )
{
	if ( g_DisablePlacement || m_bColDragging)
		return;

	static bool bInRescaleColumns = false;
	if(bInRescaleColumns)
		return;
	bInRescaleColumns = true;
	if ( m_ColSaveNeeded && !bAutoFit )
		SaveColumns();

	if ( bFreeze )
		Freeze();

	//-------------------------------------------------//
	//--- size of the client area.					---//
	//-------------------------------------------------//
	wxSize client_size		= GetClientSize();
	size_t nTotalPercent	= 0;
	size_t nRemainingWidth	= 0;
	size_t nStaticWidth		= 0;
	size_t nDynamicWidth	= 0;
	size_t nCurrItem;
	
	//-------------------------------------------------//
	//--- if gtk, add width of scrollbar to static	---//
	//-------------------------------------------------//
	#ifdef __WXGTK__
		nStaticWidth += wxSystemSettings::GetMetric(wxSYS_HSCROLL_Y);	
	#endif 	

	//-------------------------------------------------//
    //--- find the size of all the static members	---//
	//-------------------------------------------------//
	for ( size_t i = 0; i < m_ColumnOrder.GetCount(); i++ )
	{
		nCurrItem = m_ColumnOrder.Item( i );
		if ( wxGetApp().Prefs.bPlaylistColumnDynamic[nCurrItem] == 0 )
			nStaticWidth += wxGetApp().Prefs.PlaylistColumnSize[nCurrItem];
	}

	//-------------------------------------------------//
	//--- if using smart columns, we need to find	---//
	//--- what percentages mean.					---//
	//-------------------------------------------------//
	if ( wxGetApp().Prefs.bPlaylistSmartColumns == 1 || bAutoFit )
	{
		for ( size_t i = 0; i < m_ColumnOrder.GetCount(); i++ )
		{
			nCurrItem = m_ColumnOrder.Item( i );
			if ( wxGetApp().Prefs.bPlaylistColumnDynamic[nCurrItem] == 1 )
				nTotalPercent += wxGetApp().Prefs.PlaylistColumnSize[nCurrItem];
		}

		if ( nTotalPercent == 0 )
			nTotalPercent = 1;
	}

	//-------------------------------------------------//
	//--- remaining width = window width - static	---//
	//--- width. (this is width for dynamic items)	---//		
	//-------------------------------------------------//
	nRemainingWidth = client_size.GetWidth() - nStaticWidth;

	//-------------------------------------------------//
	//--- go in and set all the column widths, both	---//
	//--- static and dynamic.						---//
	//-------------------------------------------------//
	float f_Per;
	int n_Per;
	int n_LastDyn = -1;
	for ( size_t i = 0; i < m_ColumnOrder.GetCount(); i++ )
	{
		nCurrItem = m_ColumnOrder.Item( i );

		//-------------------------//
		//--- set static size	---//
		//-------------------------//
		if ( wxGetApp().Prefs.bPlaylistColumnDynamic[nCurrItem] == 0 )
			SetColumnWidth( i, wxGetApp().Prefs.PlaylistColumnSize[nCurrItem] );

		//-------------------------//
		//--- set dynamic size	---//
		//-------------------------//
		else
		{
			n_LastDyn = i;

			if ( wxGetApp().Prefs.bPlaylistSmartColumns == 1 || bAutoFit )
			{
				f_Per = ( (float)wxGetApp().Prefs.PlaylistColumnSize[nCurrItem] / (float)nTotalPercent ) * nRemainingWidth;
				n_Per = (int)f_Per;
			}

			else
			{
				f_Per = ( (float)wxGetApp().Prefs.PlaylistColumnSize[nCurrItem] / 100.0f ) * nRemainingWidth;
				n_Per = (int)f_Per;		
			}
			if(n_Per > client_size.GetWidth()/2)
				n_Per = client_size.GetWidth()/2;
			nDynamicWidth += (int)n_Per;
			SetColumnWidth( i, n_Per );
		}
	}
	
	//-------------------------------------------------//
	//--- remaining pixels, that may have been		---//
	//--- lost by integer division.					---//
	//-------------------------------------------------//
	if ( ( wxGetApp().Prefs.bPlaylistSmartColumns == 1 || bAutoFit ) && nTotalPercent && nDynamicWidth )
	{
		m_Overflow = client_size.GetWidth() - ( nStaticWidth + nDynamicWidth );
		size_t nLastSize = GetColumnWidth( n_LastDyn ) + m_Overflow;
		SetColumnWidth( n_LastDyn, nLastSize );
	}

	//-------------------------------------------------//
	//--- make sure window is properly refreshed.	---//
	//-------------------------------------------------//
	if ( bFreeze )
	{
		Thaw();
		//CMusikListCtrl::Update(); // instantly update window content
	}

	if ( bAutoFit )
		SaveColumns();
	bInRescaleColumns = false;
}

void CPlaylistCtrl::ResetColumns( bool update, bool rescale )
{
	//-------------------------------------------------//
	//--- clear all columns							---//
	//-------------------------------------------------//
	size_t nColumnCount = GetColumnCount();
	for ( size_t i = 0; i < nColumnCount; i++ )
		DeleteColumn( 0 );

	FindColumnOrder();

	//-------------------------------------------------//
	//--- construct all columns, and set any static	---//
	//--- values. RescaleColumns() will be called	---//
	//--- afterwords to setup any dynamic columns.	---//
	//-------------------------------------------------//
	size_t nCurrType;
	for ( size_t i = 0; i < m_ColumnOrder.GetCount(); i++ )
	{
		nCurrType = m_ColumnOrder.Item( i );
		InsertColumn( i, wxGetTranslation(g_PlaylistColumn[nCurrType].Label), g_PlaylistColumn[nCurrType].Aligned, 50 );
	}

	ShowIcons();

	if ( rescale )
		RescaleColumns();

	if ( update )
		Update();
}

void CPlaylistCtrl::RateSel( int nVal )
{
	if(m_pPlaylist == NULL)
		return;
	wxASSERT(nVal >= MUSIK_MIN_RATING && nVal <= MUSIK_MAX_RATING);
	int nIndex = -1;
	//--- yeah, we can rate multiple items. find whats selected ---//
	wxGetApp().Library.BeginTransaction();
	for ( int i = 0; i < GetSelectedItemCount(); i++ )
	{
		nIndex = GetNextItem( nIndex, wxLIST_NEXT_ALL , wxLIST_STATE_SELECTED );
		if ( nIndex == -1 )
			break;
		//--- set db entry, then resync item(s) ---//
		wxGetApp().Library.SetRating( m_pPlaylist->Item ( nIndex ), nVal );		
	}
	wxGetApp().Library.EndTransaction();
    nIndex = -1;
    for ( int i = 0; i < GetSelectedItemCount(); i++ )
    {
        nIndex = GetNextItem( nIndex, wxLIST_NEXT_ALL , wxLIST_STATE_SELECTED );
        if ( nIndex == -1 )
            break;
        RefreshItem( nIndex );
    }
}

void CPlaylistCtrl::EditTag( int i )
{
	int nSelCount = GetSelectedItemCount();
	//--- call the tag dialog, disable this window. ---//
	//---  tag dialog will re-enable when complete  ---//
	if ( nSelCount > 0 )
	{
		MusikTagFrame* pMusikTagFrame = new MusikTagFrame( g_MusikFrame, this, i);
//		g_MusikFrame->Enable( FALSE );
		pMusikTagFrame->Show();
	}
}




void CPlaylistCtrl::DelSelSongs(bool bDeleteFromDB, bool bDeleteFromComputer)
{
	int nSelCount = GetSelectedItemCount();
	if( m_pPlaylist == NULL || nSelCount == 0)
		return;
	if( bDeleteFromComputer )
	{
		wxMessageDialog confirm( this, _( "Delete the selected songs from your computer?" ), MUSIKAPPNAME_VERSION, wxYES_NO | wxICON_STOP );
		if ( confirm.ShowModal() == wxID_NO )
			return;
	}
	else if( bDeleteFromDB )
	{
		wxMessageDialog confirm( this, _( "Delete the selected songs from wxMusik's internal database?" ), MUSIKAPPNAME_VERSION, wxYES_NO | wxICON_QUESTION );
		if ( confirm.ShowModal() == wxID_NO )
			return;
	}
	else if( MUSIK_SOURCES_LIBRARY == g_SourcesCtrl->GetSelType() )
	{// we do not want to delete entry from the library playlist ( it would have no effect if library is reselected)
		return;// ignore
	}

	wxString sError;
	Freeze();
	int nIndex = -1;
	

	//--- find which songs are selected, delete as we go along ---//
	wxString sFile;
	nIndex = -1;
	int nFirstSel = GetNextItem( nIndex, wxLIST_NEXT_ALL , wxLIST_STATE_SELECTED );
	int nDeletedSongs = 0;
	bool bSourceNowPlayingSelected = (g_SourcesCtrl->GetSelType() == MUSIK_SOURCES_NOW_PLAYING);
	for ( int i = 0; i < nSelCount; i++ )
	{
		//--- find next item to delete ---//
		nIndex = GetNextItem( nIndex, wxLIST_NEXT_ALL , wxLIST_STATE_SELECTED );
		if ( nIndex == -1 )
			break;
		
		// correct nIndex by nIndex - nDeletedSongs, substract the number of entry,
		// which have been already deleted from the array
		// because GetNextItem() still returns the old index values

        if(bSourceNowPlayingSelected)
        {
            // HACK: active playlist is "now playing" list => tell player about it.
            wxGetApp().Player.OnPlaylistEntryRemoving(nIndex - nDeletedSongs);
        }

		if(m_pPlaylist->Item( nIndex - nDeletedSongs).IsFormat(MUSIK_FORMAT_NETSTREAM))
		{
			if( MUSIK_SOURCES_NOW_PLAYING != g_SourcesCtrl->GetSelType() )
				continue;// net streams cannot be deleted from the playlist, except from now playing list  ( they are deleted in the sources box)
		}
	   //--- if its valid, delete ---//
		if( bDeleteFromDB )
		{
            MusikSongId & songid = m_pPlaylist->Item( nIndex - nDeletedSongs);
	    	wxString songpath = songid.Song()->MetaData.Filename.GetFullPath(); // get the filename before song is deleted from the array
			if( bDeleteFromComputer )
			{
				if ( !wxRemoveFile( songpath ) )
					sError += songpath + wxT( "\n" );
			}
			wxGetApp().Library.RemoveSong( songid );
		}
		m_pPlaylist->RemoveAt( nIndex - nDeletedSongs, 1 );
		nDeletedSongs ++;
	}
	//--- if certain files couldn't be deleted ---//
	if ( !sError.IsEmpty() )
		wxMessageBox( _( "Failed to delete the following files from your computer:\n\n " ) + sError, MUSIKAPPNAME_VERSION, wxICON_STOP );

	m_pParent->Update( false );

	//--- select the last known item ---//
	if ( nFirstSel > ( GetItemCount() - 1 ) )
		nFirstSel = GetItemCount() - 1;
	SetItemState( nFirstSel, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED );

	Thaw();
	if(bDeleteFromDB)
		g_ActivityAreaCtrl->ReloadAllContents();

	
}

void CPlaylistCtrl::RenameSelFiles()
{
	if ( m_ActiveThreadController.IsAlive() == false )
    {
		MusikSongIdArray songs;
		GetSelectedSongs( songs );
		m_ActiveThreadController.AttachAndRun( new MusikPlaylistRenameThread(this, songs ) );
    }
	else
		InternalErrorMessageBox(wxT("Previous thread not terminated correctly."));

}

void CPlaylistCtrl::RetagSelFiles()
{
	if ( m_ActiveThreadController.IsAlive() == false )
	{
		CMusikAutoTaggerFrame dlg(this);
		dlg.SetConvertUnderscoresToSpaces((bool)wxGetApp().Prefs.bAutoTagConvertUnderscoresToSpaces);
		if(dlg.ShowModal()==wxID_CANCEL)
			return;
		wxGetApp().Prefs.bAutoTagConvertUnderscoresToSpaces = dlg.GetConvertUnderscoresToSpaces() ?1:0;
		MusikSongIdArray songs;
		GetSelectedSongs( songs );
		m_ActiveThreadController.AttachAndRun( new MusikPlaylistRetagThread(this, dlg.GetMask(), songs ) );
	}
	else
		InternalErrorMessageBox(wxT("Previous thread not terminated correctly."));
}

void CPlaylistCtrl::RebuildTagSelFiles()
{
		wxArrayString aFiles;
		GetSelFilesList(aFiles);
		
		g_MusikFrame->AutoUpdate(aFiles, MUSIK_UpdateFlags::RebuildTags|MUSIK_UpdateFlags::WaitUntilDone );
		RefreshSelectedSongs();
}


//--------------------------------------//
//--- dnd related playlist functions ---//	//--- please don't call these functions manually, just let dnd do it ---//
//--------------------------------------//
void CPlaylistCtrl::DNDSetCurSel()
{
	//--- find whats currently selected for dnd ---//
	aCurSel.Clear();
	int nIndex = -1;
	if(m_pPlaylist && GetSelectedItemCount() > 0 )
	{
		for( int i = 0; i < GetSelectedItemCount(); i++ )
		{
			nIndex = GetNextItem( nIndex, wxLIST_NEXT_ALL , wxLIST_STATE_SELECTED );
			if ( nIndex == -1 )
				break;
			if(m_pPlaylist->Item( nIndex ).IsFormat(MUSIK_FORMAT_NETSTREAM) == false)
			{
				aCurSel.Add( nIndex );
			}
		}
	}
}

bool CPlaylistCtrl::DNDIsSel( int nVal )
{
	for ( size_t i = 0; i < aCurSel.GetCount(); i++ )
	{
		if ( nVal == aCurSel.Item( i ) )
			return true;
	}
	return false;
}

void  CPlaylistCtrl::MovePlaylistEntrys(int nMoveTo ,const wxArrayInt &arrToMove,bool bSelectItems)
{
	// assumes that arrToMove is sorted in ascending order

	if(m_pPlaylist == NULL)
		return;
    nMoveTo = m_pPlaylist->MoveEntrys(nMoveTo,arrToMove);
	if(bSelectItems)
	{
		//--- select new songs ---//
		SelectNone( );
		for ( size_t i = 0; i < arrToMove.GetCount(); i++ )
			SetItemState( nMoveTo + i, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED );

	}

}



void CPlaylistCtrl::DNDDone()
{
	//--- finalize dragging ---//
	Update(false);
	//--- clean up ---//
	aCurSel.Clear();
}


wxDragResult CPlaylistCtrl::OnDropFiles(wxCoord WXUNUSED(x), wxCoord WXUNUSED(y), const wxArrayString& filenames,wxDragResult def)
{
	g_MusikFrame->AutoUpdate(filenames,((def == wxDragMove)|| wxGetApp().Prefs.bAutoPlayOnDropFilesInPlaylist) ? (MUSIK_UpdateFlags::InsertFilesIntoPlayer|MUSIK_UpdateFlags::PlayFiles) : MUSIK_UpdateFlags::EnquequeFilesIntoPlayer );

   return def;
}

//----------------------------------------//
//--- MusikPlaylistRenameThread events ---//
//----------------------------------------//
void CPlaylistCtrl::OnThreadStart( wxCommandEvent& event )
{
	SetProgressType(event.GetExtraLong());
	SetProgress(0);

	//--- post the event. we're up and running now! ---//
	wxCommandEvent MusikStartProgEvt( wxEVT_COMMAND_MENU_SELECTED, MUSIK_FRAME_THREAD_START );
	wxPostEvent( g_MusikFrame, MusikStartProgEvt );
}

void CPlaylistCtrl::OnThreadProg( wxCommandEvent& event )
{
	SetProgress(event.GetExtraLong());
	//--- relay thread progress message to g_MusikFrame ---//
	g_MusikFrame->SetProgress( GetProgress() );

	wxCommandEvent MusikEndProgEvt( wxEVT_COMMAND_MENU_SELECTED, MUSIK_FRAME_THREAD_PROG );
	wxPostEvent( g_MusikFrame, MusikEndProgEvt );
}

void CPlaylistCtrl::OnThreadEnd( wxCommandEvent& WXUNUSED(event) )
{
	m_ActiveThreadController.Join();

	if( GetProgressType() == MUSIK_PLAYLIST_RETAG_THREAD )
	{
  		g_ActivityAreaCtrl->ReloadAllContents();
	}
	g_SourcesCtrl->UpdateCurrent();
	Update();

	//--- update locally ---//
	SetProgressType	( 0 );
	SetProgress	( 0 );

    //--- relay thread end message to g_MusikFrame ---//
	wxCommandEvent MusikEndProgEvt( wxEVT_COMMAND_MENU_SELECTED, MUSIK_FRAME_THREAD_END );
	wxPostEvent( g_MusikFrame, MusikEndProgEvt );
}

const MusikSongIdArray & CPlaylistCtrl::Playlist()
{
    static MusikSongIdArray empty;
	return m_pPlaylist ? *m_pPlaylist:empty;
}

void CPlaylistCtrl::OnPlayInstantly( wxCommandEvent& WXUNUSED(event) )
{
	MusikSongIdArray aResult;
	GetSelectedSongs(aResult);
	wxGetApp().Player.InsertToPlaylist(aResult);

}
void CPlaylistCtrl::OnPlayAsNext ( wxCommandEvent& WXUNUSED(event) )
{
	MusikSongIdArray aResult;
	GetSelectedSongs(aResult);
	wxGetApp().Player.InsertToPlaylist(aResult,wxGetApp().Player.IsPlaying() ? false : true);

}
void CPlaylistCtrl::OnPlayEnqueued	( wxCommandEvent& WXUNUSED(event) )
{
	MusikSongIdArray aResult;
	GetSelectedSongs(aResult);
	wxGetApp().Player.AddToPlaylist(aResult,wxGetApp().Player.IsPlaying() ? false : true);
}
void CPlaylistCtrl::OnPlayReplace	( wxCommandEvent& WXUNUSED(event) )
{	
	if ( m_pPlaylist && m_nCurSel > -1 )
	{
		wxGetApp().Player.PlayReplaceList(m_nCurSel,*m_pPlaylist);
	}
}

void CPlaylistCtrl::OnPlayReplaceWithSel( wxCommandEvent& WXUNUSED(event) )
{	
	if ( m_nCurSel > -1 )
	{
		MusikSongIdArray aResult;
		GetSelectedSongs(aResult);
		wxGetApp().Player.PlayReplaceList(0,aResult);
	}
}

void CPlaylistCtrl::OnShowInLibrary( wxCommandEvent& event )
{
	int id = event.GetId() - MUSIK_PLAYLIST_CONTEXT_SHOW_IN_LIBRARY_FIRST_ENTRY;

	CActivityBox * pBox = g_ActivityAreaCtrl->GetActivityBox((size_t)id);
	wxString sEntry;
	if ( m_nCurSel > -1 && pBox)
	{
		sEntry = GetItemText( m_nCurSel, pBox->ColId());
		g_ActivityAreaCtrl->UpdateSel(pBox,true);
		g_SourcesCtrl->SelectLibrary();
		pBox->SetFocus();
        wxGetApp().Yield(true);
		pBox->SetSel(sEntry);
	}
}
void CPlaylistCtrl::OnOpenFolderInFileManager( wxCommandEvent&  )
{
	if ( m_pPlaylist && m_nCurSel > -1 && m_nCurSel < (int)m_pPlaylist->GetCount() )
	{
		wxString sCommand = wxString::Format(wxGetApp().Prefs.sFilemanagerCmd, m_pPlaylist->Item(m_nCurSel).Song()->MetaData.Filename.GetPath().c_str());
		wxExecute(sCommand);
	}
}	
enum EMUSIK_SEARCHBOX_ID
{
	MUSIK_SEARCHBOX_TEXT = 1,
	MUSIK_SEARCHBOX_SEARCHMODE = 2,
	MUSIK_SEARCHBOX_FUZZYSEARCHMODE = 3,
	MUSIK_SEARCHBOX_TIMERID = 4,
	MUSIK_SEARCHBOX_CLEAR = 5

};

BEGIN_EVENT_TABLE(CSearchBox, wxPanel)
EVT_TEXT					(MUSIK_SEARCHBOX_TEXT,			CSearchBox::OnTextInput	)	// simple query box change
EVT_CHOICE					(MUSIK_SEARCHBOX_SEARCHMODE,	CSearchBox::OnSearchMode	) 
EVT_CHOICE					(MUSIK_SEARCHBOX_FUZZYSEARCHMODE,	CSearchBox::OnFuzzySearchMode ) 
EVT_TIMER					(MUSIK_SEARCHBOX_TIMERID, CSearchBox::OnTimer)
EVT_TEXT_ENTER				(MUSIK_SEARCHBOX_TEXT, CSearchBox::OnTextEnter)
EVT_BUTTON					(MUSIK_SEARCHBOX_CLEAR,CSearchBox::OnClear)
END_EVENT_TABLE()

CSearchBox::CSearchBox( wxWindow *parent )
:wxPanel( parent, -1, wxPoint( -1, -1 ), wxSize( -1, -1 ),
		wxNO_FULL_REPAINT_ON_RESIZE
		| wxTAB_TRAVERSAL
		|wxCLIP_CHILDREN
#ifndef __WXMAC__
		|wxBORDER_RAISED
#endif		 
		)
,m_Timer(this,MUSIK_SEARCHBOX_TIMERID)
{
	//--------------------//
	//--- simple query ---//
	//--------------------//
	wxBoxSizer *pSizer = new wxBoxSizer( wxHORIZONTAL );

	wxStaticText *stSearch	= new wxStaticText_NoFlicker( this, -1, _( "Search:" ),wxPoint( -1, -1 ), wxSize( -1, -1 ), wxALIGN_RIGHT  );
	m_pTextSimpleQuery		= new wxTextCtrl_NoFlicker( this, MUSIK_SEARCHBOX_TEXT, wxT( "" ), wxPoint( -1, -1 ), wxSize( -1, -1 ), 
			wxSIMPLE_BORDER
			|wxTE_PROCESS_ENTER 
			);
	wxButton *buttonClear =new wxButton_NoFlicker(this,MUSIK_SEARCHBOX_CLEAR,_("Clear"),wxDefaultPosition,wxDefaultSize,wxBU_EXACTFIT );
	const wxString searchmode_choices[] ={_("All words"),_("Exact phrase"),_("Any word")};
	wxChoice *choiceSearchmode = new wxChoice_NoFlicker(this,MUSIK_SEARCHBOX_SEARCHMODE,wxDefaultPosition,wxDefaultSize,WXSIZEOF(searchmode_choices),searchmode_choices);
	choiceSearchmode->SetSelection(wxGetApp().Prefs.eSearchmode.val);
	const wxString fuzzysearchmode_choices[] ={_("Fuzzy None"),_("Fuzzy Low"),_("Fuzzy Middle"),_("Fuzzy High")};
	wxChoice *choiceFuzzySearchmode = new wxChoice_NoFlicker(this,MUSIK_SEARCHBOX_FUZZYSEARCHMODE,wxDefaultPosition,wxDefaultSize,WXSIZEOF(fuzzysearchmode_choices),fuzzysearchmode_choices);
	choiceFuzzySearchmode->SetSelection(wxGetApp().Prefs.eFuzzySearchmode.val);
	pSizer->Add( stSearch,	0, wxRIGHT | wxADJUST_MINSIZE | wxALIGN_CENTER_VERTICAL|wxALL, 4 );
	pSizer->Add( m_pTextSimpleQuery, 1, wxADJUST_MINSIZE |wxALIGN_CENTER_VERTICAL|wxRIGHT|wxTOP|wxBOTTOM,4);
	pSizer->Add( buttonClear, 0, wxADJUST_MINSIZE |wxALIGN_CENTER_VERTICAL|wxRIGHT|wxTOP|wxBOTTOM,4);
	pSizer->Add( choiceSearchmode, 0, wxADJUST_MINSIZE |wxALIGN_CENTER_VERTICAL|wxRIGHT|wxTOP|wxBOTTOM,4);
	pSizer->Add( choiceFuzzySearchmode, 0, wxADJUST_MINSIZE |wxALIGN_CENTER_VERTICAL|wxRIGHT|wxTOP|wxBOTTOM,4);
	m_arrFieldsToSearch.Add(g_PlaylistColumn[PlaylistColumn::ARTIST].DBName);
	m_arrFieldsToSearch.Add(g_PlaylistColumn[PlaylistColumn::ALBUM].DBName);
	m_arrFieldsToSearch.Add(g_PlaylistColumn[PlaylistColumn::TITLE].DBName);
	m_arrFieldsToSearch.Add(g_PlaylistColumn[PlaylistColumn::FILENAME].DBName);
	m_arrFieldsToSearch.Add(g_PlaylistColumn[PlaylistColumn::NOTES].DBName);
	m_arrFieldsToSearch.Add(g_PlaylistColumn[PlaylistColumn::GENRE].DBName);
	SetSizer(pSizer);
}
CSearchBox::~CSearchBox()
{
}

void CSearchBox::DoSearchQuery( wxString sQueryVal )
{
	if(   !sQueryVal.IsEmpty() )
	{
		sQueryVal.Replace( wxT("'"), wxT("''") ); //--- double apostrophe to make valid syntax ---//
		sQueryVal.MakeLower();
		wxArrayString sTokens;
		wxString sCompareTempl;
		
		wxString sSetOperator(wxT(" and "));
		EMUSIK_FUZZYSEARCHMODE fm = wxGetApp().Prefs.eFuzzySearchmode;
		double fuzzy = 0.0;
		switch (fm)
		{
		case MUSIK_FUZZYSEARCHMODE_NONE:
			sCompareTempl = wxT("LIKE('%%%s%%',%s)");
			fuzzy = 0.0;
			break;
		case MUSIK_FUZZYSEARCHMODE_LOW:
			sCompareTempl << wxT("fuzzycmp('.*%s.*',lower(%s),%d)"); 
			fuzzy = 1.2;
			break;
		case MUSIK_FUZZYSEARCHMODE_MIDDLE:
			sCompareTempl << wxT("fuzzycmp('.*%s.*',lower(%s),%d)"); 

			fuzzy = 1.5;
			break;
		case MUSIK_FUZZYSEARCHMODE_HIGH:
			sCompareTempl << wxT("fuzzycmp('.*%s.*',lower(%s),%d)"); 
			fuzzy = 1.8;
			break;

		}
		if(fuzzy > 0.0)
		{
			// we need to escape (){}[]*.|\+? etc.
			wxRegEx reMatchRegexSpecialChars(wxT("([{}\\:\\^\\*\\.\\+\\$\\(\\)\\|\\?\\\\]|\\[|\\])"));
			reMatchRegexSpecialChars.ReplaceAll(&sQueryVal,wxT("\\\\\\1"));// replace all special regex chars by \char
		}
		EMUSIK_SEARCHMODE sm = wxGetApp().Prefs.eSearchmode;
		switch(	sm )
		{
		case MUSIK_SEARCHMODE_ALLWORDS:
				DelimitStr(sQueryVal,wxT(" "),sTokens,true);
				
				break;
		case MUSIK_SEARCHMODE_ANYWORDS:
				sSetOperator = wxT(" or ");
				DelimitStr(sQueryVal,wxT(" "),sTokens,true);
				break;
		case MUSIK_SEARCHMODE_EXACTPHRASE:
				sTokens.Add(sQueryVal);
				break;
		}

		wxString sQuery;

		for ( size_t i = 0; i < sTokens.GetCount() ; i++)
		{
			sQuery+= wxT("(");
			for ( size_t field = 0; field < m_arrFieldsToSearch.GetCount() ; field++)
			{
				int errors = ((sTokens[i].Length() > 3) && (fuzzy != 0.0)) ? (int)((double) sTokens[i].Length()  / 10.0  * fuzzy +0.5): 0;  
				sQuery+= wxString::Format( sCompareTempl,sTokens[i].c_str(),m_arrFieldsToSearch[field].c_str(),errors );
				if(field != m_arrFieldsToSearch.GetCount() - 1)
					sQuery+= wxT(" or ");
			}
			sQuery+= wxT(")");
			if(i != sTokens.GetCount() - 1)
				sQuery += sSetOperator;
		}

		wxGetApp().Library.QuerySongsWhere( sQuery, g_thePlaylist ,true);  // true means query sorted
	}
	else
	{
		if ( wxGetApp().Prefs.bShowAllSongs == 1 )
			wxGetApp().Library.GetAllSongs(g_thePlaylist);
	}
	g_PlaylistBox->SetPlaylist(&g_thePlaylist);
}
void CSearchBox::OnTextInput(wxCommandEvent &)
{
	m_Timer.Start(650 );	
}
void CSearchBox::OnSearchMode( wxCommandEvent&	event )
{
	int modesel = event.GetSelection();
	wxGetApp().Prefs.eSearchmode = (EMUSIK_SEARCHMODE)modesel;
	DoSearchQuery(m_pTextSimpleQuery->GetValue());
}

void CSearchBox::OnFuzzySearchMode( wxCommandEvent&	event )
{
	int modesel = event.GetSelection();
	wxGetApp().Prefs.eFuzzySearchmode = (EMUSIK_FUZZYSEARCHMODE)modesel;
	DoSearchQuery(m_pTextSimpleQuery->GetValue());
}
void CSearchBox::OnTimer(wxTimerEvent& )
{
	m_Timer.Stop();
	DoSearchQuery(m_pTextSimpleQuery->GetValue());
}
void CSearchBox::OnTextEnter(wxCommandEvent &)
{
	m_Timer.Stop();// enter was pressed => stop the timer, so we do not DoSearchQuery again after timer is expired.
	DoSearchQuery(m_pTextSimpleQuery->GetValue());
}

void CSearchBox::OnClear(wxCommandEvent&)
{
	m_pTextSimpleQuery->SetValue(wxEmptyString);	
}
