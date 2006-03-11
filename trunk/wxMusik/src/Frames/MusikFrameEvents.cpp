/*
 *  MusikFrameEvents.cpp
 *
 *  All events dealing with Musik's main frame, MusikFrame.h / .cpp
 *  
 *  Copyright (c) 2003 Casey Langen (casey@bak.rr.com)
 *	Contributors: Simon Windmill, Dustin Carter, Gunnar Roth, Wade Brainerd
 *
 *  See the file "license.txt" for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL WARRANTIES.
*/

//--- For compilers that support precompilation, includes "wx/wx.h". ---//
#include "myprec.h"

#include "MusikFrame.h"
#include "Classes/PlaylistCtrl.h"
#include "Classes/SourcesBox.h"
//--- crossfader, other threads ---//
#include "Threads/MusikThreads.h"

//--- frames ---//
#include "MusikLibraryFrame.h"
#include "Prefs/MusikPrefsFrame.h"
#include "MusikFXFrame.h"

#include "MusikAboutDlg.h"
//--- globals ---//
#include "MusikGlobals.h"
#include "MusikUtils.h"
#include "MusikApp.h"
#ifdef __WXMSW__
	#include "../MMShellHook/MMShellHook.h"
#endif
#include "images/tray.xpm"

DECLARE_APP( MusikApp )
//--- wx stuff we need ---//
#include <wx/filedlg.h>

BEGIN_EVENT_TABLE(MusikFrame, wxFrame)
	EVT_MENU					(wxID_PREFERENCES,					MusikFrame::OnPreferences			)	// File->Preferences
    EVT_UPDATE_UI				(wxID_PREFERENCES,					MusikFrame::OnUpdateUIPreferences			)	// File->Preferences
	EVT_MENU					(wxID_EXIT,							MusikFrame::OnMenuClose				)	// File->Exit
	EVT_MENU					(wxID_ABOUT,						MusikFrame::OnMenuAbout				)	// Help->About
	EVT_MENU					(MUSIK_MENU_SOURCES_STATE,			MusikFrame::OnSourcesState			)	// View->Show Sources
	EVT_UPDATE_UI				(MUSIK_MENU_SOURCES_STATE,			MusikFrame::OnUpdateUISourcesState	)	
	EVT_MENU					(MUSIK_MENU_ACTIVITIES_STATE,		MusikFrame::OnActivitiesState		)	// View->Show Selections
	EVT_UPDATE_UI				(MUSIK_MENU_ACTIVITIES_STATE,		MusikFrame::OnUpdateUIActivitiesState)	
	EVT_MENU					(MUSIK_MENU_PLAYLISTINFO_STATE,		MusikFrame::OnPlaylistInfoState		)	// View->Show Playlist Info
	EVT_UPDATE_UI				(MUSIK_MENU_PLAYLISTINFO_STATE,		MusikFrame::OnUpdateUIPlaylistInfoState	)	
	EVT_MENU					(MUSIK_MENU_ALBUMART_STATE,			MusikFrame::OnAlbumartState		)	// View->Show Album Art
	EVT_UPDATE_UI				(MUSIK_MENU_ALBUMART_STATE,			MusikFrame::OnUpdateUIAlbumartState	)	
	EVT_MENU					(MUSIK_MENU_NOWPLAYINGCONTROL_ONTOP_STATE,			MusikFrame::OnNowPlayingControlOnTopState		)	// View->Show Album Art
	EVT_UPDATE_UI				(MUSIK_MENU_NOWPLAYINGCONTROL_ONTOP_STATE,			MusikFrame::OnUpdateUINowPlayingControlOnTopState	)	
	EVT_MENU_RANGE				(MUSIK_MENU_SELECT_SOURCES_LIBRARY,MUSIK_MENU_SELECT_SOURCES_NOWPLAYING	,		MusikFrame::OnSelectSources			)	// View->Select xxx

	EVT_MENU					(MUSIK_MENU_STAY_ON_TOP,			MusikFrame::OnStayOnTop				)	// View->Stay On Top
	EVT_UPDATE_UI				(MUSIK_MENU_STAY_ON_TOP,			MusikFrame::OnUpdateUIStayOnTop		)	
	EVT_MENU					(MUSIK_MENU_FX,						MusikFrame::OnFX					)
	EVT_MENU					(MUSIK_MENU_PATHS,					MusikFrame::OnSetupPaths			)	// Library->Setup Paths
    EVT_UPDATE_UI				(MUSIK_MENU_PATHS,					MusikFrame::OnUpdateUISetupPaths			)	// Library->Setup Paths
	EVT_MENU					(MUSIK_MENU_CUSTOMQUERY,			MusikFrame::OnCustomQuery			)	// Library->Custom Query
	EVT_MENU					(MUSIK_MENU_VIEW_DIRTY_TAGS,		MusikFrame::OnViewDirtyTags			)	// Library->Write Tags->View
	EVT_MENU					(MUSIK_MENU_WRITE_TAGS,				MusikFrame::OnWriteTags				)	// Library->Write Tags->Write
	EVT_MENU					(MUSIK_MENU_WRITE_CLEAR_DIRTY,		MusikFrame::OnWriteTagsClearDirty	)	// Library->Write Tags->Finalize DB
	EVT_MOVE					(									MusikFrame::OnMove					)	// main dlg moved
	EVT_MAXIMIZE				(									MusikFrame::OnMaximize				)	// main dlg maximized
	EVT_ICONIZE					(									MusikFrame::OnIconize				)	// main dlg minimized
	EVT_CLOSE					(									MusikFrame::OnClose					)
	EVT_SIZE					(									MusikFrame::OnSize					)	// main dlg resized
	EVT_CHAR					(									MusikFrame::OnTranslateKeys			)	// key pressed					

	//---------------------------------------------------------//
	//--- threading events.. we use EVT_MENU becuase its	---//
	//--- nice and simple, and gets the job done. this may	---//
	//--- become a little prettier later, but it works.		---//
	//---------------------------------------------------------//
    EVT_MENU					( MUSIK_FRAME_THREAD_START,		MusikFrame::OnStartProgress			) 
	EVT_MENU					( MUSIK_FRAME_THREAD_END,		MusikFrame::OnEndProgress			) 
	EVT_MENU					( MUSIK_FRAME_THREAD_PROG,		MusikFrame::OnUpdateProgress		)
	EVT_MENU					( MUSIK_FRAME_EXIT_FADE_DONE,	MusikFrame::OnMenuClose				)
	EVT_SASH_DRAGGED			( MUSIK_SOURCES,				MusikFrame::OnSashDraggedSourcesBox	)
	EVT_SASH_DRAGGED			( MUSIK_ACTIVITYCTRL,			MusikFrame::OnSashDraggedActivityCtrl)
#ifndef __WXMAC__
	EVT_ERASE_BACKGROUND		( MusikFrame::OnEraseBackground )
#endif
END_EVENT_TABLE()


//------------------------------------------//
//--- non activity box event definitions ---//
//------------------------------------------//
void MusikFrame::OnMove ( wxMoveEvent& WXUNUSED(event) )	
{ 
	if ( !g_DisablePlacement )
		wxGetApp().Prefs.sFramePlacement = GetFramePlacement( this );
}

void MusikFrame::OnMaximize( wxMaximizeEvent& event )
{
	if ( !g_DisablePlacement )
		wxGetApp().Prefs.sFramePlacement = GetFramePlacement( this );
	event.Skip();
}

void MusikFrame::OnIconize( wxIconizeEvent& event )
{
	if ( !g_DisablePlacement )
		wxGetApp().Prefs.sFramePlacement = GetFramePlacement( this );
#ifdef wxHAS_TASK_BAR_ICON
	if( m_pTaskBarIcon && wxGetApp().Prefs.bHideOnMinimize )
	{
		if(event.Iconized())
		{
			Show(FALSE);
		}
		event.Skip(FALSE);
	}
	else
		event.Skip();
#else
	event.Skip();
#endif
}

void MusikFrame::OnSize	( wxSizeEvent& WXUNUSED(event) )	
{
//	wxFrame::OnSize(event);
//	m_pBottomPanel->SetDefaultSize(vsTopBottom->GetMinSize());
//	m_pBottomPanel->Layout();

	wxLayoutAlgorithm layout;
    layout.LayoutWindow(this,g_PlaylistBox);

//	m_pBottomPanel->Layout();

//	m_pNowPlayingCtrl->Refresh();
//	m_pNowPlayingCtrl->Update();

	if ( !g_DisablePlacement )
		wxGetApp().Prefs.sFramePlacement = GetFramePlacement( this );
}

void MusikFrame::OnClose( wxCloseEvent& WXUNUSED(event) )
{
    //-------------------------------------------------//
    //--- if fade out on exit is enabled, hide the	---//
    //--- frame and allow the fade to happen. an 	---//
	//--- event will be posted back when done.		---//
    //-------------------------------------------------//
	Show( false );
	if ( wxGetApp().Prefs.bGlobalFadeEnable && wxGetApp().Prefs.bFadeExitEnable && wxGetApp().Player.IsPlaying() && !wxGetApp().Player.IsPaused() )
	{
		wxGetApp().Player.Stop( true, true );
		return;
	}

	#ifdef __WXMSW__
		UnSetMMShellHook((HWND)GetHWND());
	#endif

	//-------------------------------------------------//
	//--- write playlist columns to prefs			---//
	//-------------------------------------------------//
	g_PlaylistBox->PlaylistCtrl().SaveColumns();

	//-------------------------------------------------//
	//--- save prefs and paths,						---//
	//-------------------------------------------------//
	g_Paths.Save();

    //-------------------------------------------------//
    //--- stop webserver if necessary				---//
    //-------------------------------------------------//
    if ( wxGetApp().Prefs.bWebServerEnable )
		wxGetApp().WebServer.Stop();

	wxGetApp().Player.Shutdown();
	Destroy();
}

void MusikFrame::OnSetupPaths( wxCommandEvent& WXUNUSED(event) )
{
	wxSize mysize( 400, 300 );
	MusikLibraryDialog* pMusikLibraryFrame = new MusikLibraryDialog(this, wxDefaultPosition, mysize );
	pMusikLibraryFrame->Show();
}
void MusikFrame::OnUpdateUISetupPaths( wxUpdateUIEvent&	event)
{
    event.Enable(!IsTLWActive(CLASSINFO(MusikLibraryDialog)));
}
void MusikFrame::OnMenuAbout( wxCommandEvent &WXUNUSED(event) )
{
	CMusikAboutDlg d(this);
	d.ShowModal();

}
void MusikFrame::OnPreferences( wxCommandEvent &WXUNUSED(event) )
{
	wxDialog *pDlg = new MusikPrefsDialog( this, wxString(MUSIKAPPNAME) + _(" Preferences") );
	pDlg->Show();
}
void MusikFrame::OnUpdateUIPreferences( wxUpdateUIEvent &event )
{
    
    event.Enable(!IsTLWActive(CLASSINFO(MusikPrefsDialog)));
}

void MusikFrame::OnFX( wxCommandEvent &WXUNUSED(event) )
{
	wxWindow *w = wxWindow::FindWindowById(MUSIK_FRAME_ID_FX);
	if(w)
	{
		w->Raise();
	}
	else
	{
		w = new MusikFXDialog( this, wxString(MUSIKAPPNAME) + _(" FX"), wxDefaultPosition, wxDefaultSize );
		w->Show();
	}	
}

void MusikFrame::OnStayOnTop( wxCommandEvent &WXUNUSED(event) )	
{ 
	wxGetApp().Prefs.bStayOnTop = !wxGetApp().Prefs.bStayOnTop;
	SetStayOnTop(wxGetApp().Prefs.bStayOnTop);
}
void MusikFrame::OnUpdateUIStayOnTop( wxUpdateUIEvent& event)
{
	event.Check(wxGetApp().Prefs.bStayOnTop);
}
void MusikFrame::OnPlaylistInfoState( wxCommandEvent& WXUNUSED(event) )	
{ 
	TogglePlaylistInfo();
}
void MusikFrame::OnUpdateUIPlaylistInfoState( wxUpdateUIEvent& event)
{
	event.Check(wxGetApp().Prefs.bShowPLInfo);
}
void MusikFrame::OnNowPlayingControlOnTopState( wxCommandEvent& WXUNUSED(event) )	
{ 
	wxGetApp().Prefs.bShowNowPlayingControlOnTop = !wxGetApp().Prefs.bShowNowPlayingControlOnTop;
	m_pBottomPanel->SetAlignment(wxGetApp().Prefs.bShowNowPlayingControlOnTop == true ? wxLAYOUT_TOP : wxLAYOUT_BOTTOM);
	SendSizeEvent();	
	//Show(false); // HACK to force complete redraw
	//Show(true); 			
}
void MusikFrame::OnUpdateUINowPlayingControlOnTopState( wxUpdateUIEvent& event)
{
	event.Check(wxGetApp().Prefs.bShowNowPlayingControlOnTop);
}

void MusikFrame::OnAlbumartState( wxCommandEvent& WXUNUSED(event) )	
{ 
	wxGetApp().Prefs.bShowAlbumArt = !wxGetApp().Prefs.bShowAlbumArt;
	ShowAlbumArt();
	
}
void MusikFrame::OnUpdateUIAlbumartState( wxUpdateUIEvent& event)
{
	event.Check(wxGetApp().Prefs.bShowAlbumArt);
	event.Enable(g_SourcesCtrl->IsShown());

}

void MusikFrame::OnSourcesState( wxCommandEvent& WXUNUSED(event) )
{ 
	ToggleSources();
}
void MusikFrame::OnUpdateUISourcesState( wxUpdateUIEvent& event)
{
	event.Check( wxGetApp().Prefs.bShowSources );
}

void MusikFrame::OnActivitiesState( wxCommandEvent& WXUNUSED(event) )
{ 
	ToggleActivities();
}
void MusikFrame::OnUpdateUIActivitiesState( wxUpdateUIEvent& event)
{
	event.Check(wxGetApp().Prefs.bShowActivities);
}

void MusikFrame::OnCustomQuery( wxCommandEvent& WXUNUSED(event) )
{ 
	LibraryCustomQuery();		
}

void MusikFrame::OnViewDirtyTags( wxCommandEvent& WXUNUSED(event) )
{ 
	static MusikSongIdArray s_dirty;
	wxGetApp().Library.QuerySongsWhere( wxT( "dirty = 1" ), s_dirty );
	if ( s_dirty.GetCount() > 0 )
	{

		g_PlaylistBox->SetPlaylist(&s_dirty);
		g_SourcesCtrl->SelectLibrary(false);  // only change selection, not the view( to protect playlist from being changed. ok that is a hack, but else i would have to change too much of the structure. this will be done sometime later)
		ShowActivityArea( false );
	}
	else
		wxMessageBox( _( "There are no pending tags available to display." ), MUSIKAPPNAME_VERSION, wxICON_INFORMATION );
}

void MusikFrame::OnWriteTags( wxCommandEvent& WXUNUSED(event) )
{ 
	WriteTags();			
}

void MusikFrame::OnWriteTagsClearDirty( wxCommandEvent& WXUNUSED(event) )
{
	int nCleared = wxGetApp().Library.ClearDirtyTags();
	if ( !nCleared )
		wxMessageBox( _( "There are no pending tags to finalize." ), MUSIKAPPNAME_VERSION, wxICON_INFORMATION );
	else
		wxMessageBox( IntTowxString( nCleared ) + wxT( " tags pending to be written were finalized for the database only. These tags will not be written to file." ), MUSIKAPPNAME_VERSION, wxICON_INFORMATION );
}

void MusikFrame::OnTranslateKeys( wxKeyEvent& event )
{
	if ( event.GetKeyCode() == WXK_ESCAPE )		
	{
		if ( GetActiveThread() != NULL )
		{
			SetTitle( _( ": Aborting process, please wait..." ) );
			GetActiveThread()->Delete();
		}
	}
	else
		event.Skip();
}

void MusikFrame::WriteTags()
{
	if ( g_MusikFrame->GetActiveThread() == NULL )
        {
			//--- ask user if he wants to clear tags first ---//
			bool bClear = false;
			if ( wxMessageBox( _( "Clear old tag information before writing?" ), MUSIKAPPNAME_VERSION, wxYES_NO | wxICON_QUESTION  ) == wxYES )
				bClear = true;
			else
				bClear = false;		

			//--- fire up the thread ---//
			pWriteDirtyThread = new MusikWriteDirtyThread( bClear );
			pWriteDirtyThread->Create();
			pWriteDirtyThread->Run();
        }
	else
		InternalErrorMessageBox(wxT("Previous thread not terminated correctly."));
}

//------------------------//
//--- Library Routines ---//
//------------------------//
void MusikFrame::LibraryCustomQuery()
{
	wxString sQuery = g_SourcesCtrl->PromptDynamicPlaylist( wxT( "" ) );

	if ( !sQuery.IsEmpty() )
	{
		g_SourcesCtrl->SelectLibrary(false);  // only change selection, not the view( to protect playlist from being changed. ok that is a hack, but else i would have to much of the structure. this will be done sometime later)
		wxGetApp().Library.QuerySongsWhere( sQuery, g_thePlaylist );
		g_PlaylistBox->SetPlaylist(&g_thePlaylist);
	}
}

//-----------------------------------//
//--- events dealing with threads ---//
//-----------------------------------//
void MusikFrame::OnStartProgress( wxCommandEvent& WXUNUSED(event) )
{
	EnableProgress( true );

	if ( GetProgressType() == MUSIK_ACTIVITY_RENAME_THREAD )
	{
		SetTitle( _( ": Updating tags from selection box (ESC to abort)" ) );
	}
	else if ( GetProgressType() == MUSIK_PLAYLIST_RENAME_THREAD )
	{
		SetTitle( _( ": Renaming selected playlist items (ESC to abort)" ) );
	}
	else if ( GetProgressType() == MUSIK_WRITE_DIRTY_THREAD )
	{
		SetTitle( _( ": Writing dirty tags to file (ESC to abort)" ) );
	}
}

void MusikFrame::OnUpdateProgress( wxCommandEvent& WXUNUSED(event) )
{
	if ( GetActiveThread() != NULL )
	{
		m_pBottomPanel->SetProgress( GetProgress() );
	}
}

void MusikFrame::OnEndProgress( wxCommandEvent& WXUNUSED(event) )
{
	EnableProgress( false );
    m_pBottomPanel->SetProgress( 0 );
	SetProgress	( 0 );
	SetProgressType	( 0 );
	SetActiveThread	( NULL );

	SetTitle();
    g_PlaylistBox->Update();
}

void MusikFrame::OnSashDraggedSourcesBox	(wxSashEvent & WXUNUSED(ev))
{
	wxLayoutAlgorithm layout;
    layout.LayoutWindow(this,g_PlaylistBox);
}
void MusikFrame::OnSashDraggedActivityCtrl	(wxSashEvent & WXUNUSED(ev))
{
	wxLayoutAlgorithm layout;
    layout.LayoutWindow(this,g_PlaylistBox);
}


void MusikFrame::OnEraseBackground( wxEraseEvent& event )
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
	const wxWindowList & children = GetChildren();
	for ( const wxWindowList::Node *node = children.GetFirst(); node; node = node->GetNext() )
	{
		const wxWindow *current = (const wxWindow *)node->GetData();
        if(current->IsTopLevel())
            continue;
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

void MusikFrame::OnSelectSources( wxCommandEvent &event )
{
	switch(event.GetId())
	{
	case MUSIK_MENU_SELECT_SOURCES_NOWPLAYING:
		g_SourcesCtrl->SelectNowPlaying();
		break;
	case MUSIK_MENU_SELECT_SOURCES_LIBRARY:
		g_SourcesCtrl->SelectLibrary();

	}
}
void MusikFrame::OnSongChanged(MusikPlayerEvent & ev)
{
    std::auto_ptr<CMusikSong> pSong = ev.MusikPlayer().GetCurrentSongid().Song();
    if(pSong.get())
        SetSongInfoText( *pSong );
    ev.Skip();
}

void MusikFrame::OnPlayStop(MusikPlayerEvent & ev)
{
    SetTitle();
#ifdef wxHAS_TASK_BAR_ICON
    if(	m_pTaskBarIcon )
    {
        m_pTaskBarIcon->SetIcon(MUSIK_TRAY_ICON, _( "Playback Stopped" ) );
    }
#endif
    ev.Skip();
}

bool MusikFrame::IsTLWActive(wxClassInfo * pci)
{
    const wxWindowList & children = GetChildren();
    for ( const wxWindowList::Node *node = children.GetFirst(); node; node = node->GetNext() )
    {
        const wxWindow *current = (const wxWindow *)node->GetData();
        if(!current->IsTopLevel())
            continue;
        if(current->IsKindOf(pci))
            return true;
    }
    return false;
}