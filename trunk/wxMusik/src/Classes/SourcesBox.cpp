/*
 *  SourcesBox.cpp
 *
 *  The "Sources Box" control
 *	This is the control visible on the left side of the main frame.
 *	It's main function is playlist management.
 *
 *  Copyright (c) 2003 Casey Langen (casey@bak.rr.com)
 *	Contributors: Simon Windmill, Dustin Carter, Gunnar Roth, Wade Brainerd
 *
 *  See the file "license.txt" for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL WARRANTIES.
*/

//--- For compilers that support precompilation, includes "wx/wx.h". ---//
#include "myprec.h"

#include "Classes/SourcesBox.h"
#include "PictureBox.h"
#include "Classes/PlaylistCtrl.h" //TODO: remove the dependancy
#include "Classes/ActivityAreaCtrl.h" //TODO: remove the dependancy
//--- globals ---//
#include "MusikGlobals.h"
#include "MusikUtils.h"

//--- frames ---//
#include "Frames/MusikFrame.h"

//--- file ops ---//
#include <wx/textfile.h>

//other
#include "DataObjectCompositeEx.h"
#include "DNDHelper.h"



//-------------------------//
//--- SourcesDropTarget ---//
//-------------------------//
class SourcesDropTarget : public wxDropTarget
{
public:
	SourcesDropTarget( CSourcesListBox *pBox )	
	{ 
		m_SourcesListBox = pBox; 
		wxDataObjectCompositeEx * dobj = new wxDataObjectCompositeEx;
		dobj->Add(m_pSourcesDObj = new CMusikSourcesDataObject(),true);
		dobj->Add(m_pSonglistDObj = new CMusikSonglistDataObject());
		SetDataObject(dobj);
	}
	virtual wxDragResult OnData(wxCoord x, wxCoord y, wxDragResult def);

	virtual bool			OnDropSources(wxCoord x, wxCoord y, const wxString& text);
	virtual bool			OnDropSonglist(wxCoord x, wxCoord y, const wxString& sSongIds);
	virtual wxDragResult	OnDragOver(wxCoord x, wxCoord y, wxDragResult def);
	bool HighlightSel( const  wxPoint & pPos );

private:
	CSourcesListBox* m_SourcesListBox;
	int nLastHit;
	CMusikSourcesDataObject * m_pSourcesDObj;
	CMusikSonglistDataObject * m_pSonglistDObj;

};

wxDragResult SourcesDropTarget::OnData(wxCoord x, wxCoord y, wxDragResult def)
{
	bool bRes = false;
	if (GetData() )
	{
		wxDataObjectSimple *dobj = ((wxDataObjectCompositeEx *)GetDataObject())->GetActualDataObject();

		if( dobj == (wxDataObjectSimple *)m_pSourcesDObj )
		{
			bRes = OnDropSources(x, y, m_pSourcesDObj->GetText());
		}
		else if( dobj == (wxDataObjectSimple *)m_pSonglistDObj )
		{
			bRes = OnDropSonglist(x, y, m_pSonglistDObj->GetText());
		}
	}
	return bRes ? def : wxDragNone;
}

bool SourcesDropTarget::OnDropSources( wxCoord x, wxCoord y, const wxString &sSource )
{	
	long n = -1;
	//--- where did we land? ---//
	const wxPoint& pt = wxPoint( x, y );
	int nHitFlags = 0;
	n = m_SourcesListBox->HitTest( pt, nHitFlags );
	if ( n == m_SourcesListBox->GetDragIndex() )
		return false;
	if ( m_SourcesListBox->GetType( n ) == MUSIK_SOURCES_NOW_PLAYING ) 
	{
		return m_SourcesListBox->AddSourceContentToNowPlaying(m_SourcesListBox->GetDragIndex());
	}
	m_SourcesListBox->Freeze();
	// remove from old position
	m_SourcesListBox->m_SourcesList.RemoveAt	( m_SourcesListBox->GetDragIndex() );

	//--- if its -1, we will just push it to the bottom ---//
	if ( nHitFlags == wxLIST_HITTEST_NOWHERE )
		m_SourcesListBox->m_SourcesList.Add( sSource );

	//--- dragged above old pos, insert, push down... old item is + 1 ---//
	else if (n >= 0 && n < m_SourcesListBox->GetDragIndex() )		
		m_SourcesListBox->m_SourcesList.Insert	( sSource, n );

	//--- dragged below old pos, insert there, then push all others down ---//
	else if ( n > m_SourcesListBox->GetDragIndex() )	
		m_SourcesListBox->m_SourcesList.Insert	( sSource, n - 1);
	//--- update ---//
	m_SourcesListBox->Thaw();

	return true;

}
bool SourcesDropTarget::OnDropSonglist( wxCoord x, wxCoord y, const wxString &sSongIds )
{
	if ( !sSongIds.IsEmpty() )
	{
		wxArrayString aSongIdList;
		DelimitStr(sSongIds,wxT("\n"),aSongIdList);
		MusikSongIdArray arrSongIds(aSongIdList);
		//--- where did we land? ---//
		const wxPoint& pt = wxPoint( x, y );
		int nFlags;
		long n = m_SourcesListBox->HitTest( pt, nFlags );
	
		//--- drag not over an object, create new list ---//
		if ( n == -1 )
		{
			wxTextEntryDialog dlg( g_MusikFrame, _( "Enter name for new playlist:" ), MUSIKAPPNAME_VERSION, wxT( "" ) );
			if ( dlg.ShowModal() == wxID_OK )
			{
				wxString sName = dlg.GetValue();
				m_SourcesListBox->NewPlaylist( sName, arrSongIds, MUSIK_SOURCES_PLAYLIST_STANDARD );		
			}
		}

		//--- drag over library, can't do that ---//
		else if ( m_SourcesListBox->GetType( n ) == MUSIK_SOURCES_LIBRARY )
		{
			wxMessageBox( _( "Cannot drag songs into the library, they already exist here." ), MUSIKAPPNAME_VERSION, wxOK | wxICON_INFORMATION );
			return false;
		}
		else if ( m_SourcesListBox->GetType( n ) == MUSIK_SOURCES_NOW_PLAYING )
		{
			wxGetApp().Player.AddToPlaylist(arrSongIds,false);
			
		}
		else if ( m_SourcesListBox->GetType( n ) == MUSIK_SOURCES_NETSTREAM )
		{
			wxMessageBox( _( "Cannot drag songs into a net stream." ), MUSIKAPPNAME_VERSION, wxOK | wxICON_INFORMATION );
			return false;
		}
		else if ( m_SourcesListBox->GetType( n ) == MUSIK_SOURCES_PLAYLIST_DYNAMIC)
		{
			wxMessageBox( _( "Cannot drag songs into a dynamic playlist." ), MUSIKAPPNAME_VERSION, wxOK | wxICON_INFORMATION );
			return false;
		}

		//--- drag over an existing item, append ---//
		else if ( n > 0 )
		{
			wxString sName = m_SourcesListBox->GetItemText( n );
			m_SourcesListBox->PlaylistToFile( sName, arrSongIds,MUSIK_SOURCES_PLAYLIST_STANDARD,false );
		}
		
	}
	return true;
}

wxDragResult SourcesDropTarget::OnDragOver(wxCoord x, wxCoord y, wxDragResult def)
{
	if(def == wxDragNone)
		return wxDragNone;

	const wxPoint& pt = wxPoint( x, y );
	return HighlightSel( pt )? wxDragCopy : wxDragNone;// return wxDragCopy because def is not set correctly by wxwidgets(it should be the operation which is proposed by windows, but wxwidgets uses the keystate for determining the drop effect)
	// wxDragCopy because CPlaylistCtrl uses wxDrag_CopyOnly( to prohibit moving files by explorer when files are dropped there.)
}

bool SourcesDropTarget::HighlightSel( const wxPoint &pPos )
{
	int nFlags;
	long n = m_SourcesListBox->HitTest( pPos, nFlags );
#ifndef __WXMAC__
	m_SourcesListBox->SetFocus();
#endif	
	EMUSIK_SOURCES_TYPE Type = m_SourcesListBox->GetType(n);
#ifndef __WXMAC__	
	if(n != m_SourcesListBox->m_CurSel)
		m_SourcesListBox->SetItemState( m_SourcesListBox->m_CurSel, 0, wxLIST_STATE_FOCUSED );
#endif		
	long topitem = m_SourcesListBox->GetTopItem();

	long countperpage = m_SourcesListBox->GetCountPerPage();
	if( n == topitem && n > 0)
		m_SourcesListBox->EnsureVisible(n - 1);
	else if((n == topitem + countperpage - 1) &&  (n < m_SourcesListBox->GetItemCount() - 1))
		m_SourcesListBox->EnsureVisible(n + 1);
	if(m_SourcesListBox->GetDragIndex() == -1)
	{// NOT dragging from source to source
		if(Type == MUSIK_SOURCES_NONE)
		{
			return true;// drag over non, means create new playlist from drag data
		}
		else if((Type != MUSIK_SOURCES_NOW_PLAYING) && (Type != MUSIK_SOURCES_PLAYLIST_STANDARD) )
		{
			return false;
		}
	}
	else if(Type == MUSIK_SOURCES_LIBRARY)
	{
		// do not move library source entry
		return false;
	}
#ifndef __WXMAC__	
	if ( n != nLastHit )
	{
		m_SourcesListBox->SetItemState( n, wxLIST_STATE_FOCUSED, wxLIST_STATE_FOCUSED );
	}
#endif	
	nLastHit = n;
	return true;
}

//-----------------------//
//--- CSourcesListBox ---//
//-----------------------//
BEGIN_EVENT_TABLE(CSourcesListBox, CMusikListCtrl)
	EVT_MENU(MUSIK_PLAYLIST_CLEARPLAYERLIST,				CSourcesListBox::OnClearPlayerlist	)	// Sources Context -> Create -> Create from Current
	EVT_MENU(MUSIK_SOURCE_CONTEXT_CREATE_CURRENT_PLAYLIST,	CSourcesListBox::CreateCurPlaylist	)	// Sources Context -> Create -> Create from Current
	EVT_MENU(MUSIK_SOURCE_CONTEXT_STANDARD_PLAYLIST,		CSourcesListBox::StandardPlaylist	)	// Sources Context -> Create -> Standard Playlist
	EVT_MENU(MUSIK_SOURCE_CONTEXT_DYNAMIC_PLAYLIST,			CSourcesListBox::DynamicPlaylist	)	// Sources Context -> Create -> Dynamic Playlist
	EVT_MENU(MUSIK_SOURCE_CONTEXT_CREATE_NETSTREAM,			CSourcesListBox::NetStream			)	// Sources Context -> Create -> Net Stream
	EVT_MENU(MUSIK_SOURCE_CONTEXT_EDIT_QUERY,				CSourcesListBox::EditQuery			)	// Sources Context -> Edit Query
	EVT_MENU(MUSIK_SOURCE_CONTEXT_EDIT_URL,					CSourcesListBox::EditURL			)	// Sources Context -> Edit Query
	EVT_MENU(MUSIK_SOURCE_CONTEXT_DELETE,					CSourcesListBox::Delete				)	// Sources Context -> Delete
	EVT_MENU(MUSIK_SOURCE_CONTEXT_RENAME,					CSourcesListBox::Rename				)	// Sources Context -> Rename
	EVT_MENU(MUSIK_SOURCE_CONTEXT_SHOW_ICONS,				CSourcesListBox::ToggleIconsEvt		)	// Sources Context -> Show Icons
	EVT_MENU(MUSIK_SOURCE_CONTEXT_COPY_FILES,				CSourcesListBox::CopyFiles			)	// Sources Context -> Copy files
    EVT_MENU(MUSIK_SOURCE_CONTEXT_EXPORT_PLAYLIST,			CSourcesListBox::ExportPlaylist		)	// Sources Context -> Export m3u
	EVT_LIST_BEGIN_DRAG			(wxID_ANY,	CSourcesListBox::BeginDrag				)	// user drags files from sources
	EVT_LIST_BEGIN_LABEL_EDIT	(wxID_ANY,	CSourcesListBox::BeginEditLabel			)   // user edits a playlist filename
	EVT_LIST_END_LABEL_EDIT		(wxID_ANY,	CSourcesListBox::EndEditLabel			)   // user edits a playlist filename
	EVT_LIST_KEY_DOWN			(wxID_ANY,	CSourcesListBox::TranslateKeys			)	// user presses a key in the sources list
	EVT_LIST_COL_BEGIN_DRAG		(wxID_ANY,	CSourcesListBox::OnSourcesColSize		)
    EVT_LISTSEL_CHANGED_COMMAND (wxID_ANY,	CSourcesListBox::OnUpdateSel			)    
END_EVENT_TABLE()

CSourcesListBox::CSourcesListBox( wxWindow* parent )
	: CMusikListCtrl( parent, MUSIK_SOURCES_LISTCTRL, wxPoint( -1, -1 ), wxSize( -1, -1 ), wxLC_ALIGN_LEFT |wxLC_EDIT_LABELS | wxLC_SINGLE_SEL | wxNO_BORDER|wxLC_NO_SORT_HEADER)
{
#ifdef __WXMSW__
    m_bHideHorzScrollbar = true;
#endif
	//--- initialize variables ---//
	m_CurSel = 0;
	m_DragIndex	= -1;
	

	InsertColumn( 0, _( "Sources" ), wxLIST_FORMAT_LEFT );
	SetDropTarget( new SourcesDropTarget( this ) );

	m_Deleting = false;

	ShowIcons();

	Load();
	RescanPlaylistDir();
	Update();
}

CSourcesListBox::~CSourcesListBox()
{
	Save();
}

wxMenu * CSourcesListBox::CreateContextMenu()
{
	int nSelIndex = GetNextItem( -1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );

	//--- setup context menus ---//
	wxMenu *context_menu = new wxMenu;
	
	//---------------------------------------------------------//
	//--- if a dynamic playlist is selected, it can have	---//
	//--- its query edited. other playlists cannot.			---//
	//---------------------------------------------------------//
	if(nSelIndex != -1 && GetSelType() == MUSIK_SOURCES_PLAYLIST_DYNAMIC)
		context_menu->Append( MUSIK_SOURCE_CONTEXT_EDIT_QUERY, _( "&Edit Query" ) );
	else if((nSelIndex != -1) && (GetSelType() == MUSIK_SOURCES_NETSTREAM))
	{
		context_menu->Append( MUSIK_SOURCE_CONTEXT_EDIT_URL, _( "&Edit URL" ) );
	}
	else if(GetSelType() == MUSIK_SOURCES_NOW_PLAYING)
	{
		context_menu->Append( MUSIK_PLAYLIST_CLEARPLAYERLIST,			_( "&Clear List" ),					wxT("") );
	}

	//---------------------------------------------------------//
	//--- if  library or now playing entry is selected, it can't be deleted	or renamed	---//
	//---------------------------------------------------------//
	if((nSelIndex != -1 )&& (GetSelType() != MUSIK_SOURCES_LIBRARY) && (GetSelType() != MUSIK_SOURCES_NOW_PLAYING))
	{
		context_menu->Append( MUSIK_SOURCE_CONTEXT_RENAME, _( "&Rename" ) );
		context_menu->Append( MUSIK_SOURCE_CONTEXT_DELETE, _( "&Delete" ) );
	}
	if(context_menu->GetMenuItemCount())
		context_menu->AppendSeparator();

	wxMenu * submenu_new = new wxMenu;
	submenu_new->Append( MUSIK_SOURCE_CONTEXT_CREATE_CURRENT_PLAYLIST, _( "Playlist from Current" ) );
	submenu_new->Enable(MUSIK_SOURCE_CONTEXT_CREATE_CURRENT_PLAYLIST,g_PlaylistBox->PlaylistCtrl().GetCount() > 0);
	submenu_new->AppendSeparator();
	submenu_new->Append( MUSIK_SOURCE_CONTEXT_STANDARD_PLAYLIST, _( "Standard Playlist" ) );
	submenu_new->Append( MUSIK_SOURCE_CONTEXT_DYNAMIC_PLAYLIST, _( "Dynamic Playlist" ) );
	submenu_new->Append( MUSIK_SOURCE_CONTEXT_CREATE_NETSTREAM, _( "Net Stream" ) );

	context_menu->Append( MUSIK_SOURCE_CONTEXT_CREATE, _( "&Create New" ), submenu_new );

	context_menu->AppendSeparator();

	context_menu->Append( MUSIK_SOURCE_CONTEXT_SHOW_ICONS, _( "&Show Icons" ), wxT( "" ), wxITEM_CHECK );
	context_menu->Check( MUSIK_SOURCE_CONTEXT_SHOW_ICONS, ( bool )wxGetApp().Prefs.bShowSourcesIcons );
	if((nSelIndex != -1 ) &&  (GetSelType() != MUSIK_SOURCES_NETSTREAM))
	{
		context_menu->AppendSeparator();
		context_menu->Append( MUSIK_SOURCE_CONTEXT_COPY_FILES, _("Copy files to directory") );
		context_menu->Enable( MUSIK_SOURCE_CONTEXT_COPY_FILES,g_PlaylistBox->PlaylistCtrl().GetCount() > 0);
		context_menu->Append( MUSIK_SOURCE_CONTEXT_EXPORT_PLAYLIST, _("Export Playlist (M3U)") );
	}
	return context_menu;

}

void CSourcesListBox::OnClearPlayerlist( wxCommandEvent& event )
{
	g_PlaylistBox->PlaylistCtrl().OnClearPlayerlist(event);
}

void CSourcesListBox::CreateCurPlaylist( wxCommandEvent& WXUNUSED(event) )
{
	wxTextEntryDialog dlg( this, _( "Enter name for new playlist:" ), MUSIKAPPNAME_VERSION, wxT( "" ) );
	if ( dlg.ShowModal() == wxID_OK )
	{
		wxString sName = dlg.GetValue();
		NewPlaylist( sName, g_PlaylistBox->PlaylistCtrl().Playlist(), MUSIK_SOURCES_PLAYLIST_STANDARD );
	}
}

void CSourcesListBox::StandardPlaylist( wxCommandEvent& WXUNUSED(event) )
{
	wxTextEntryDialog dlg( this, _( "Enter name for new playlist:" ), MUSIKAPPNAME_VERSION, wxT( "" ) );
	if ( dlg.ShowModal() == wxID_OK )
	{
		wxString sName = dlg.GetValue();
		NewPlaylist( sName, MusikSongIdArray(), MUSIK_SOURCES_PLAYLIST_STANDARD );
	}
}

void CSourcesListBox::DynamicPlaylist( wxCommandEvent& WXUNUSED(event) )
{
	wxTextEntryDialog dlg( this, _( "Enter name for new dynamic playlist:" ), MUSIKAPPNAME_VERSION, wxT( "" ) );
	if ( dlg.ShowModal() == wxID_OK )
	{
		wxString sName = dlg.GetValue();
		NewPlaylist( sName,MusikSongIdArray(), MUSIK_SOURCES_PLAYLIST_DYNAMIC );
	}
}
void CSourcesListBox::NetStream( wxCommandEvent& WXUNUSED(event) )
{
	wxTextEntryDialog dlg( this, _( "Enter name of the net stream:" ), MUSIKAPPNAME_VERSION, wxT( "" ) );
	if ( dlg.ShowModal() == wxID_OK )
	{
		wxString sName = dlg.GetValue();
		NewPlaylist( sName, MusikSongIdArray(), MUSIK_SOURCES_NETSTREAM );
	}
}
void CSourcesListBox::EditQuery( wxCommandEvent& WXUNUSED(event) )
{
	int nSelPos = GetNextItem( -1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );
	UpdateDynPlaylist( nSelPos );
}

void CSourcesListBox::EditURL( wxCommandEvent& WXUNUSED(event) )
{
	int nSelPos = GetNextItem( -1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );
	UpdateNetStream( nSelPos );
}

void CSourcesListBox::ToggleIconsEvt( wxCommandEvent& WXUNUSED(event) )
{
	ToggleIcons();
}

/*
What could be improved here:
-Option to prepend a numerical value to the destination filename to maintain 
 the same order as the playlist
-Option to create a directory in the destination directory based on playlist name

SiW
*/
void CSourcesListBox::CopyFiles( wxCommandEvent& WXUNUSED(event) )
{
	wxGetApp().CopyFiles(g_PlaylistBox->PlaylistCtrl().Playlist());
}


void CSourcesListBox::ExportPlaylist (wxCommandEvent& WXUNUSED(event) )
{
	//get the current list of files,
	//build a playlist
	//offer option to save it.
	wxFileDialog fdlg (g_MusikFrame, _("Select the path and filename for your playlist"),
						wxT(""), wxT(""), wxT("Winamp Playlist (.m3u)|*.m3u"), 
						wxSAVE);
	if ( fdlg.ShowModal() != wxID_OK )
		return;
	
	//Test for file exist
	wxTextFile f(fdlg.GetPath());
	if (f.Exists())
	{	
		if ( wxMessageBox( _( "A playlist with this name already exists, would you like to replace it?" ), MUSIKAPPNAME_VERSION, wxYES_NO | wxICON_QUESTION ) == wxNO )
				return;
	}
	f.Create();
	const MusikSongIdArray &songs = g_PlaylistBox->PlaylistCtrl().Playlist();

	for (size_t n =0; n< songs.GetCount();n++)
	{
		const wxFileName &name = songs[n].Song()->MetaData.Filename;
		f.AddLine(name.GetFullPath());
	}
	f.Write();
	

}
void CSourcesListBox::BeginDrag( wxListEvent &WXUNUSED(event) )
{
    long n = m_CurSel;
	if ( n != -1 )
	{
		//-------------------------//
		//--- get selected item	---//
		//-------------------------//
		
		EMUSIK_SOURCES_TYPE Type = GetType(n);
		if(( Type == MUSIK_SOURCES_NONE)
			|| (Type == MUSIK_SOURCES_NOW_PLAYING)
			|| (Type == MUSIK_SOURCES_LIBRARY))
		{
			return;//not allowed to be dragged.
		}
		m_DragIndex = n;
		const wxString & sDrop	= m_SourcesList.Item( m_DragIndex );

		//------------------------------------------------------//
		//--- initialize drag and drop                       ---//
		//--- SourcesDropTarget should take care of the rest ---//
		//------------------------------------------------------//
		wxDropSource dragSource( this );
		CMusikSourcesDataObject sources_data( sDrop );
		dragSource.SetData( sources_data );
		dragSource.DoDragDrop( TRUE );

		Update();
		m_DragIndex		= -1;
	}
}


void CSourcesListBox::OnUpdateSel( wxCommandEvent& event )
{
    event.Skip();
    wxTheApp->Yield(true);   // call yield to let the SetItemState changes go through the system.
    UpdateSel( event.GetInt() );	
}

void CSourcesListBox::UpdateSel( size_t index )
{

	///HACK///
	static bool bInFunction = false;
	if(	bInFunction )
		return;
	bInFunction = true;
    wxBusyCursor  busycursor;
	if((index == (size_t)-2) || (index == (size_t)-4))
	{	// IF -2, this is used to protect playlists from being accidently changed
		// if -4 we actually want to change ciew(not only selection)

		bInFunction = (index == (size_t)-4); // HACK!!,
        m_CurSel = FindInSources(wxT( "" ),MUSIK_SOURCES_LIBRARY);
		SetItemState( m_CurSel, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED );
		bInFunction = false;
		return;
    }
	if(index == (size_t)-3)
	{	//select now playing
		bInFunction = false;// HACK!!, this leads to recursive execution of this function (	call of SetItemState() will generate an event)
        int sel = FindInSources(wxT( "Now Playing" ),MUSIK_SOURCES_NOW_PLAYING);
		SetItemState( sel, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED );
		return;
    }

	//--- save std playlist. if deleting, no need to worry ---//
	int nLastSel = m_CurSel;
	if ( !m_Deleting )
	{
		if ( GetType( nLastSel ) == MUSIK_SOURCES_PLAYLIST_STANDARD )
			RewriteStdPlaylist();
	}
	
	m_CurSel = index;
	int nSelType = GetSelType();

	if ( m_CurSel == -1 )
	{
		SetItemState( nLastSel, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED );
		m_CurSel = nLastSel;
	}

	//--- library selected ---//
	else if ( nSelType == MUSIK_SOURCES_LIBRARY )
	{
		g_ActivityAreaCtrl->UpdateSel( g_ActivityAreaCtrl->GetParentBox() );
		g_MusikFrame->ShowActivityArea( wxGetApp().Prefs.bShowActivities );
		g_PlaylistBox->ShowSearchBox(true);
	}

	else 
	{
		g_MusikFrame->ShowActivityArea( false );
		g_PlaylistBox->ShowSearchBox(false);
		//--- standard playlist selected ---//
		if (m_CurSel != -1 && nSelType == MUSIK_SOURCES_PLAYLIST_STANDARD )
		{
			LoadStdPlaylist( GetItemText( m_CurSel ), g_thePlaylist );
		}

		//--- dynamic playlist selected ---//
		else if (  m_CurSel != -1 && nSelType == MUSIK_SOURCES_PLAYLIST_DYNAMIC )
		{
			wxString sQuery = LoadDynPlaylist( GetItemText( m_CurSel ) );
			RealizeDynPlaylist(sQuery,g_thePlaylist);
		}
		else if ( m_CurSel != -1 && nSelType == MUSIK_SOURCES_NETSTREAM )
		{
			CMusikSong *pSong = new CMusikSong();
			LoadNetStream(GetItemText( m_CurSel ), *pSong);
			g_thePlaylist.Clear();
			g_thePlaylist.Add(MusikSongId(pSong));
		}
		else if ( m_CurSel != -1 && nSelType == MUSIK_SOURCES_NOW_PLAYING )
		{
			g_PlaylistBox->SetPlaylist( &wxGetApp().Player.GetPlaylist());
			g_PlaylistBox->PlaylistCtrl().EnsureVisible(wxGetApp().Player.GetCurIndex());
			bInFunction = false;
			return;
		}
		//--- update ui with new list ---//
		g_PlaylistBox->SetPlaylist(&g_thePlaylist);
	}
	bInFunction = false;
}

void CSourcesListBox::BeginEditLabel( wxListEvent& event )
{
	EMUSIK_SOURCES_TYPE nType = GetType( event.GetIndex() );
	//--- Musik Library entry edited ---//
	if ( nType == MUSIK_SOURCES_LIBRARY )
	{
 		event.Veto();
	}
	//--- Now Playing entry edited ---//
	else if ( nType == MUSIK_SOURCES_NOW_PLAYING )
	{
		event.Veto();   
	}
}

void CSourcesListBox::EndEditLabel( wxListEvent& event )
{
#if wxVERSION_NUMBER >= 2500
	if(event.IsEditCancelled())
		return;
#endif
    wxString sCheck = event.GetText();
	sCheck.Replace( wxT( " " ), wxT( "" ), TRUE );
	if ( sCheck.IsEmpty() )
		return; // do not want to rename to an empty string ( or one that only consists of spaces

	EMUSIK_SOURCES_TYPE nType = GetType( event.GetIndex() );

	wxString sType;
	if(!GetTypeAsString(nType,sType))
	{
		wxASSERT(FALSE);
		return;
	}
	//--- Musik Library entry edited ---//
	if ( nType == MUSIK_SOURCES_LIBRARY )
	{
 		
	}
	//--- Now Playing entry edited ---//
	else if ( nType == MUSIK_SOURCES_NOW_PLAYING )
	{
        
	}
	//--- "playlist with data in a file" renamed ---//
	else
	{
		//--- rename file ---//
		wxString sOldFile = OnGetItemText(event.GetIndex(),0);
		wxString sNewFile = event.GetText();
		
		SourcesToFilename( &sOldFile, nType );
		SourcesToFilename( &sNewFile, nType );

		wxRenameFile( sOldFile, sNewFile );
	}
	//--- rename in musiksources.dat ---//
	m_SourcesList.Item( event.GetIndex() ) = sType + event.GetText();
}

void CSourcesListBox::TranslateKeys( wxListEvent& event )
{
	if ( event.GetKeyCode() == WXK_DELETE || event.GetKeyCode() == WXK_BACK )
		DelSel();
	else if ( event.GetKeyCode() == WXK_F2 )
		RenameSel();
	else
		event.Skip();
}

void CSourcesListBox::Create()
{
	wxTextFile Out( MUSIK_SOURCES_FILENAME );

	Out.Create();
/*	if ( Out.Open() )
	{
		Out.AddLine( wxT( "[l] Musik Library" ) );
		Out.Write();
		Out.Close();
	}
*/
}

void CSourcesListBox::Load()
{
	if ( !wxFileExists( MUSIK_SOURCES_FILENAME ) )
		Create();

	m_SourcesList = FileToStringArray( MUSIK_SOURCES_FILENAME );

	size_t i = 0;
	for(i = 0; i < m_SourcesList.GetCount();i++)
	{
		if(m_SourcesList[i].Left(3) == wxT( "[l]" ))
			break;
	}
	if((m_SourcesList.GetCount() == i) )
	{// no [l] found
		m_SourcesList.Insert(wxT( "[l] Musik Library"), 0);
	}
	for(i = 0; i < m_SourcesList.GetCount();i++)
	{
		if(m_SourcesList[i].Left(3) == wxT( "[n]" ))
			break;
	}
	if((m_SourcesList.GetCount() == i) )
	{// no [n] found
		m_SourcesList.Insert(wxT( "[n] Now Playing"),1);
	}

}

void CSourcesListBox::Save()
{
	// SiW - removed this because it was causing the loss of playlists
	// when exiting.. any side effects?
	//if ( GetType( m_CurSel ) == MUSIK_SOURCES_PLAYLIST_STANDARD )
	//		RewriteStdPlaylist();

	if ( wxFileExists( MUSIK_SOURCES_FILENAME ) )
		wxRemoveFile( MUSIK_SOURCES_FILENAME );
	
	wxTextFile Out( MUSIK_SOURCES_FILENAME );
	Out.Create();
	if ( Out.Open() )
	{
		for ( size_t i = 0; i < m_SourcesList.GetCount(); i++ )
			Out.AddLine( m_SourcesList.Item( i ) );
		Out.Write( Out.GuessType() );
		Out.Close();
	}
}

//---------------------------------------------------//
//--- virtual functions. careful with these ones, ---// 
//---        boys. they drive everything          ---//
//---------------------------------------------------//
wxString CSourcesListBox::OnGetItemText(long item, long column) const
{
	switch ( column )
	{
	case 0:
		return GetSourceEntry(item);
		break;
	}

	return wxT("");
}

wxListItemAttr* CSourcesListBox::OnGetItemAttr(long item) const
{
#ifdef __WXMAC__
	wxListItemAttr *pDefAttr = 	NULL;
#else
	wxListItemAttr *pDefAttr = 	(wxListItemAttr*)&m_Light;
#endif
	wxString type = m_SourcesList.Item( item ).Left( 3 );
	type.MakeLower();

	//--- stripes ---//
	if ( wxGetApp().Prefs.bSourcesStripes == 1 )
	{
		//--- even ---//
		if ( item % 2 == 0 )
		{
			if ( type == wxT( "[l]" ) )
				return ( wxListItemAttr* )&m_LightBold;
			else
				return pDefAttr;
		}
		else
		{
			if ( type == wxT( "[l]" ) )
				return ( wxListItemAttr* )&m_DarkBold;
			else
				return ( wxListItemAttr* )&m_Dark;
		}
	}

	//--- no stripes ---//
	if ( type == wxT( "[l]" ) )
		return ( wxListItemAttr* )&m_LightBold;

	return pDefAttr;
}

int	CSourcesListBox::OnGetItemImage	(long item) const
{

	return GetType(item);	
}


wxString CSourcesListBox::GetSourceEntry(long i) const
{
	switch(i)
	{
	case 0:
		return _("Musik Library");
	case 1:
		return _("Now Playing");
		
	default:
		{
			wxString sItem = m_SourcesList.Item( i ).Right( m_SourcesList.Item( i ).Length() - 3 );
			sItem.Trim();sItem.Trim(false);
			return sItem;
		}
	}
}

void CSourcesListBox::Update()
{
	//----------------------------------------------------------------------------------//
	//---        note that the sources box control is now virtual, so we don't		 ---//
	//---    add items directly to it.  Instead, we have the object pItems a		 ---//
	//---  *wxArrayString, that the virtual listctrl then references via callbacks   ---//
	//----------------------------------------------------------------------------------//

	m_Light		= wxListItemAttr( wxSystemSettings::GetColour( wxSYS_COLOUR_WINDOWTEXT), wxSystemSettings::GetColour( wxSYS_COLOUR_BTNHIGHLIGHT ), wxNullFont	);
//	m_LightBold	= wxListItemAttr( wxSystemSettings::GetColour( wxSYS_COLOUR_WINDOWTEXT), wxSystemSettings::GetColour( wxSYS_COLOUR_BTNHIGHLIGHT ), g_fntListBold		);
	m_LightBold	= wxListItemAttr( GetForegroundColour(), GetBackgroundColour(), g_fntListBold		);

	m_Dark		= wxListItemAttr( wxSystemSettings::GetColour( wxSYS_COLOUR_WINDOWTEXT), StringToColour( wxGetApp().Prefs.sSourcesStripeColour ), wxNullFont );
	m_DarkBold	= wxListItemAttr( wxSystemSettings::GetColour( wxSYS_COLOUR_WINDOWTEXT), StringToColour( wxGetApp().Prefs.sSourcesStripeColour ), g_fntListBold );

	SetItemCount( m_SourcesList.GetCount() );
	RescaleColumns();
	Refresh();

}


void CSourcesListBox::DelSel()
{
	//-----------------------------------------------------//
	//--- the EVT_LIST_ITEM_SELECTED will check this	---//
	//--- as there is no need to update the sel until	---//
	//--- we're done									---//
	//-----------------------------------------------------//
    SuppressListItemStateEventsWrapper(*this);
    m_Deleting = true;

	//--- where are we selected? ---//
	int nIndex = - 1;
	nIndex = GetNextItem( nIndex, wxLIST_NEXT_ALL , wxLIST_STATE_SELECTED );

	int nType = GetType( nIndex );

	//--- no item selected? bail ---//
	if ( nIndex == - 1 || nType == MUSIK_SOURCES_LIBRARY || nType == MUSIK_SOURCES_NOW_PLAYING )
		return;

	//--- if we have a standard or dynamic playlist, delete the file ---//
	if ( nType == MUSIK_SOURCES_PLAYLIST_STANDARD || nType == MUSIK_SOURCES_PLAYLIST_DYNAMIC || MUSIK_SOURCES_NETSTREAM)
	{
		wxString sFilename = GetItemText( nIndex );
		SourcesToFilename( &sFilename, nType );

		if ( wxFileExists( sFilename ) )
			wxRemoveFile( sFilename );
	}
	
	//--- remove item from list ---//
	m_SourcesList.RemoveAt( nIndex );
	Update();


	int nNextSel = nIndex;
	if ( nNextSel != 0 )
		nNextSel -= 1;

	SetItemState( nNextSel, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED );
	UpdateSel( nNextSel );

	m_Deleting = false;
}

void CSourcesListBox::RenameSel()
{
	int nIndex = -1;
	nIndex = GetNextItem( nIndex, wxLIST_NEXT_ALL , wxLIST_STATE_SELECTED );
	if ( nIndex > -1 )
		EditLabel( nIndex );
}

EMUSIK_SOURCES_TYPE CSourcesListBox::GetType(long index) const 
{
	if( index < 0)
		return MUSIK_SOURCES_NONE;
	wxString sType = m_SourcesList.Item( index ).Left( 3 );
	
	if ( sType == wxT( "[l]" ) )
		return MUSIK_SOURCES_LIBRARY;
	else if ( sType == wxT( "[s]" ) )
		return MUSIK_SOURCES_PLAYLIST_STANDARD;
	else if ( sType == wxT( "[d]" ) )
		return MUSIK_SOURCES_PLAYLIST_DYNAMIC;
	else if ( sType == wxT( "[c]" ) )
		return MUSIK_SOURCES_CDROM_DRIVE;
	else if ( sType == wxT( "[n]" ) )
		return MUSIK_SOURCES_NOW_PLAYING;
	else if ( sType == wxT( "[u]" ) )
		return MUSIK_SOURCES_NETSTREAM;
	
	return MUSIK_SOURCES_NONE;
}

int CSourcesListBox::GetItemImage( long index )
{
	wxListItem item;
	item.SetId( index );
	item.SetMask( wxLIST_MASK_IMAGE );
	GetItem( item );
	return item.GetImage();
}

void CSourcesListBox::RescaleColumns()
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

void CSourcesListBox::RescanPlaylistDir()
{
	wxArrayString playlists;
	GetPlaylistDir( playlists );

	if ( playlists.GetCount() > 0 )
	{
		AddMissing( playlists ,MUSIK_SOURCES_PLAYLIST_STANDARD);
		AddMissing( playlists ,MUSIK_SOURCES_PLAYLIST_DYNAMIC);
		AddMissing( playlists ,MUSIK_SOURCES_NETSTREAM);
	}
}

void CSourcesListBox::ShowIcons()
{
	if ( wxGetApp().Prefs.bShowSourcesIcons == 0 )
		SetImageList( g_NullImageList, wxIMAGE_LIST_SMALL );
	else
		SetImageList( g_SourcesImages, wxIMAGE_LIST_SMALL );
}


void CSourcesListBox::ToggleIcons()
{
	if ( wxGetApp().Prefs.bShowSourcesIcons == 0 )
		wxGetApp().Prefs.bShowSourcesIcons = 1;
	else
		wxGetApp().Prefs.bShowSourcesIcons = 0;

	ShowIcons();
}

void CSourcesListBox::NewPlaylist( wxString sName, const MusikSongIdArray & arrSongIds, int nType )
{
	//--- check validity ---//
	wxString sCheck = sName;
	sCheck.Replace( wxT( " " ), wxT( "" ) );
	if ( sCheck == wxT( "" ) )
	{
		wxMessageBox( _( "Invalid playlist name." ), MUSIKAPPNAME_VERSION, wxOK | wxICON_INFORMATION );
		return;
	}
	sName.Trim();
	sName.Trim(false);
	switch ( nType )
	{
	case MUSIK_SOURCES_PLAYLIST_STANDARD:
    //--- create standard playlist ---//
		CreateStdPlaylist( sName, arrSongIds );
		break;
	//--- create dynamic playlist
	case MUSIK_SOURCES_PLAYLIST_DYNAMIC:
		CreateDynPlaylist( sName );
		break;
	case MUSIK_SOURCES_NETSTREAM:
		CreateNetStream( sName );
		break;
	default:
		wxASSERT(false);
	}
}	

bool CSourcesListBox::PlaylistExists( wxString sName, int type )
{
	SourcesToFilename( &sName, type );
	return wxFileExists( sName );
}

void CSourcesListBox::RewriteStdPlaylist()
{
	//--- read what we're gonna write ---//
	wxString sFilename = GetItemText( m_CurSel );	
	wxString sItemText = sFilename;
	
	//--- remove the old file ---//
	SourcesToFilename( &sFilename );
	wxRemoveFile( sFilename );
	
	//--- write the new file ---//
	PlaylistToFile( sItemText, g_PlaylistBox->PlaylistCtrl().Playlist(), MUSIK_SOURCES_PLAYLIST_STANDARD );
}

bool CSourcesListBox::CreateStdPlaylist( wxString sName, const MusikSongIdArray & arrSongIds )
{
	//-------------------------------------------------------------//
	//--- see if the playlist exists, if it does check to see	---//
	//--- if its not visible. if its not then ask if it should	---//
	//--- be. will also prompt to append if already exists		---//
	//-------------------------------------------------------------//
	if ( PlaylistExists( sName, MUSIK_SOURCES_PLAYLIST_STANDARD ) )
	{
		//-----------------------------------------------------//
		//--- search the list to see if the item is visible ---//
		//-----------------------------------------------------//
		if ( FindInSources( sName, MUSIK_SOURCES_PLAYLIST_STANDARD ) == -1 )
		{
			if ( wxMessageBox( _( "Standard playlist \"" ) + sName + _( "\" already exists, but does not appear to be visible in the panel.\n\nWould you like to show it?" ), MUSIKAPPNAME_VERSION, wxYES_NO | wxICON_QUESTION ) == wxYES )
			{		
				m_SourcesList.Add( wxT( "[s] " ) + sName );
				Update();
			}
		}

		//-----------------------------------------------------//
		//--- if we don't have songs to add, don't bother	---//
		//--- with asking user to append the playlist. this	---//
		//--- will happen when the context menu is used	to	---//
		//--- create the list.								---//
		//-----------------------------------------------------//
		if ( arrSongIds.GetCount())
		{
			if ( wxMessageBox( _( "A playlist with this name already exists, would you like to append to the existing one?" ), MUSIKAPPNAME_VERSION, wxYES_NO | wxICON_QUESTION ) == wxYES )
				PlaylistToFile( sName, arrSongIds,MUSIK_SOURCES_PLAYLIST_STANDARD,false );
		}

		return true;
	}
	
	//-------------------------------------------------------------//
	//--- otherwise the playlist does not already exist, so	we	---//
	//--- need to add it to the sources list and update the		---//
	//--- control.												---//
	//-------------------------------------------------------------//
	if ( PlaylistToFile( sName, arrSongIds, MUSIK_SOURCES_PLAYLIST_STANDARD ) )
	{    
		m_SourcesList.Add( wxT( "[s] " ) + sName );
		Update();
	}
	
	return true;
}

bool CSourcesListBox::PlaylistToFile( wxString sName, const MusikSongIdArray & arrSongIds, int nType, bool bDelOld )
{
	SourcesToFilename( &sName, nType );
	if ( bDelOld )
	{
		if ( wxFileExists( sName ) )
			wxRemoveFile( sName );
	}

	wxTextFile Out;	
    if(bDelOld)
    {
        Out.Create( sName );
        Out.Open();
    }
    else
        Out.Open(sName);

	if ( !Out.IsOpened() )
		return false;

    
    for(size_t i = 0 ; i < arrSongIds.GetCount();i++)  
    {
    
        Out.AddLine( arrSongIds[i].Song()->MetaData.Filename.GetFullPath() );
    }
	Out.Write( Out.GuessType() );
	Out.Close();
    
	return true;
}

bool CSourcesListBox::PlaylistToFile( wxString sName, const wxString & sData, int nType, bool bDelOld )
{
	SourcesToFilename( &sName, nType );
	if ( bDelOld )
	{
		if ( wxFileExists( sName ) )
			wxRemoveFile( sName );
	}

	wxTextFile Out;	
	Out.Create( sName );
	Out.Open();

	if ( !Out.IsOpened() )
		return false;

    Out.AddLine( sData );
	Out.Write( Out.GuessType() );
	Out.Close();
    
	return true;
}

bool CSourcesListBox::CreateDynPlaylist( wxString sName )
{
	//-----------------------------------------------------//
	//--- see if the playlist with this name already	---//
	//--- exists on the user's hard drive. otherwise	---//
	//--- create it.									---//
	//-----------------------------------------------------//
	if ( PlaylistExists( sName, MUSIK_SOURCES_PLAYLIST_DYNAMIC ) )
	{
		//-----------------------------------------------------//
		//--- search the list to see if the item is visible ---//
		//-----------------------------------------------------//
		int nItemPos = FindInSources( sName, MUSIK_SOURCES_PLAYLIST_DYNAMIC );
		if ( nItemPos == -1 )
		{
			if ( wxMessageBox( _( "Dynamic playlist \"" ) + sName + _( "\" already exists, but does not appear to be visible in the panel.\n\nWould you like to show it?" ), MUSIKAPPNAME_VERSION, wxYES_NO | wxICON_QUESTION ) == wxYES )
			{		
				nItemPos = m_SourcesList.GetCount();
				m_SourcesList.Add( wxT( "[d] " ) + sName );
				Update();
			}
		}

		//-----------------------------------------------------//
		//--- ask the user if he wants to edit the current	---//
		//--- query since it already exists					---//
		//-----------------------------------------------------//
		int nAnswer = wxMessageBox( _( "A dynamic playlist with this name already exists, would you like to edit the existing query?" ), MUSIKAPPNAME_VERSION, wxYES_NO | wxICON_QUESTION );
		if ( nAnswer == wxYES )
			UpdateDynPlaylist( nItemPos );

		return false;
	}

	//-----------------------------------------------------//
	//--- ask user for a query. if the user doesn't		---//
	//--- give a blank one back, save this to file, and	---//
	//--- return.										---//
	//-----------------------------------------------------//
	else
	{
		wxString sQuery = PromptDynamicPlaylist( wxT( "" ) );

		if ( sQuery != wxT( "" ) )
		{
			PlaylistToFile( sName, sQuery, MUSIK_SOURCES_PLAYLIST_DYNAMIC );
			m_SourcesList.Add( wxT( "[d] " ) + sName );
			Update();
			
			return true;
		}
		else
			return false;
	}
}

void CSourcesListBox::UpdateDynPlaylist( int nIndex )
{
	wxString sName	= GetPlaylistName( nIndex );
	wxString sQuery = LoadDynPlaylist( sName );

	sQuery = PromptDynamicPlaylist( sQuery );

	if ( sQuery != wxT( "" ) )
	{
		m_SourcesList.Item( nIndex ) = wxT( "[d] " ) + sName;
		PlaylistToFile( sName, sQuery, MUSIK_SOURCES_PLAYLIST_DYNAMIC );

		RealizeDynPlaylist(sQuery,g_thePlaylist);
		g_PlaylistBox->SetPlaylist(&g_thePlaylist);
	}
}

wxString CSourcesListBox::PromptDynamicPlaylist( wxString sQuery )
{
	wxString sInfo;
	sInfo += _("The following fields can be used in your query");
	sInfo +=wxT(":\n");
	for( size_t i = 0; i < PlaylistColumn::NCOLUMNS ; i ++)
	{
		sInfo += g_PlaylistColumn[i].DBName;
		if(i < PlaylistColumn::NCOLUMNS - 1)
			sInfo += wxT(", ");
		if((i % 8) == 7)
			sInfo += wxT("\n");

	}
	sInfo += wxT("\n\n");
	sInfo += wxString(_("Examples:\n")) +
		_T("title like '%funky%'\n")		 +
		_("(all titles containing funky)\n") +
		_T("bitrate < 128, vbr = 0\n")		 +
		_("(all low quality, non-VBR)\n")	 +
		_T("timesplayed > 10 order by artist\n") +   
		_("(your popular tracks)\n")			 +
		_T("lastplayed !='' and lastplayed >= julianday('now','start of month')\n")   +
		_("(all songs played this month)");
	wxMultiLineTextEntryDialog dlg( this,sInfo , MUSIKAPPNAME_VERSION, sQuery );

	if ( dlg.ShowModal() == wxID_OK )
		return dlg.GetValue();	

	return wxT( "" );
}

bool CSourcesListBox::CreateNetStream( wxString sName)
{
	//-------------------------------------------------------------//
	//--- see if the net stream exists, if it does check to see	---//
	//--- if its not visible. if its not then ask if it should	---//
	//--- be. will also prompt to append if already exists		---//
	//-------------------------------------------------------------//
	if ( PlaylistExists( sName, MUSIK_SOURCES_NETSTREAM ) )
	{
		//-----------------------------------------------------//
		//--- search the list to see if the item is visible ---//
		//-----------------------------------------------------//
		if ( FindInSources( sName, MUSIK_SOURCES_NETSTREAM ) == -1 )
		{
			if ( wxMessageBox( _( "Net Stream \"" ) + sName + _( "\" already exists, but does not appear to be visible in the panel.\n\nWould you like to show it?" ), MUSIKAPPNAME_VERSION, wxYES_NO | wxICON_QUESTION ) == wxYES )
			{		
				m_SourcesList.Add( wxT( "[u] " ) + sName );
				Update();
			}
		}
		return true;
	}
	
	//---------------------------------------------------------------//
	//--- otherwise the net stream does not already exist, so we  ---//
	//--- need to add it to the sources list and update the		  ---//
	//--- control.												  ---//
	//---------------------------------------------------------------//
	wxString sAddress = PromptNetStreamAddress(wxT(""));
	if ((sAddress.IsEmpty() == false) && PlaylistToFile( sName, sAddress, MUSIK_SOURCES_NETSTREAM ) )
	{    
		m_SourcesList.Add( wxT( "[u] " ) + sName );
		Update();
		return true;
	}
	return false;
	
}
wxString CSourcesListBox::PromptNetStreamAddress( const wxString &sAddress )
{
	wxTextEntryDialog dlg( this, _( "Enter net stream address(URL):" ), MUSIKAPPNAME_VERSION, sAddress );

	if ( dlg.ShowModal() == wxID_OK )
		return dlg.GetValue();	

	return wxT( "" );
}
void CSourcesListBox::UpdateNetStream( int nIndex )
{
	wxString sName	= GetPlaylistName( nIndex );
    std::auto_ptr<CMusikSong> pSong(new CMusikSong);
	LoadNetStream( sName,*pSong );

	wxString sAddress = PromptNetStreamAddress(pSong->MetaData.Filename.GetFullPath() );

	if ( sAddress != wxT( "" ) )
	{
		m_SourcesList.Item( nIndex ) = wxT( "[u] " ) + sName;
		PlaylistToFile( sName, sAddress, MUSIK_SOURCES_NETSTREAM );
		g_thePlaylist.Clear();
		pSong->MetaData.Filename =  sAddress;
		g_thePlaylist.Add(MusikSongId(pSong.release()));
		g_PlaylistBox->SetPlaylist(&g_thePlaylist);
	}
}
wxString CSourcesListBox::GetPlaylistName( int nIndex )
{
	wxString sRet = m_SourcesList.Item( nIndex );
	sRet = sRet.Right( sRet.Length() - 4 );
	return sRet;
}

void CSourcesListBox::LoadStdPlaylist(wxString sName, MusikSongIdArray & songids )
{
	songids.Clear();

	SourcesToFilename( &sName );

	if ( !wxFileExists( sName ) )
	{
		return ;
	}
	wxTextFile In( sName );
	In.Open();
	if ( !In.IsOpened() )
	{
		return;
	}
	wxArrayString arrFilenames;
	arrFilenames.Alloc(In.GetLineCount());
	for ( size_t i = 0; i < In.GetLineCount(); i++ )
	{
		wxString sCheck = In.GetLine( i );
		sCheck.Replace( wxT( " " ), wxT( "" ), TRUE );
		if ( !sCheck.IsEmpty() )
        {
            arrFilenames.Add(In.GetLine( i ));
        }
	}
	In.Close();

    wxGetApp().Library.GetFilelistSongs(arrFilenames,songids);
	return;
}

wxString CSourcesListBox::LoadDynPlaylist( wxString sName )
{
	wxString sReturn;

	SourcesToFilename( &sName, MUSIK_SOURCES_PLAYLIST_DYNAMIC );

	if ( !wxFileExists( sName ) )
		return wxT( "" );

	wxTextFile In( sName );
	In.Open();
	if ( !In.IsOpened() )
		return wxT( "" );
	for ( size_t i = 0; i < In.GetLineCount(); i++ )
	{
		sReturn += In.GetLine( i );
	}
	return sReturn;
}
void CSourcesListBox::RealizeDynPlaylist(  const wxString & sQuery, MusikSongIdArray & aReturn )
{
	wxString myQuery = sQuery;
	if(sQuery.StartsWith(wxT("FROM "),&myQuery))
	{
		wxGetApp().Library.QuerySongsFrom( myQuery ,aReturn);
	}
	else
		wxGetApp().Library.QuerySongsWhere( sQuery, aReturn );
}
void CSourcesListBox::LoadNetStream(wxString sName, CMusikSong & song )
{
	wxString sFilename = sName;
	SourcesToFilename( &sFilename ,MUSIK_SOURCES_NETSTREAM);

	if ( !wxFileExists( sFilename ) )
	{
		return ;
	}
	wxTextFile In( sFilename );
	In.Open();
	if ( !In.IsOpened() )
	{
		return;
	}
	if(In.GetLineCount() >= 1)
	{
		song.MetaData.Title = ConvToUTF8( sName );
		song.MetaData.Artist = ConvToUTF8( _("Net Stream"));
		song.MetaData.Filename = ( In.GetLine(0) );
		song.MetaData.eFormat = MUSIK_FORMAT_NETSTREAM;

	}
	In.Close();

	return;
}
void CSourcesListBox::AddMissing( const wxArrayString & playlists ,EMUSIK_SOURCES_TYPE t)
{
	wxString sExt, sName, sAdd,sTypeExt;
	sTypeExt = GetExtFromType(t);
	GetTypeAsString( t, sAdd);
	for ( size_t i = 0; i < playlists.GetCount(); i++ )
	{
		sName = playlists.Item( i );

		sExt = sName.Right( 3 );
		sExt.MakeLower();
		if ( sExt != sTypeExt )
		{
			continue;
		}

		sName = sName.Left( sName.Length() - 4 );
		sName.Replace( wxT( "_" ), wxT( " " ), true );

		if ( FindInSources( sName, t ) == -1 )
			m_SourcesList.Add( sAdd + sName);
	}
}
wxString CSourcesListBox::GetExtFromType(EMUSIK_SOURCES_TYPE t)const
 {
	 wxString sExt;
	 switch( t )
	 {
	 case MUSIK_SOURCES_PLAYLIST_STANDARD:
		 sExt = wxT( "mpl" );
		 break;
	 case MUSIK_SOURCES_PLAYLIST_DYNAMIC:
		 sExt = wxT( "mpd" );
		 break;
	 case MUSIK_SOURCES_NETSTREAM:
		 sExt = wxT( "mpu" );
		 break;
	 default:
		 return wxEmptyString;
	 }
	 return sExt;
 }

bool CSourcesListBox::GetTypeAsString(int nType,wxString &sType)  const
{
	switch( nType )
	{
	case MUSIK_SOURCES_LIBRARY:
		sType = wxT( "[l] " );
		break;
	case MUSIK_SOURCES_PLAYLIST_STANDARD:
		sType = wxT( "[s] " );
		break;
	case MUSIK_SOURCES_PLAYLIST_DYNAMIC:
		sType = wxT( "[d] " );
		break;
	case MUSIK_SOURCES_CDROM_DRIVE:
		sType = wxT( "[c] " );
		break;
	case MUSIK_SOURCES_NOW_PLAYING:
		sType = wxT( "[n] " );
		break;
	case MUSIK_SOURCES_NETSTREAM:
		sType = wxT( "[u] " );
		break;
	default:
		return false;
	}
	return true;
}
int CSourcesListBox::FindInSources( wxString sName, int nType )
{
	switch( nType )
	{
	case MUSIK_SOURCES_LIBRARY:
		return 0;
		break;
	case MUSIK_SOURCES_NOW_PLAYING:
		return 1;
		break;
	default:
		{
			wxString sFind;
			if( !GetTypeAsString( nType, sFind))
				return -1;
			sFind += sName;
			sFind.MakeLower();

			for ( size_t i = 0; i < m_SourcesList.GetCount(); i++ )
			{
				if ( sFind == m_SourcesList.Item( i ).Lower() )
					return i;
			}
		}
	}
	return -1;
}

void CSourcesListBox::SourcesToFilename( wxString* sSources, int nType )
{
	wxString sName = *sSources;
	sName.Replace( wxT( " " ), wxT( "_" ), TRUE );
	sName = sName.Lower();	

	wxString sExt;
	switch ( nType )
	{
	case MUSIK_SOURCES_PLAYLIST_STANDARD:
		sExt = wxT( ".mpl" );
		break;
	case MUSIK_SOURCES_PLAYLIST_DYNAMIC:
		sExt = wxT( ".mpd" );
		break;
	case MUSIK_SOURCES_NETSTREAM:
		sExt = wxT( ".mpu" );
		break;
	default:
		wxASSERT(false);
	}	
	*sSources = MUSIK_PLAYLIST_DIR + sName + sExt;
}


bool CSourcesListBox::AddSourceContentToNowPlaying(int nIndex)
{
	int nType = GetType(nIndex);
	MusikSongIdArray songids;
	if (nType == MUSIK_SOURCES_PLAYLIST_STANDARD )
	{
		LoadStdPlaylist( GetItemText( nIndex ), songids );
	}

	//--- dynamic playlist selected ---//
	else if ( nType == MUSIK_SOURCES_PLAYLIST_DYNAMIC )
	{
		wxString sQuery = LoadDynPlaylist( GetItemText( nIndex ) );
		RealizeDynPlaylist(sQuery,songids);
	}
	else if ( nType == MUSIK_SOURCES_NOW_PLAYING )
	{
		return false;
	}
	else if ( nType == MUSIK_SOURCES_NETSTREAM )
	{
		CMusikSong *pSong = new CMusikSong;
		LoadNetStream(GetItemText( nIndex ), *pSong);
		songids.Add( MusikSongId(pSong));
	}
	else
	{
		return false;
	}
	wxGetApp().Player.AddToPlaylist(songids,false);
	return true;
}

BEGIN_EVENT_TABLE(CSourcesBox, wxSashLayoutWindow)
	EVT_SASH_DRAGGED			( MUSIK_SOURCES,				CSourcesBox::OnSashDragged			)	
	EVT_SIZE					( 	CSourcesBox::OnSize			)	
END_EVENT_TABLE()

CSourcesBox::CSourcesBox( wxWindow *parent )
	: wxSashLayoutWindow( parent, MUSIK_SOURCES, wxDefaultPosition , wxDefaultSize, wxNO_BORDER | wxCLIP_CHILDREN |wxSW_3D|wxTAB_TRAVERSAL )
{

	m_pPanel = new wxPanel( this, -1, wxDefaultPosition , wxDefaultSize , wxNO_BORDER|wxCLIP_CHILDREN|wxTAB_TRAVERSAL );
	//--- CSourcesListBox ---//
	m_pListBox	= new CSourcesListBox( m_pPanel );
	m_pPictureBox = new CPictureBox(m_pPanel);
	
	//--- top sizer ---//
	wxBoxSizer *pSizer = new wxBoxSizer( wxVERTICAL );
	pSizer->Add( m_pListBox, 1, wxEXPAND|wxALIGN_TOP , 0 );
	m_pPictureBox && pSizer->Add( m_pPictureBox, 0, wxEXPAND|wxALIGN_BOTTOM , 0 );
	m_pPanel->SetSizer( pSizer );
	Update();
}

void CSourcesBox::OnSashDragged	(wxSashEvent & ev)
{
	wxGetApp().Prefs.nSourceBoxWidth = ev.GetDragRect().width;
	SetDefaultSize(wxSize(wxGetApp().Prefs.nSourceBoxWidth, 1000));
	ev.Skip();
}
void CSourcesBox::OnSize( wxSizeEvent& event )
{
	if(m_pPictureBox && m_pPictureBox->IsShown())
	{
		wxSize s =GetClientSize();
		s.SetHeight(s.GetWidth());
		m_pPictureBox->SetSizeHints(s,s);
		wxSashLayoutWindow::OnSize(event);
		m_pPictureBox->Refresh();
	}
	else
	{	
		//m_pPictureBox->SetSizeHints(0,0);
		event.Skip();
	}
}
void CSourcesBox::ShowAlbumArt(bool bShow)
{
	m_pPictureBox && m_pPictureBox->Show(bShow);
	if(m_pPictureBox && bShow)
	{
		wxSize s =GetClientSize();
		s.SetHeight(s.GetWidth());
		m_pPictureBox->SetSizeHints(s,s);
	}
	else
	{	
		//m_pPictureBox->SetSizeHints(0,0);
	}
	m_pPanel->Layout();
}
void CSourcesBox::Update( ) 
{ 
	m_pListBox->Update();
#ifndef __WXMAC__	
	SetBackgroundColour(wxGetApp().Prefs.bPlaylistBorder ?  
		StringToColour(wxGetApp().Prefs.sPlaylistBorderColour)
		:wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
	m_pPictureBox && m_pPictureBox->SetBackgroundColour(wxGetApp().Prefs.bPlaylistBorder ?  
		StringToColour(wxGetApp().Prefs.sPlaylistBorderColour)
		:wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
#endif		
}

CSourcesBox::~CSourcesBox()
{
	
}
