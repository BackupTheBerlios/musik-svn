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
#include "wx/wxprec.h"

#include "wx/file.h"

#include "PlaylistCtrl.h"

//--- globals ---//
#include "../MusikGlobals.h"
#include "../MusikUtils.h"

//--- frames ---//
#include "../Frames/MusikFrame.h"
#include "../Frames/MusikTagFrame.h"
#include "../Frames/MusikAutoTaggerFrame.h"
//--- threads ---//
#include "../Threads/PlaylistCtrlThreads.h"

// other
#include "../DataObjectCompositeEx.h"
#include "../DNDHelper.h"


class PlaylistDropTarget : public wxDropTarget
{
public:
	//-------------------//
	//--- constructor ---//
	//-------------------//
	PlaylistDropTarget( CPlaylistCtrl *pPList )	
	{ 
		m_pPlaylistCtrl = pPList;	

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
	wxTextDataObject * m_pSonglistDObj;
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

	//--- CSourcesListBox ---//
	m_pPlaylistCtrl	= new CPlaylistCtrl( this, MUSIK_PLAYLIST, wxPoint( -1, -1 ), wxSize( -1, -1 ) );
	m_pPlaylistInfoCtrl = new CPlaylistInfoCtrl( this ,  m_pPlaylistCtrl);
	m_pSearchBox = new CSearchBox(this);
	m_pInfoSearchSizer = new wxBoxSizer( wxVERTICAL );
	m_pInfoSearchSizer->Add( m_pSearchBox, 0, wxEXPAND|wxALL,2 );
	m_pInfoSearchSizer->Add( m_pPlaylistInfoCtrl, 0, wxEXPAND );
	//--- top sizer ---//
	m_pMainSizer = new wxBoxSizer( wxVERTICAL );
	m_pMainSizer->Add( m_pInfoSearchSizer, 0, wxADJUST_MINSIZE|wxEXPAND|wxRIGHT|wxLEFT|wxBOTTOM|wxTOP  , 2 );
	
	m_pMainSizer->Add(  m_pPlaylistCtrl, 1, wxEXPAND|wxRIGHT|wxLEFT|wxBOTTOM,5);
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
	SetBackgroundColour(wxGetApp().Prefs.bPlaylistBorder ?  
										StringToColour(wxGetApp().Prefs.sPlaylistBorderColour)
										:wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
	m_pPlaylistCtrl->Update(bSelFirstItem);
	if ( wxGetApp().Prefs.bShowPLInfo )
	{
		m_pPlaylistInfoCtrl->Update();
	}
	Layout();

}
void CPlaylistBox::OnEraseBackground( wxEraseEvent& (event) )
{	
	// empty => no background erasing to avoid flicker

	wxDC * TheDC = event.m_dc;
	wxColour BGColor =  GetBackgroundColour();
	wxBrush MyBrush(BGColor ,wxSOLID);
	TheDC->SetBackground(MyBrush);

	wxCoord width,height;
	TheDC->GetSize(&width,&height);
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
	EVT_MENU_RANGE				( MUSIK_PLAYLIST_CONTEXT_SHOW_IN_LIBRARY_ARTIST,MUSIK_PLAYLIST_CONTEXT_SHOW_IN_LIBRARY_YEAR, CPlaylistCtrl::OnShowInLibrary)
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
	EVT_MENU					( MUSIK_PLAYLIST_DISPLAY_FIT,											CPlaylistCtrl::OnDisplayFit			)
	EVT_MENU					( MUSIK_PLAYLIST_CLEARPLAYERLIST,										CPlaylistCtrl::OnClearPlayerlist	)
	EVT_CONTEXT_MENU			(																		CPlaylistCtrl::ShowMenu				)
	EVT_KEY_DOWN				(																		CPlaylistCtrl::OnKeyDown			)
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
		//if ( !m_pPlaylistCtrl->DNDIsSel( nLastHit ) )
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
	
{
	SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_BTNHIGHLIGHT ) );

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
		playlist_context_rating_menu->Append(  MUSIK_PLAYLIST_CONTEXT_RATING  + (i - MUSIK_MIN_RATING), (i == 0) ? _( "Unrated" ) : IntToString(i), wxT( "" ), wxITEM_CHECK );
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
	playlist_context_edit_tag_menu->Append( MUSIK_PLAYLIST_CONTEXT_RENAME_FILES,	_( "&Auto Rename" ) );
	playlist_context_edit_tag_menu->Append( MUSIK_PLAYLIST_CONTEXT_RETAG_FILES,		_( "A&uto Retag..." ) );
	playlist_context_edit_tag_menu->Append( MUSIK_PLAYLIST_CONTEXT_REBUILDTAG,		_( "&Rebuild Tag" ) );

	//--- delete menu ---//
	wxMenu *playlist_context_delete_menu = new wxMenu;
	playlist_context_delete_menu->Append( MUSIK_PLAYLIST_DELETE_CONTEXT_DELETE_FROM_PLAYLIST,	_( "From Playlist\tDel" ) );
	playlist_context_delete_menu->Append( MUSIK_PLAYLIST_DELETE_CONTEXT_DELETE_FROM_DB,			_( "From Database\tAlt+Del" ) );
	playlist_context_delete_menu->Append( MUSIK_PLAYLIST_DELETE_CONTEXT_DELETE_FILES,			_( "From Computer\tCtrl+Del" ) );

	//--- columns context menu ---//
	wxMenu *playlist_context_display_menu = new wxMenu;
	for(size_t i = 0 ; i < NPLAYLISTCOLUMNS;i ++)
	{
		playlist_context_display_menu->AppendCheckItem( MUSIK_PLAYLIST_DISPLAY_FIRST + i,wxGetTranslation(g_PlaylistColumnLabels[i]) );
	}
	playlist_context_display_menu->AppendSeparator();
	playlist_context_display_menu->AppendCheckItem( MUSIK_PLAYLIST_DISPLAY_FIT,				_( "Fit Columns" ) );
	playlist_context_display_menu->AppendCheckItem( MUSIK_PLAYLIST_DISPLAY_SMART,			_( "No Horizontal Scroll" ) );

	bool bNetStreamSel = false;
	if ( GetSelectedItemCount() > 0 )
	{
		int nFirstIndex = GetNextItem( -1, wxLIST_NEXT_ALL , wxLIST_STATE_SELECTED );
		bNetStreamSel  = (g_Playlist.Item ( nFirstIndex ).MetaData.eFormat == MUSIK_FORMAT_NETSTREAM);
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

	if(g_ActivityAreaCtrl->GetActivityBox(MUSIK_LBTYPE_ARTISTS))
		playlist_context_show_in_library_menu->Append( MUSIK_PLAYLIST_CONTEXT_SHOW_IN_LIBRARY_ARTIST,_( "This artist" ),	wxT("") );
	if(g_ActivityAreaCtrl->GetActivityBox(MUSIK_LBTYPE_ALBUMS))
		playlist_context_show_in_library_menu->Append( MUSIK_PLAYLIST_CONTEXT_SHOW_IN_LIBRARY_ALBUM,_( "This album" ),	wxT("") );
	if(g_ActivityAreaCtrl->GetActivityBox(MUSIK_LBTYPE_GENRES))
		playlist_context_show_in_library_menu->Append( MUSIK_PLAYLIST_CONTEXT_SHOW_IN_LIBRARY_GENRE,_( "This genre" ),	wxT("") );
	if(g_ActivityAreaCtrl->GetActivityBox(MUSIK_LBTYPE_YEARS))
		playlist_context_show_in_library_menu->Append( MUSIK_PLAYLIST_CONTEXT_SHOW_IN_LIBRARY_YEAR,_( "This year" ),	wxT("") );



	playlist_context_menu->Append( MUSIK_PLAYLIST_CONTEXT_SHOW_IN_LIBRARY_NODE,_( "&Show in Library" ),	playlist_context_show_in_library_menu );
	playlist_context_menu->Append( MUSIK_PLAYLIST_CONTEXT_OPEN_FOLDER_IN_FILEMANAGER,_( "&Open Folder in File Manager" ),wxT(""));
	

	playlist_context_menu->Append( MUSIK_PLAYLIST_CONTEXT_RATENODE,			_( "&Rating" ),					playlist_context_rating_menu );
	playlist_context_menu->Append( MUSIK_PLAYLIST_CONTEXT_DISPLAYNODE,		_( "Display" ),					playlist_context_display_menu );
	playlist_context_menu->AppendSeparator();
	playlist_context_menu->Append( MUSIK_PLAYLIST_CONTEXT_DELETENODE,		_( "Delete" ),		playlist_context_delete_menu );
	playlist_context_menu->AppendSeparator();
	playlist_context_menu->Append( MUSIK_PLAYLIST_CONTEXT_TAGNODE,			_( "Edit &Tag" ),				playlist_context_edit_tag_menu );

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
	if(g_SourcesCtrl->GetSelType() != MUSIK_SOURCES_LIBRARY) // only sort in library view, else we will destroy the user playlist
		return;
	int ActualColumn = m_ColumnOrder.Item( event.GetColumn() );
	
	int currentsortorder = m_aColumnSorting.Item( ActualColumn );
	for(size_t i = 0; i < m_aColumnSorting.GetCount();i++)
    	m_aColumnSorting.Item( i ) = 0;	   // set all columns to unsorted

	m_aColumnSorting.Item( ActualColumn ) = currentsortorder > 0 ? -1 : 1;   // toggle sort order

	bool desc = ( m_aColumnSorting.Item( ActualColumn ) < 0 );

	wxGetApp().Library.SetSortOrderField( ActualColumn, desc );
	wxGetApp().Library.RedoLastQuerySongsWhere( g_Playlist ,true);//sorted
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
			wxGetApp().Prefs.nPlaylistColumnSize[nCurrCol] = GetColumnWidth( i );
			nStaticWidth += GetColumnWidth( i );
		}
	}

	//---------------------------------------------------------//
	//--- calculate the dynamic size columns. these are		---//
	//--- these are the percent of the window the current	---//
	//--- column takes up.									---//
	//---------------------------------------------------------//
	size_t nRemaining = client_size.GetWidth() - nStaticWidth;
	for ( size_t i = 0; i < m_ColumnOrder.GetCount(); i++ )
	{
		nCurrCol = m_ColumnOrder.Item( i );

		if ( wxGetApp().Prefs.bPlaylistColumnDynamic[nCurrCol] == 1 )
		{
			f_Pos = (float)GetColumnWidth( i ) / (float)nRemaining * 100.0f;
			n_Pos = MusikRound( f_Pos );

			if ( n_Pos < 1 )
				n_Pos = 1;

			wxGetApp().Prefs.nPlaylistColumnSize[nCurrCol] = n_Pos;
		}
	}
}

//--------------//
//--- events ---//
//--------------//
void CPlaylistCtrl::ShowMenu( wxContextMenuEvent& WXUNUSED(event) )
{
	wxPoint pos = ScreenToClient( wxGetMousePosition() );

	wxMenu *playlist_context_menu = CreateContextMenu();

	PopupMenu( playlist_context_menu, pos );
	delete playlist_context_menu;
}
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

	g_Playlist.Clear();
	wxGetApp().Player.SetPlaylist(g_Playlist);
	if(wxGetApp().Prefs.bStopSongOnNowPlayingClear)
		wxGetApp().Player.Stop();
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
	if(nFirstIndex >= 0 && g_Playlist.Item ( nFirstIndex ).MetaData.eFormat == MUSIK_FORMAT_NETSTREAM)
	{
		if((event.GetId() == MUSIK_PLAYLIST_DELETE_CONTEXT_DELETE_FROM_PLAYLIST) 
			&&(g_SourcesCtrl->GetSelType() == MUSIK_SOURCES_NOW_PLAYING))
			bEnable = true;
		else
			bEnable = false;

	}
	event.Enable(bEnable);
}

void CPlaylistCtrl::OnUpdateUIRateSel ( wxUpdateUIEvent &event)
{
	//--- get rating for first sel ---//
	int item = GetNextItem( -1, wxLIST_NEXT_ALL , wxLIST_STATE_SELECTED );
	
	if(item > -1)
	{
		int nRating = g_Playlist.Item( item ).Rating;
		wxASSERT(nRating >= MUSIK_MIN_RATING && nRating <= MUSIK_MAX_RATING);
		event.Check( event.GetId() == (MUSIK_PLAYLIST_CONTEXT_RATING + (nRating  - MUSIK_MIN_RATING)));
	}
}

void CPlaylistCtrl::OnDisplaySmart( wxCommandEvent& WXUNUSED(event) )
{
	wxGetApp().Prefs.bPlaylistSmartColumns = !wxGetApp().Prefs.bPlaylistSmartColumns;
	RescaleColumns();
}

void CPlaylistCtrl::BeginDrag( wxListEvent& WXUNUSED(event) )
{
	//--- pass selected items ---//
	wxString sValidSelFiles = GetSelFiles();
	if(!sValidSelFiles.IsEmpty())
	{
		wxString sDrop (sValidSelFiles);
		DNDSetCurSel();
		  
		//--- initialize drag and drop... SourcesDropTarget / PlaylistDropTarget should take care of the rest ---//
		wxDropSource dragSource( this );
		CMusikSonglistDataObject *psong_data = new CMusikSonglistDataObject(sDrop);
		wxFileDataObject *pfile_data = new wxFileDataObject;
		for (size_t i = 0; i < aCurSel.GetCount();i++)
		{
			pfile_data->AddFile(g_Playlist[aCurSel[i]].MetaData.Filename.GetFullPath());
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
	if ( event.AltDown() == TRUE )
	{
		switch( nKeyCode )
		{
			case WXK_DELETE:
			case WXK_BACK:
				DelSelSongs(true);// delete songs from db
				break;
			default:
				event.Skip();
				return;
		}
	}
	else if ( event.ControlDown() == TRUE )
	{
		switch( nKeyCode )
		{
		case 'A':	//--- a / A ---//
			wxListCtrlSelAll( this );
			break;
		case 'D':	//--- d / D ---//
			wxListCtrlSelNone( this );
			break;
		case WXK_DELETE:
		case WXK_BACK:
			DelSelSongs(true,true);// delete from db and computer
			break;
		default:
			event.Skip();
			return;
		}
	}
	else if ( event.AltDown() == FALSE )
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
					break;
				case WXK_DELETE:
				case WXK_BACK:
					DelSelSongs();
					break;
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
					break;
				case 'I':
					{
						wxCommandEvent dummy;
						OnPlayInstantly( dummy );
					}
					break;
				case 'E':
					{
						wxCommandEvent dummy;
						OnPlayEnqueued( dummy );
					}
					break;
				case 'N':
					{
						wxCommandEvent dummy;
						OnPlayAsNext( dummy );
					}
					break;
				default:
					event.Skip();
					return;
			}
		}
	}
	else
		event.Skip();   // we did not handle the key, so propagate it further
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

	EPLAYLISTCOLUMNS eCurrType = (EPLAYLISTCOLUMNS)m_ColumnOrder.Item( column );
	return GetItemText(item,eCurrType);

}

wxString CPlaylistCtrl::EmptyColumnString()	const
{
	return wxGetApp().Prefs.bDisplayEmptyPlaylistColumnAsUnkown == true ? _( "<unknown>" ):wxEmptyString;
}

wxString CPlaylistCtrl::GetItemText(long item, EPLAYLISTCOLUMNS eColumnType) const
{
	if(item >= (long)g_Playlist.GetCount())
		return wxT( "" );
	const CMusikSong & song = g_Playlist.Item ( item );
	switch ( eColumnType )
	{
	case PLAYLISTCOLUMN_RATING:
		break;

	case PLAYLISTCOLUMN_TRACK:
		if ( song.MetaData.nTracknum > 0 )
			return wxString::Format( wxT( "%d" ), song.MetaData.nTracknum );
		else
			return wxT( "-" );
		break;

	case PLAYLISTCOLUMN_TITLE:
		return SanitizedString(ConvFromUTF8( song.MetaData.Title ));		
		break;

	case PLAYLISTCOLUMN_ARTIST:
		if ( song.MetaData.Artist.IsEmpty() )
			return EmptyColumnString();
			
		else 
		{
			return SanitizedString(ConvFromUTF8( song.MetaData.Artist ));
		}
		break;

	case PLAYLISTCOLUMN_ALBUM:
		if ( song.MetaData.Album.IsEmpty() )
			return  EmptyColumnString();
		else
			return SanitizedString(ConvFromUTF8( song.MetaData.Album ));
		break;

	case PLAYLISTCOLUMN_YEAR:
		if ( song.MetaData.Year.IsEmpty() )
			return  EmptyColumnString();
		else
			return ConvFromUTF8(song.MetaData.Year);
		break;

	case PLAYLISTCOLUMN_GENRE:
		if ( song.MetaData.Genre.IsEmpty() )
			return  EmptyColumnString();
		else
			return SanitizedString(ConvFromUTF8( song.MetaData.Genre ));
		break;

	case PLAYLISTCOLUMN_TIMES_PLAYED:
		{

			return (song.TimesPlayed > 0) ? wxString(IntToString(song.TimesPlayed)) : wxString(wxT("-"));
		}
		break;

	case PLAYLISTCOLUMN_LAST_PLAYED:
		if ( song.LastPlayed != 0.0 )
		{
			wxDateTime dt(song.LastPlayed);
			return dt.Format(wxT("%x %X"));
		}
		else
			return _("Never");
		break;

	case PLAYLISTCOLUMN_TIME:
		return MStoStr( song.MetaData.nDuration_ms );
		break;

	case PLAYLISTCOLUMN_BITRATE:
		return IntTowxString( song.MetaData.nBitrate );
		break;

	case PLAYLISTCOLUMN_FILENAME:
		{
			return song.MetaData.Filename.GetFullPath();
		}
		break;
	case PLAYLISTCOLUMN_NOTES:
		return ConvFromUTF8(song.MetaData.Notes);
	case PLAYLISTCOLUMN_TIMEADDED:
		{
		wxDateTime dt(song.TimeAdded);
		return dt.Format(wxT("%x %X"));
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
	for ( int i = 0; i < NPLAYLISTCOLUMNS; i++ )
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
	return g_Playlist.Item ( item ).Rating - MUSIK_MIN_RATING;
}

wxListItemAttr* CPlaylistCtrl::OnGetItemAttr(long item) const
{
	if(item >= (long)g_Playlist.GetCount())
		return (wxListItemAttr *)&m_LightAttr;
	const CMusikSong & song = g_Playlist.Item ( item );
	if(wxGetApp().Player.IsPlaying() && (g_SourcesCtrl->GetSelType() == MUSIK_SOURCES_NOW_PLAYING) 
		&& (wxGetApp().Player.GetCurIndex() == (size_t)item ) && (song.songid == wxGetApp().Player.GetCurrentSongid()))
	{
		if ( wxGetApp().Prefs.bPLStripes == 1 )
			return item % 2 ? (wxListItemAttr *)&m_SelectedDarkAttr : (wxListItemAttr *)&m_SelectedLightAttr;
		else
			return (wxListItemAttr *)&m_SelectedLightAttr;
	}
	else if ( wxGetApp().Player.IsPlaying() && (g_SourcesCtrl->GetSelType() != MUSIK_SOURCES_NOW_PLAYING) 
		&& song.songid == wxGetApp().Player.GetCurrentSongid() && song.MetaData.Filename.GetFullPath() == wxGetApp().Player.GetCurrentFilename() )
	{
		if ( wxGetApp().Prefs.bPLStripes == 1 )
			return item % 2 ? (wxListItemAttr *)&m_SelectedDarkAttr : (wxListItemAttr *)&m_SelectedLightAttr;
		else
			return (wxListItemAttr *)&m_SelectedLightAttr;
	}
	else
	{
		if ( wxGetApp().Prefs.bPLStripes == 1 )
			return item % 2 ? (wxListItemAttr *)&m_DarkAttr : (wxListItemAttr *)&m_LightAttr;
		else
			return (wxListItemAttr *)&m_LightAttr;
	}
}

//--------------------------------------------------//
//--- functions to get information from the list ---//
//--------------------------------------------------//
wxString CPlaylistCtrl::GetSubitemText( int nItem, int nSubitem )
{
	wxListItem iInfo;
	iInfo.SetMask		( wxLIST_MASK_TEXT );
	iInfo.SetId			( nItem );
	iInfo.SetColumn		( nSubitem );
	GetItem				( iInfo );
	return				( iInfo.GetText() );
}

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

wxString CPlaylistCtrl::GetSelFiles()
{
	wxString sResult;
	int nIndex = -1;
	int itemcount = GetSelectedItemCount();
	sResult.Alloc(itemcount * 50); // 50 is just a for the average file name length
	for ( int i = 0; i < itemcount; i++ )
	{
		nIndex = GetNextItem( nIndex, wxLIST_NEXT_ALL , wxLIST_STATE_SELECTED );
		if( i == -1)
			break;
		if(g_Playlist.Item( nIndex ).MetaData.eFormat != MUSIK_FORMAT_NETSTREAM)
		{
			sResult += GetFilename( nIndex );
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

void CPlaylistCtrl::GetSelectedSongs(CMusikSongArray & aResult)
{
	aResult.Clear();
	int nIndex = -1;
	
	if( GetSelectedItemCount() > 0 )
	{
		for( int i = 0; i < GetSelectedItemCount(); i++ )
		{
			nIndex = GetNextItem( nIndex, wxLIST_NEXT_ALL , wxLIST_STATE_SELECTED );
			if ( nIndex == -1 )
				break;
			aResult.Add( g_Playlist.Item ( nIndex ) ); 
			
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
			CMusikSong & refSong = g_Playlist[nIndex];
			if(refSong.MetaData.eFormat == MUSIK_FORMAT_NETSTREAM)
				continue; // skip net streams
			 wxGetApp().Library.GetSongFromSongid(refSong.songid,&refSong);
			 RefreshItem(nIndex);
		}	
	}
}
wxLongLong CPlaylistCtrl::GetTotalFilesize()
{
	wxLongLong filesize = 0;
	for ( size_t i = 0; i < g_Playlist.GetCount(); i++ )
	{
		const CMusikSong &song = g_Playlist.Item( i );
		filesize += song.MetaData.nFilesize;
	}

	return filesize;
}

int CPlaylistCtrl::GetTotalPlayingTimeInSeconds()
{
	int Duration = 0;
	for ( int i = 0; i < GetItemCount(); i++ )
	{
		const CMusikSong & song = g_Playlist.Item ( i );
		Duration += song.MetaData.nDuration_ms/1000;
	}
	return Duration;
}

const wxString & CPlaylistCtrl::GetFilename( int nItem )
{
	if ( nItem > -1 )
	{
		const CMusikSong & song = g_Playlist.Item ( nItem );
		return song.MetaData.Filename.GetFullPath();
	}

	//--- not found? return a null string ---//
	return m_EmptyString;
}



//----------------------------------------//
//--- various other functions we need. ---//
//----------------------------------------//
void CPlaylistCtrl::ShowIcons()
{
	if ( wxGetApp().Prefs.bPlaylistColumnEnable[PLAYLISTCOLUMN_RATING] == 0 )
		SetImageList( g_NullImageList, wxIMAGE_LIST_SMALL );
	else
		SetImageList( g_RatingImages, wxIMAGE_LIST_SMALL );
}

void CPlaylistCtrl::UpdateSel( wxListEvent& event )
{
	m_nCurSel = event.GetIndex();
}

void CPlaylistCtrl::ResynchItem( int item, int lastitem, bool refreshonly )
{
	if ( !refreshonly )
	{
		int songid = g_Playlist.Item( item ).songid;
		wxGetApp().Library.GetSongFromSongid( songid, &g_Playlist.Item( item ) );
		
		if ( lastitem > -1 && lastitem != item )
		{
			int songid = g_Playlist.Item( lastitem ).songid;
			wxGetApp().Library.GetSongFromSongid( songid, &g_Playlist.Item(lastitem ) );		
		}			
	}

	if(lastitem == -1)
		 RefreshItem( item );
	else
		Refresh();
}
void CPlaylistCtrl::ResynchItem( int item, const CMusikSong & song)
{
	if(g_Playlist.GetCount() && (item < (int)g_Playlist.GetCount()) && (g_SourcesCtrl->GetSelType() == MUSIK_SOURCES_NOW_PLAYING))
	{
		g_Playlist.Item( item ) = song;
		RefreshItem( item );
	}
}

void CPlaylistCtrl::Update( bool bSelFirst)
{
	//----------------------------------------------------------------------------------//
	//---         note that the playlist control is now virtual, so we don't         ---//
	//---    add items directly to it.  Instead, we have MusikFrame::g_Playlist, a   ---//
	//---  CMusikSongArray, that the virtual listctrl then references via callbacks  ---//
	//----------------------------------------------------------------------------------//

	//--- setup listbox colours from prefs	---//
	wxColour bk_col_light_active( wxGetApp().Prefs.bPlaylistBorder ? StringToColour( wxGetApp().Prefs.sPlaylistBorderColour ) : wxSystemSettings::GetColour( wxSYS_COLOUR_BTNHIGHLIGHT ) );
	wxColour bk_col_dark_active( wxGetApp().Prefs.bPlaylistBorder ? StringToColour( wxGetApp().Prefs.sPlaylistBorderColour ) : StringToColour( wxGetApp().Prefs.sPLStripeColour ) );

	m_LightAttr			= wxListItemAttr( wxSystemSettings::GetColour( wxSYS_COLOUR_WINDOWTEXT), wxSystemSettings::GetColour( wxSYS_COLOUR_BTNHIGHLIGHT ), wxNullFont );
	m_SelectedLightAttr	= wxListItemAttr( wxSystemSettings::GetColour( wxSYS_COLOUR_WINDOWTEXT), bk_col_light_active, g_fntListBold );
	m_DarkAttr			= wxListItemAttr( wxSystemSettings::GetColour( wxSYS_COLOUR_WINDOWTEXT), StringToColour( wxGetApp().Prefs.sPLStripeColour ), wxNullFont );
	m_SelectedDarkAttr	= wxListItemAttr( wxSystemSettings::GetColour( wxSYS_COLOUR_WINDOWTEXT), bk_col_dark_active, g_fntListBold );

	//--- SetItemCount() kinda tells the virtual list control to udpate ---//
	// no Freeze() here , because RescaleColumns(); will not work correctly then
	SetItemCount( ( long )g_Playlist.GetCount() );
	RescaleColumns();

	wxListCtrlSelNone( this );

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
			nStaticWidth += wxGetApp().Prefs.nPlaylistColumnSize[nCurrItem];
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
				nTotalPercent += wxGetApp().Prefs.nPlaylistColumnSize[nCurrItem];
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
			SetColumnWidth( i, wxGetApp().Prefs.nPlaylistColumnSize[nCurrItem] );

		//-------------------------//
		//--- set dynamic size	---//
		//-------------------------//
		else
		{
			n_LastDyn = i;

			if ( wxGetApp().Prefs.bPlaylistSmartColumns == 1 || bAutoFit )
			{
				f_Per = ( (float)wxGetApp().Prefs.nPlaylistColumnSize[nCurrItem] / (float)nTotalPercent ) * nRemainingWidth;
				n_Per = (int)f_Per;
			}

			else
			{
				f_Per = ( (float)wxGetApp().Prefs.nPlaylistColumnSize[nCurrItem] / 100.0f ) * nRemainingWidth;
				n_Per = (int)f_Per;		
			}

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
		wxWindow::Update(); // instantly update window content
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
		InsertColumn( i, wxGetTranslation(g_PlaylistColumnLabels[nCurrType]), g_PlaylistColumnAlign[nCurrType], 50 );
	}

	ShowIcons();

	if ( rescale )
		RescaleColumns();

	if ( update )
		Update();
}

void CPlaylistCtrl::RateSel( int nVal )
{
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
		wxGetApp().Library.SetRating( g_Playlist.Item ( nIndex ).songid, nVal );
		g_Playlist.Item( nIndex ).Rating = nVal;
		RefreshItem( nIndex );
		
	}
	wxGetApp().Library.EndTransaction();
}

void CPlaylistCtrl::EditTag( int i )
{
	int nSelCount = GetSelectedItemCount();
	//--- call the tag dialog, disable this window. ---//
	//---  tag dialog will re-enable when complete  ---//
	if ( nSelCount > 0 )
	{
		MusikTagFrame* pMusikTagFrame = new MusikTagFrame( g_MusikFrame, this, i);
		g_MusikFrame->Enable( FALSE );
		pMusikTagFrame->Show();
	}
}




void CPlaylistCtrl::DelSelSongs(bool bDeleteFromDB, bool bDeleteFromComputer)
{
	int nSelCount = GetSelectedItemCount();
	if( nSelCount == 0)
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
		if(g_Playlist.Item( nIndex - nDeletedSongs).MetaData.eFormat == MUSIK_FORMAT_NETSTREAM)
		{
			if( MUSIK_SOURCES_NOW_PLAYING != g_SourcesCtrl->GetSelType() )
				continue;// net streams cannot be deleted from the playlist, except from now playing list  ( they are deleted in the sources box)
		}
	   //--- if its valid, delete ---//
		if( bDeleteFromDB )
		{
	    	const CMusikSong &song = g_Playlist.Item( nIndex - nDeletedSongs); // get the filename before song is deleted from the array
			if( bDeleteFromComputer )
			{
				if ( !wxRemoveFile( song.MetaData.Filename.GetFullPath() ) )
					sError += song.MetaData.Filename.GetFullPath() + wxT( "\n" );
			}
			wxGetApp().Library.RemoveSong( song.songid );
		}
		g_Playlist.RemoveAt( nIndex - nDeletedSongs, 1 );
		if(bSourceNowPlayingSelected)
		{
			// active playlist is "now playing" list => delete songs from players playlist too
			wxGetApp().Player.RemovePlaylistEntry(nIndex - nDeletedSongs);
		}
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
		g_ActivityAreaCtrl->ResetAllContents();

	
}

void CPlaylistCtrl::RenameSelFiles()
{
	if ( m_ActiveThreadController.IsAlive() == false )
    {
		CMusikSongArray songs;
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
		CMusikSongArray songs;
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

bool CPlaylistCtrl::ViewDirtyTags()
{
	CMusikSongArray dirty;
	wxGetApp().Library.QuerySongsWhere( wxT( "dirty = 1" ), dirty );
	if ( dirty.GetCount() > 0 )
	{

		g_Playlist = dirty;
		g_SourcesCtrl->SelectLibrary(false);  // only change selection, not the view( to protect playlist from being changed. ok that is a hack, but else i would have to much of the structure. this will be done sometime later)
		Update();
		return true;
	}
	else
		wxMessageBox( _( "There are no pending tags available to display." ), MUSIKAPPNAME_VERSION, wxICON_INFORMATION );

	return false;
}

//--------------------------------------//
//--- dnd related playlist functions ---//	//--- please don't call these functions manually, just let dnd do it ---//
//--------------------------------------//
void CPlaylistCtrl::DNDSetCurSel()
{
	//--- find whats currently selected for dnd ---//
	aCurSel.Clear();
	int nIndex = -1;
	if( GetSelectedItemCount() > 0 )
	{
		for( int i = 0; i < GetSelectedItemCount(); i++ )
		{
			nIndex = GetNextItem( nIndex, wxLIST_NEXT_ALL , wxLIST_STATE_SELECTED );
			if ( nIndex == -1 )
				break;
			if(g_Playlist.Item( nIndex ).MetaData.eFormat != MUSIK_FORMAT_NETSTREAM)
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

size_t CPlaylistCtrl::DNDDelSongs()
{
	//--- delete songs we're moving around ---//
	for ( size_t i = 0; i < aCurSel.GetCount(); i++ )
		g_Playlist.RemoveAt( aCurSel.Item( i ) - i, 1 );
	return aCurSel.GetCount();
}
void  CPlaylistCtrl::MovePlaylistEntrys(int nMoveTo ,const wxArrayInt &arrToMove,bool bSelectItems)
{
	// assumes that arrToMove is sorted in ascending order

	wxASSERT(nMoveTo >= 0 && nMoveTo <= (int)g_Playlist.GetCount()); 
	int i = arrToMove.GetCount() - 1;
	// first move all entrys which are behind nMoveTo position
	for(;i >= 0 ; i--)
	{
		if(nMoveTo > arrToMove[i])
			break;
		g_Playlist.Insert(g_Playlist.Detach(arrToMove[i] + ( arrToMove.GetCount() - 1 - i)),nMoveTo);
	}
	// now move all entry which are before
	for(int j = i; j >= 0; j--)
	{
		g_Playlist.Insert(g_Playlist.Detach(arrToMove[j]),nMoveTo - (i - j)-1);
	}
	if(g_SourcesCtrl->GetSelType() == MUSIK_SOURCES_NOW_PLAYING)
	{
		wxGetApp().Player.MovePlaylistEntrys(	nMoveTo , arrToMove );
	}
	if(bSelectItems)
	{
		//--- select new songs ---//
		wxListCtrlSelNone( this );
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
  		g_ActivityAreaCtrl->ResetAllContents();
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

CMusikSongArray * CPlaylistCtrl::GetPlaylist()
{
	return &g_Playlist;
}

void CPlaylistCtrl::OnPlayInstantly( wxCommandEvent& WXUNUSED(event) )
{
	CMusikSongArray aResult;
	GetSelectedSongs(aResult);
	wxGetApp().Player.InsertToPlaylist(aResult);

}
void CPlaylistCtrl::OnPlayAsNext ( wxCommandEvent& WXUNUSED(event) )
{
	CMusikSongArray aResult;
	GetSelectedSongs(aResult);
	wxGetApp().Player.InsertToPlaylist(aResult,wxGetApp().Player.IsPlaying() ? false : true);

}
void CPlaylistCtrl::OnPlayEnqueued	( wxCommandEvent& WXUNUSED(event) )
{
	CMusikSongArray aResult;
	GetSelectedSongs(aResult);
	wxGetApp().Player.AddToPlaylist(aResult,wxGetApp().Player.IsPlaying() ? false : true);
}
void CPlaylistCtrl::OnPlayReplace	( wxCommandEvent& WXUNUSED(event) )
{	
	if ( m_nCurSel > -1 )
	{
		wxGetApp().Player.PlayReplaceList(m_nCurSel,g_Playlist);
	}
}

void CPlaylistCtrl::OnPlayReplaceWithSel( wxCommandEvent& WXUNUSED(event) )
{	
	if ( m_nCurSel > -1 )
	{
		CMusikSongArray aResult;
		GetSelectedSongs(aResult);
		wxGetApp().Player.PlayReplaceList(0,aResult);
	}
}

void CPlaylistCtrl::OnShowInLibrary( wxCommandEvent& event )
{
	int id = event.GetId() - MUSIK_PLAYLIST_CONTEXT_SHOW_IN_LIBRARY_ARTIST;

	CActivityBox * pBox = g_ActivityAreaCtrl->GetActivityBox((EMUSIK_ACTIVITY_TYPE)(MUSIK_LBTYPE_ARTISTS + id));
	wxString sEntry;
	if ( m_nCurSel > -1 && pBox)
	{
		EPLAYLISTCOLUMNS column = PLAYLISTCOLUMN_ARTIST;
		switch(MUSIK_LBTYPE_ARTISTS + id)
		{
		case MUSIK_LBTYPE_ARTISTS:
			column = PLAYLISTCOLUMN_ARTIST;
			break;
		case MUSIK_LBTYPE_ALBUMS:
			column = PLAYLISTCOLUMN_ALBUM;
			break;
		case MUSIK_LBTYPE_GENRES:
			column = PLAYLISTCOLUMN_GENRE;
			break;
		case MUSIK_LBTYPE_YEARS:
			column = PLAYLISTCOLUMN_YEAR;
			break;
		}
		sEntry = GetItemText( m_nCurSel, column);
		g_ActivityAreaCtrl->ResetAllContents();
		g_SourcesCtrl->SelectLibrary();
		pBox->SetFocus();
		pBox->SetSel(sEntry);
	}
}
void CPlaylistCtrl::OnOpenFolderInFileManager( wxCommandEvent&  )
{
	if ( m_nCurSel > -1 && m_nCurSel < (int)g_Playlist.GetCount() )
	{
		wxString sCommand = wxString::Format(wxGetApp().Prefs.sFilemanagerCmd, g_Playlist[m_nCurSel].MetaData.Filename.GetPath().c_str());
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
EVT_BUTTON					(MUSIK_SEARCHBOX_CLEAR,CSearchBox::OnClear)
END_EVENT_TABLE()

CSearchBox::CSearchBox( wxWindow *parent )
:wxPanel( parent, -1, wxPoint( -1, -1 ), wxSize( -1, -1 ),wxNO_FULL_REPAINT_ON_RESIZE| wxTAB_TRAVERSAL|wxCLIP_CHILDREN|wxBORDER_RAISED )
,m_Timer(this,MUSIK_SEARCHBOX_TIMERID)
{
	//--------------------//
	//--- simple query ---//
	//--------------------//
	wxBoxSizer *pSizer = new wxBoxSizer( wxHORIZONTAL );

	wxStaticText *stSearch	= new wxStaticText_NoFlicker( this, -1, _( "Search:" ),wxPoint( -1, -1 ), wxSize( -1, -1 ), wxALIGN_RIGHT | wxTRANSPARENT_WINDOW );
	m_pTextSimpleQuery		= new wxTextCtrl_NoFlicker( this, MUSIK_SEARCHBOX_TEXT, wxT( "" ), wxPoint( -1, -1 ), wxSize( -1, -1 ), wxSIMPLE_BORDER );
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
	m_arrFieldsToSearch.Add(g_PlaylistColumnDBNames[PLAYLISTCOLUMN_ARTIST]);
	m_arrFieldsToSearch.Add(g_PlaylistColumnDBNames[PLAYLISTCOLUMN_ALBUM]);
	m_arrFieldsToSearch.Add(g_PlaylistColumnDBNames[PLAYLISTCOLUMN_TITLE]);
	m_arrFieldsToSearch.Add(g_PlaylistColumnDBNames[PLAYLISTCOLUMN_FILENAME]);
	m_arrFieldsToSearch.Add(g_PlaylistColumnDBNames[PLAYLISTCOLUMN_NOTES]);
	m_arrFieldsToSearch.Add(g_PlaylistColumnDBNames[PLAYLISTCOLUMN_GENRE]);
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

		wxGetApp().Library.QuerySongsWhere( sQuery, g_Playlist ,true);  // true means query sorted
	}
	else
	{
		if ( wxGetApp().Prefs.bShowAllSongs == 1 )
			wxGetApp().Library.GetAllSongs(g_Playlist);
	}
	g_PlaylistBox->Update( );
}
void CSearchBox::OnTextInput(wxCommandEvent &)
{
	m_Timer.Start(650,wxTIMER_ONE_SHOT );	
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
	DoSearchQuery(m_pTextSimpleQuery->GetValue());
}

void CSearchBox::OnClear(wxCommandEvent&)
{
	m_pTextSimpleQuery->SetValue(wxEmptyString);	
}
