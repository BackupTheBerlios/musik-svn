/*
 *  MusikFrame.cpp
 *
 *  Musik's primary frame
 *  
 *  Copyright (c) 2003 Casey Langen (casey@bak.rr.com)
 *	Contributors: Simon Windmill, Dustin Carter, Gunnar Roth, Wade Brainerd
 *
 *  See the file "license.txt" for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL WARRANTIES.
*/

//--- For compilers that support precompilation, includes "wx/wx.h". ---//
#include "wx/wxprec.h"

#include "MusikFrame.h"

//--- globals ---//
#include "../MusikGlobals.h"
#include "../MusikUtils.h"
#include "../MusikApp.h"
#include "../Classes/NowPlayingCtrl.h"
//--- frames ---	//
#include "MusikLibraryFrame.h"
#include "MusikPrefsFrame.h"
#include "MusikTagFrame.h"

#ifdef __WXMSW__
	#include "../MMShellHook/MMShellHook.h"
#endif
#ifdef __WXGTK__
static void XF86AudioKeyGrab_init();
static void XF86AudioKeyGrab_cleanup();
#endif
//--- images ---//
#include "../images/sources/dynamic.xpm"
#include "../images/sources/library.xpm"
#include "../images/sources/standard.xpm"
#include "../images/sources/netstream.xpm"
#include "../images/sources/nowplaying.xpm"

#include "../images/ratings/rating0.xpm"
#include "../images/ratings/rating1.xpm"
#include "../images/ratings/rating2.xpm"
#include "../images/ratings/rating3.xpm"
#include "../images/ratings/rating4.xpm"
#include "../images/ratings/rating5.xpm"
#include "../images/ratings/rating6.xpm"
#include "../images/ratings/rating7.xpm"
#include "../images/ratings/rating8.xpm"
#include "../images/ratings/rating9.xpm"

#include "../images/ratings/rating-1.xpm"
#include "../images/ratings/rating-2.xpm"
#include "../images/ratings/rating-3.xpm"
#include "../images/ratings/rating-4.xpm"
#include "../images/ratings/rating-5.xpm"
#include "../images/ratings/rating-6.xpm"
#include "../images/ratings/rating-7.xpm"
#include "../images/ratings/rating-8.xpm"
#include "../images/ratings/rating-9.xpm"

#include "../images/tray.xpm"
DECLARE_APP( MusikApp )

#ifdef wxHAS_TASK_BAR_ICON
enum {
    PU_RESTORE = 11101,
    PU_HIDE ,
    PU_PLAYPAUSE,
	PU_PREV,
	PU_NEXT,
	PU_STOP,
    PU_EXIT
};


BEGIN_EVENT_TABLE(MusikTaskBarIcon, wxTaskBarIcon)
    EVT_MENU(PU_RESTORE,	MusikTaskBarIcon::OnMenuRestore)
    EVT_MENU(PU_HIDE,		MusikTaskBarIcon::OnMenuHide)
    EVT_MENU(PU_PLAYPAUSE,	MusikTaskBarIcon::OnMenuPlayPause)
    EVT_MENU(PU_PREV,	MusikTaskBarIcon::OnMenuPrev)
    EVT_MENU(PU_NEXT,	MusikTaskBarIcon::OnMenuNext)
    EVT_MENU(PU_STOP,	MusikTaskBarIcon::OnMenuStop)
	EVT_MENU_RANGE				( MUSIK_PLAYLIST_CONTEXT_RATING, MUSIK_PLAYLIST_CONTEXT_RATING + (MUSIK_MAX_RATING - MUSIK_MIN_RATING) + 1,			MusikTaskBarIcon::OnMenuRateSel			) 	
	EVT_UPDATE_UI_RANGE			( MUSIK_PLAYLIST_CONTEXT_RATING, MUSIK_PLAYLIST_CONTEXT_RATING + (MUSIK_MAX_RATING - MUSIK_MIN_RATING) + 1,			MusikTaskBarIcon::OnUpdateUIRateSel	)

    EVT_MENU(PU_EXIT,		MusikTaskBarIcon::OnMenuExit)
    EVT_TASKBAR_RIGHT_UP     (MusikTaskBarIcon::OnRButtonUp)
    EVT_TASKBAR_LEFT_DOWN  (MusikTaskBarIcon::OnLButtonDown)
END_EVENT_TABLE()

void MusikTaskBarIcon::RestoreFrame()
{
	m_pFrame->Show(TRUE);
	m_pFrame->Raise();
	//RemoveIcon();
}
void MusikTaskBarIcon::OnMenuRestore(wxCommandEvent& )
{
	RestoreFrame();
}
void MusikTaskBarIcon::OnMenuHide(wxCommandEvent& )
{
	m_pFrame->Show(FALSE);
}
void MusikTaskBarIcon::OnMenuPlayPause(wxCommandEvent& )
{
	wxGetApp().Player.PlayPause();
}
void MusikTaskBarIcon::OnMenuPrev(wxCommandEvent& )
{
	wxGetApp().Player.PrevSong();
}
void MusikTaskBarIcon::OnMenuNext(wxCommandEvent& )
{
	wxGetApp().Player.NextSong();
}
void MusikTaskBarIcon::OnMenuStop(wxCommandEvent& )
{
	wxGetApp().Player.Stop();
}

void MusikTaskBarIcon::OnMenuRateSel( wxCommandEvent& event )
{
	int songid = wxGetApp().Player.GetCurrentSongid();
	wxGetApp().Library.SetRating(songid,(event.GetId() - MUSIK_PLAYLIST_CONTEXT_RATING) + MUSIK_MIN_RATING );
}
void MusikTaskBarIcon::OnUpdateUIRateSel ( wxUpdateUIEvent &event)
{
	int songid = wxGetApp().Player.GetCurrentSongid();
	CMusikSong song;
	if(wxGetApp().Library.GetSongFromSongid(songid,&song ))
	{
		int nRating = song.Rating;
		wxASSERT(nRating >= MUSIK_MIN_RATING && nRating <= MUSIK_MAX_RATING);
		event.Check( event.GetId() == (MUSIK_PLAYLIST_CONTEXT_RATING + (nRating  - MUSIK_MIN_RATING)));
	}
	else
		event.Enable(false);
}
void MusikTaskBarIcon::OnMenuExit(wxCommandEvent& )
{

	wxCloseEvent event(wxEVT_CLOSE_WINDOW, -1);
	event.SetEventObject(m_pFrame);
	event.SetCanVeto(true);
	wxPostEvent(m_pFrame,event);
 
}



// Overridables
void MusikTaskBarIcon::OnRButtonUp(wxTaskBarIconEvent&)
{
    wxMenu      menu;

	if(!m_pFrame->IsShown())
	    menu.Append(PU_RESTORE, _("&Restore wxMusik"));
	else
		menu.Append(PU_HIDE, _("&Hide wxMusik"));
	menu.AppendSeparator();

	if ( wxGetApp().Player.IsPlaying() && !wxGetApp().Player.IsPaused() )
		menu.Append(PU_PLAYPAUSE, _("&Pause"));
	else if ( wxGetApp().Player.IsPlaying() && wxGetApp().Player.IsPaused() )
		menu.Append(PU_PLAYPAUSE, _("&Resume"));
	else if ( !wxGetApp().Player.IsPlaying() )
		menu.Append(PU_PLAYPAUSE, _("&Play"));
	menu.Append(PU_PREV, _("&Previous"));
	menu.Append(PU_NEXT, _("&Next"));
	if ( wxGetApp().Player.IsPlaying() )
		menu.Append(PU_STOP, _("&Stop"));

	//--- rating menu ---//
	wxMenu *rating_menu = new wxMenu;
	for(int i = MUSIK_MIN_RATING; i <= MUSIK_MAX_RATING ;i++)
	{
		rating_menu->Append(  MUSIK_PLAYLIST_CONTEXT_RATING  + (i - MUSIK_MIN_RATING), (i == 0) ? _( "Unrated" ) : wxString() << i, wxT( "" ), wxITEM_CHECK );
	}
	menu.Append( MUSIK_PLAYLIST_CONTEXT_RATENODE,_( "&Rating" ),	rating_menu );

	menu.AppendSeparator();
    menu.Append(PU_EXIT,    _("E&xit"));

    PopupMenu(&menu);
}

void MusikTaskBarIcon::OnLButtonDown(wxTaskBarIconEvent&)
{
	if(m_pFrame->IsShown())
		m_pFrame->Show(FALSE);
	else
		RestoreFrame();
}


#endif //#ifdef wxHAS_TASK_BAR_ICON

// main dialog constructor
MusikFrame::MusikFrame() 
	: wxFrame( (wxFrame*)NULL, -1, MUSIKAPPNAME_VERSION, wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE | wxTAB_TRAVERSAL | wxCLIP_CHILDREN )
{
	//--- pointer to main dialog, defined in MusikGlobals ---//
	g_MusikFrame = this;

	//--- prevent destroying saved placement info while constructing the frame ---//
	g_DisablePlacement = true;

	//--- icon ---//
	#ifdef __WXMSW__
		SetIcon( wxICON( musicbox ) );
	#else
		SetIcon( wxIcon(tray_xpm) );
	#endif
#ifdef wxHAS_TASK_BAR_ICON
	if(wxGetApp().Prefs.bDisableTrayIcon)
		m_pTaskBarIcon = NULL;
	else
		m_pTaskBarIcon = new MusikTaskBarIcon(this);
#endif

#ifdef __WXMAC__
	MacSetMetalAppearance(true);
#endif
	//--- colours ---//
//	this->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_3DFACE ) );

	//--- load image lists and fonts ---//
	LoadImageLists();
	GetFonts();

	//-------------------------//
	//--- initialize sizers ---//
	//-------------------------//
	vsTopBottom			= new wxBoxSizer	( wxVERTICAL	);



	m_pBottomPanel = new wxSashLayoutWindow(this,-1,wxDefaultPosition,wxDefaultSize,wxNO_BORDER|wxCLIP_CHILDREN);
	m_pBottomPanel->SetDefaultSize(wxSize(1000,70));
	m_pBottomPanel->SetAlignment(wxGetApp().Prefs.bShowNowPlayingControlOnTop == true ? wxLAYOUT_TOP : wxLAYOUT_BOTTOM);
	m_pBottomPanel->SetOrientation(wxLAYOUT_HORIZONTAL);
	//----------------//
	//--- playlist ---//
	//----------------//
	g_PlaylistBox  = new CPlaylistBox( this );
	GetListCtrlFont();

	//---------------//
	//--- sources ---//
	//---------------//
	g_SourcesCtrl = new CSourcesBox( this );

	g_SourcesCtrl->SetSashVisible(wxSASH_RIGHT, true);
	g_SourcesCtrl->SetDefaultSize(wxSize(wxGetApp().Prefs.nSourceBoxWidth,1000));
	g_SourcesCtrl->SetAlignment(wxLAYOUT_LEFT);
	g_SourcesCtrl->SetOrientation(wxLAYOUT_VERTICAL);
	g_SourcesCtrl->SetSashBorder(wxSASH_RIGHT, true);
	g_SourcesCtrl->SetSashBorder(wxSASH_RIGHT, true);
	g_SourcesCtrl->SetDefaultBorderSize(3);
	g_SourcesCtrl->SetExtraBorderSize(1);
	//---------------------//
	//--- activity area ---//
	//---------------------//
	g_ActivityAreaCtrl = new CActivityAreaCtrl( this );
	g_ActivityAreaCtrl->SetSashVisible(wxSASH_BOTTOM, true);
	g_ActivityAreaCtrl->SetDefaultSize(wxSize(1000,wxGetApp().Prefs.nActivityCtrlHeight));
	g_ActivityAreaCtrl->SetAlignment(wxLAYOUT_TOP);
	g_ActivityAreaCtrl->SetOrientation(wxLAYOUT_HORIZONTAL);
	g_ActivityAreaCtrl->SetSashBorder(wxSASH_BOTTOM, true);
	g_ActivityAreaCtrl->SetDefaultBorderSize(3);
	g_ActivityAreaCtrl->SetExtraBorderSize(1);
	

	//-------------------//
	//--- now playing ---//
	//-------------------//
	m_pNowPlayingCtrl = new CNowPlayingCtrl( m_pBottomPanel );

	//---------------------------------//
	//--- progress bar for whatever ---//
	//---------------------------------//
	m_pProgressGauge = new wxGauge( m_pBottomPanel, -1, 100, wxPoint( 0, 0 ), wxSize( 0, 18 ), wxGA_SMOOTH );

	//--------------------------//
	//--- top / bottom sizer ---//
	//--------------------------//
//	vsTopBottom->Add( hsLeftRight,		1, wxEXPAND | wxALL				  );
	vsTopBottom->Add( m_pProgressGauge,		0, wxEXPAND | wxLEFT | wxRIGHT, 1 );
	vsTopBottom->Add( m_pNowPlayingCtrl, 0, wxEXPAND | wxLEFT | wxRIGHT, 1 );

	//--- hide progress bar for the time being, and set its abort var to false ---//
	vsTopBottom->Show( m_pProgressGauge, false );

	m_pBottomPanel->SetSizer(vsTopBottom);
	//--- taylor ui ---//
	ShowPlaylistInfo();
	ShowSources();
	ShowAlbumArt();
	SetStayOnTop(( bool )wxGetApp().Prefs.bStayOnTop);

	CreateMainMenu();

	//--- restore placement or use defaults ---//
	g_DisablePlacement = false;

	wxGetApp().Player.SetPlaymode(wxGetApp().Prefs.ePlaymode);

	//--- update database information, then set sound volume ---//
	wxGetApp().Player.SetVolume();

	SetActiveThread( NULL );

#ifdef __WXMSW__
	SetMMShellHook((HWND)GetHWND());
#endif
#ifdef __WXGTK__
	XF86AudioKeyGrab_init();
#endif

}

MusikFrame::~MusikFrame()
{
#ifdef __WXGTK__
	XF86AudioKeyGrab_cleanup();
#endif

#ifdef wxHAS_TASK_BAR_ICON
  delete m_pTaskBarIcon;
#endif 
}
void MusikFrame::CreateMainMenu()
{
	//------------------//
	//--- menu stuff ---//
	//------------------//
	//--- file ---//
	wxMenu* file_menu = new wxMenu;
	file_menu->Append( wxID_PREFERENCES, _("&Preferences\tCtrl-P") );
	file_menu->AppendSeparator();
	file_menu->Append( wxID_EXIT, _("E&xit") );

	//--- view ---//
	wxMenu* view_menu = new wxMenu;
	view_menu->Append	( MUSIK_MENU_SOURCES_STATE,	_("Show Sources\tCtrl-1"), wxT(""), wxITEM_CHECK );
	view_menu->Append	( MUSIK_MENU_ACTIVITIES_STATE, _("Show Selections\tCtrl-2"), wxT(""), wxITEM_CHECK );
	view_menu->Append	( MUSIK_MENU_PLAYLISTINFO_STATE, _("Show Playlist Info\tCtrl-3"), wxT(""), wxITEM_CHECK );
	view_menu->Append	( MUSIK_MENU_ALBUMART_STATE, _("Show Album Info\tCtrl-4"), wxT(""), wxITEM_CHECK );
	view_menu->Append	( MUSIK_MENU_NOWPLAYINGCONTROL_ONTOP_STATE, _("Show Playing Control on top\tCtrl-5"), wxT(""), wxITEM_CHECK );
	view_menu->AppendSeparator();
	view_menu->Append	( MUSIK_MENU_SELECT_SOURCES_LIBRARY, _("Select Library\tCtrl-F1"), wxT("") );
	view_menu->Append	( MUSIK_MENU_SELECT_SOURCES_NOWPLAYING, _("Select Now Playing\tCtrl-F2"), wxT(""));
	view_menu->AppendSeparator();
	view_menu->Append	( MUSIK_MENU_FX, _("FX\tCtrl-F") );
#ifdef __WXMSW__
	view_menu->AppendSeparator();
	view_menu->Append	( MUSIK_MENU_STAY_ON_TOP, _("Always On Top\tCtrl-T"), wxT(""), wxITEM_CHECK );
#endif

	//--- library -> pending tags ---//
	wxMenu* library_writetags_menu = new wxMenu;
	library_writetags_menu->Append( MUSIK_MENU_VIEW_DIRTY_TAGS, _("Vie&w") );
	library_writetags_menu->AppendSeparator();
	library_writetags_menu->Append( MUSIK_MENU_WRITE_TAGS, _("Write Changes to &File") );
	library_writetags_menu->Append( MUSIK_MENU_WRITE_CLEAR_DIRTY, _("Finalize for Database &Only") );

	//--- library ---//
	wxMenu *library_menu = new wxMenu;
	library_menu->Append( MUSIK_MENU_PATHS, _("&Setup Library\tCtrl-L") );
	library_menu->AppendSeparator();
	library_menu->Append( MUSIK_MENU_CUSTOMQUERY, _("&Custom Query") );
	library_menu->AppendSeparator();
	library_menu->Append( MUSIK_MENU_WRITE, _("&Pending Tags"), library_writetags_menu );
	wxMenu *help_menu = new wxMenu;
	help_menu->Append( wxID_ABOUT, wxString(_("&About")) + wxT(" ") + MUSIKAPPNAME +wxT(" ...") );
	
	//----------------//
	//--- menu bar ---//
	//----------------//
	wxMenuBar *menu_bar = new wxMenuBar;
	menu_bar->Append( file_menu,	_("&File") );
	menu_bar->Append( view_menu, 	_("&View") );
	menu_bar->Append( library_menu,	_("&Library") );
	menu_bar->Append( help_menu, _("&Help") );

	SetMenuBar( menu_bar );

}
void MusikFrame::AutoUpdate	( const wxArrayString & Filenames ,unsigned long flags )
{
	MusikLibraryDialog *p= new MusikLibraryDialog( this, Filenames,flags );
	this->Enable	( FALSE );
	p->Show	( TRUE	); 
}


//----------------------------------//
//--- Image and image list stuff ---//
//----------------------------------//
void MusikFrame::LoadImageLists()
{
	//--- null image list ---//
	g_NullImageList = new wxImageList( 1, 1, true );

	//--- sources image list ---//
	g_SourcesImages = new wxImageList( 20, 20, true );
	bmpLibrary	= wxIcon( library_xpm );
	bmpPlaylist = wxIcon( standard_xpm );
	bmpDynamic	= wxIcon( dynamic_xpm );
	bmpNowplaying =  wxIcon( nowplaying_xpm );
	bmpNetstream =  wxIcon( netstream_xpm );
	g_SourcesImages->Add( bmpLibrary );
	g_SourcesImages->Add( bmpPlaylist );
	g_SourcesImages->Add( bmpDynamic );
///// just to fill it up
	g_SourcesImages->Add( bmpPlaylist );
	g_SourcesImages->Add( bmpNetstream );
	g_SourcesImages->Add( bmpNowplaying );

	//--- ratings image list ---//
	g_RatingImages = new wxImageList( 32, 14, true );
	int rateidx = 0;
	bmpRating[rateidx++] = wxIcon( ratingminus9_xpm);
	bmpRating[rateidx++] = wxIcon( ratingminus8_xpm);
	bmpRating[rateidx++] = wxIcon( ratingminus7_xpm);
	bmpRating[rateidx++] = wxIcon( ratingminus6_xpm);
	bmpRating[rateidx++] = wxIcon( ratingminus5_xpm);
	bmpRating[rateidx++] = wxIcon( ratingminus4_xpm);
	bmpRating[rateidx++] = wxIcon( ratingminus3_xpm);
	bmpRating[rateidx++] = wxIcon( ratingminus2_xpm);
	bmpRating[rateidx++] = wxIcon( ratingminus1_xpm);
	bmpRating[rateidx++] = wxIcon( rating0_xpm);
	bmpRating[rateidx++] = wxIcon( rating1_xpm);
	bmpRating[rateidx++] = wxIcon( rating2_xpm);
	bmpRating[rateidx++] = wxIcon( rating3_xpm);
	bmpRating[rateidx++] = wxIcon( rating4_xpm);
	bmpRating[rateidx++] = wxIcon( rating5_xpm);
	bmpRating[rateidx++] = wxIcon( rating6_xpm);
	bmpRating[rateidx++] = wxIcon( rating7_xpm);
	bmpRating[rateidx++] = wxIcon( rating8_xpm);
	bmpRating[rateidx++] = wxIcon( rating9_xpm);

	wxASSERT(rateidx == (MUSIK_MAX_RATING - MUSIK_MIN_RATING) + 1);
	for(int i = 0 ;  i < (MUSIK_MAX_RATING - MUSIK_MIN_RATING) + 1;i ++)
	{
		g_RatingImages->Add( bmpRating[i] );
	}
}

void MusikFrame::DeleteImageLists()
{
	//-------------------------------------------------//
	//--- wxListCtrl::SetImageList() is supposed to	---//
	//--- NOT delete the image list, but in wxGTK	---//
	//--- it apparently does. 						---//
	//-------------------------------------------------//
	#ifdef __WXMSW__
		g_SourcesImages->RemoveAll();
		g_NullImageList->RemoveAll();
		g_RatingImages->RemoveAll();
		delete g_SourcesImages;
		delete g_NullImageList;
		delete g_RatingImages;
	#endif
}

void MusikFrame::GetFonts()
{
	wxString sFace;
	sFace =	wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT).GetFaceName();
	#if defined __WXMSW__

		g_fntSong.SetFaceName	( sFace );
		g_fntSong.SetPointSize	( 14 );

		g_fntArtist.SetFaceName	( sFace );	
		g_fntArtist.SetWeight	( wxBOLD );
		g_fntArtist.SetPointSize( 10 );

		g_fntTime.SetFaceName	( sFace );
		g_fntTime.SetWeight		( wxBOLD );
		g_fntTime.SetPointSize	( 9 );


		g_fntRegular.SetFaceName( sFace );

		g_fntBold.SetFaceName	( sFace );
		g_fntBold.SetWeight		( wxBOLD );

	#else
	
		g_fntSong.SetFaceName	( sFace );
		g_fntSong.SetPointSize	( 14 );

		g_fntArtist.SetFaceName	( sFace );	
		g_fntArtist.SetPointSize( 10 );

		g_fntTime.SetFaceName	( sFace );
		g_fntTime.SetPointSize	( 9 );


		g_fntRegular.SetFaceName( sFace );
		//g_fntRegular.SetPointSize( 9 );

		g_fntBold.SetFaceName	( sFace );
		g_fntBold.SetWeight		( wxBOLD );
		g_fntBold.SetPointSize	( 9 );
	#endif

}

void MusikFrame::GetListCtrlFont()
{
	g_fntListBold = g_PlaylistBox->PlaylistCtrl().GetFont();
	g_fntListBold.SetWeight( wxBOLD );
}

//--------------------------------------------//
//--- showing / hiding / toggling controls ---//
//--------------------------------------------//
void MusikFrame::TogglePlaylistInfo()
{
	wxGetApp().Prefs.bShowPLInfo = !wxGetApp().Prefs.bShowPLInfo;
	ShowPlaylistInfo();

}

void MusikFrame::ShowPlaylistInfo()
{
	g_PlaylistBox->ShowPlaylistInfo();
}

void MusikFrame::ShowSources()
{
	g_SourcesCtrl->Show(  ( bool )wxGetApp().Prefs.bShowSources );
	wxLayoutAlgorithm layout;
    layout.LayoutWindow(this,g_PlaylistBox);
}
void MusikFrame::ShowAlbumArt()
{
	g_SourcesCtrl->ShowAlbumArt(  ( bool )wxGetApp().Prefs.bShowAlbumArt );
	wxLayoutAlgorithm layout;
	layout.LayoutWindow(this,g_PlaylistBox);
}
void MusikFrame::ToggleSources()
{
	wxGetApp().Prefs.bShowSources = !wxGetApp().Prefs.bShowSources;

	ShowSources();
}
void MusikFrame::SetStayOnTop( bool bStayOnTop )
{
#ifdef __WXMSW__
	long style = GetWindowStyle();
	if ( bStayOnTop )
		style |= wxSTAY_ON_TOP;
	else
		style &= ~wxSTAY_ON_TOP;
	SetWindowStyle( style );
#endif
}
void MusikFrame::ShowActivityArea( bool bShow )
{
	if(g_SourcesCtrl->GetSelType() != MUSIK_SOURCES_LIBRARY)
		bShow=false;
	g_ActivityAreaCtrl->Show( bShow );
	wxLayoutAlgorithm layout;
    layout.LayoutWindow(this,g_PlaylistBox);
}

void MusikFrame::ToggleActivities()
{
	
	wxGetApp().Prefs.bShowActivities = !wxGetApp().Prefs.bShowActivities;

	ShowActivityArea( wxGetApp().Prefs.bShowActivities );
}

void MusikFrame::EnableProgress( bool enable )
{

	vsTopBottom->Show( m_pProgressGauge, enable );
	Enable( !enable );
	m_pBottomPanel->Layout();
   	m_pBottomPanel->SetDefaultSize(vsTopBottom->GetMinSize());
	wxLayoutAlgorithm layout;
    layout.LayoutWindow(this,g_PlaylistBox);
	m_pBottomPanel->Layout();
}

void MusikFrame::SetTitle(const wxString& title)
{
	wxFrame::SetTitle(wxT( "[ " ) + title + wxT( " ] " )+ wxString( MUSIKAPPNAME_VERSION ));
}
void MusikFrame::SetSongInfoText(const wxString & sSongInfoText)
{
#ifdef wxHAS_TASK_BAR_ICON
	if(	m_pTaskBarIcon )
	{
		m_pTaskBarIcon->SetIcon(wxIcon(tray_xpm), sSongInfoText);
	}
#endif
}
void MusikFrame::SetSongInfoText(const CMusikSong& song)
{
#ifdef wxHAS_TASK_BAR_ICON
	wxString sArtist = SanitizedString( ConvFromUTF8( song.MetaData.Artist ));
	wxString sTitle = SanitizedString( ConvFromUTF8( song.MetaData.Title ));
	wxString sAlbum = SanitizedString( ConvFromUTF8( song.MetaData.Album ));
	/* this is usually to large for the current wxWidgets implemetation on window */
	/* Tip text can only be 64 chars	*/
	//#ifndef __WXMSW__
//	wxString sSongInfoText(
//		wxString(_("Title")+wxT(":")+ sTitle +	wxT("\n") +
//		_("Artist")+wxT(":")+ sArtist + wxT("\n") +
//		_("Album"))+wxT(":")+ sAlbum;
//		
//		 
//
//		);
	//#else
	//	wxString sSongInfoText(
	//		sAlbum + wxT("\n") +
	//		sArtist + wxT("\n") +
	//		sTitle + wxT("\n") 
	//		);
	//#endif
	wxString sInfo;
	if(!sArtist.empty())
		sInfo += _("by ") + sArtist + wxT("\n");
	if(!sAlbum.empty())
		sInfo += wxString(_("from the album")) + wxT(" '") + sAlbum + wxT("'");
	

	if(	m_pTaskBarIcon )
	{
		m_pTaskBarIcon->SetIcon(wxIcon(tray_xpm), sTitle + wxT("\n") + sInfo );
#ifdef __WXMSW__
		if(wxGetApp().Prefs.bEnableBalloonSongInfo)
			m_pTaskBarIcon->ShowBalloonInfo(sTitle,sInfo);
#endif
	}
#endif
}

//---------------------------------------------------------//
//--- support for microsoft windows multimedia keyboard ---//
//---------------------------------------------------------//
#ifdef __WXMSW__

	#ifndef WM_APPCOMMAND
		#define WM_APPCOMMAND                   0x0319
		#define FAPPCOMMAND_MASK  0xF000
		#define GET_APPCOMMAND_LPARAM(lParam) ((short)(HIWORD(lParam) & ~FAPPCOMMAND_MASK))
		#define APPCOMMAND_MEDIA_NEXTTRACK        11
		#define APPCOMMAND_MEDIA_PREVIOUSTRACK    12
		#define APPCOMMAND_MEDIA_STOP             13
		#define APPCOMMAND_MEDIA_PLAY_PAUSE       14
	#endif

	long MusikFrame::MSWWindowProc(WXUINT message, WXWPARAM wParam, WXLPARAM lParam)
	{
		if(message == WM_APPCOMMAND)
		{
			
			switch( GET_APPCOMMAND_LPARAM( lParam ) )
			{
			case APPCOMMAND_MEDIA_NEXTTRACK:
				if( wxGetApp().Player.IsPlaying() )
					wxGetApp().Player.NextSong();
				return 1;
			case APPCOMMAND_MEDIA_PREVIOUSTRACK:
				if( wxGetApp().Player.IsPlaying() )
					wxGetApp().Player.PrevSong();
				return 1;
			case APPCOMMAND_MEDIA_STOP:
				if( wxGetApp().Player.IsPlaying() )
					wxGetApp().Player.Stop();;
				return 1;
			case APPCOMMAND_MEDIA_PLAY_PAUSE:
					wxGetApp().Player.PlayPause();
				return 1;		
			}
		}
		return wxFrame::MSWWindowProc(message,wParam,lParam);
	}	
#endif

#ifdef __WXGTK__

// support for XF86AUDIO keys on gtk
// the following code is taken from XMMS-XF86AUDIO Plugin
// and is licensed under GPL

/*
 * $Id: xf86audio.c 403 2004-10-24 07:18:08Z aqua $
 * 
 * XF86Audio keys plugin for XMMS
 *
 * This is a "general" XMMS plugin (that is, one that doesn't provide any
 * audio processing functions).  When enabled, it grabs the XF86Audio*
 * keys for play/stop/next/previous, then translates keyrelease events
 * on those keys into XMMS actions.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * Copyright (c) 2003 by Devin Carraway <xf86audio-plugin@devin.com>.
 *
 */


#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <gdk/gdk.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <string.h>
#undef _
#define _(string) (string)
enum xf86audio_value {
       XF86AUDIO_PLAY = 0,
//       XF86AUDIO_PAUSE,
       XF86AUDIO_STOP,
       XF86AUDIO_NEXT,
       XF86AUDIO_PREV,
//       XF86AUDIO_RAISEVOLUME,
//       XF86AUDIO_LOWERVOLUME,
//       XF86AUDIO_MUTE,
//       XF86AUDIO_MEDIA,
       XF86AUDIO_MAX
};

static KeyCode map[XF86AUDIO_MAX];

//enum onplay_value {
//	ONPLAY_PAUSE,
//	ONPLAY_RESTART
//};

static void grab_keys();
static void ungrab_keys();
static GdkFilterReturn xf86audio_filter(GdkXEvent *xevent, GdkEvent *event, gpointer data);



static void XF86AudioKeyGrab_init()
{
	gdk_window_add_filter(GDK_ROOT_PARENT(), xf86audio_filter, map);
	grab_keys();
}

static void XF86AudioKeyGrab_cleanup()
{
	ungrab_keys();
	gdk_window_remove_filter(NULL, xf86audio_filter, map);
}




static GdkFilterReturn xf86audio_filter(GdkXEvent *xevent, GdkEvent *event, gpointer data)
{
	XEvent *xev = (XEvent *)xevent;
	XKeyEvent *keyevent = (XKeyEvent *)xevent;
	KeyCode *k = (KeyCode *)data;
	gint i;

//	if (xev->type != KeyRelease) // key release event is not received if wxMusik is the active app. dont know why.
	if (xev->type != KeyPress)
		return GDK_FILTER_CONTINUE;

	for (i=0; i<XF86AUDIO_MAX; i++) {
		if (k[i] == keyevent->keycode)
			break;
	}
	if (i == XF86AUDIO_MAX) {
		g_warning(_("Received KeyRelease event for unrequested keycode %d"),
				keyevent->keycode);
		return GDK_FILTER_CONTINUE;
	}

	switch (i) {
		case XF86AUDIO_STOP:
			if( wxGetApp().Player.IsPlaying() )
					wxGetApp().Player.Stop();
			break;
		case XF86AUDIO_PREV:
			if( wxGetApp().Player.IsPlaying() )
					wxGetApp().Player.PrevSong();
			break;
		case XF86AUDIO_NEXT:
			if( wxGetApp().Player.IsPlaying() )
					wxGetApp().Player.NextSong();
			break;
		case XF86AUDIO_PLAY:
			wxGetApp().Player.PlayPause();
			break;
		default: return GDK_FILTER_CONTINUE;
	}
	return GDK_FILTER_REMOVE;
}


static KeyCode grab_key(char *keystring)
{
	KeySym sym;
	KeyCode code;
	gint i;

	if ((sym = XStringToKeysym(keystring)) == NoSymbol)
		return 0;
	if ((code = XKeysymToKeycode(GDK_DISPLAY(), sym)) == 0)
		return 0;

	gdk_error_trap_push();
	for (i = 0; i < ScreenCount(GDK_DISPLAY()); i++) {
		XGrabKey(GDK_DISPLAY(), code,
				AnyModifier, RootWindow(GDK_DISPLAY(),i),
				1, GrabModeAsync, GrabModeAsync);
	}

	gdk_flush();
	if (gdk_error_trap_pop()) {
		g_warning(_("Couldn't grab %s: another client may already have done so"),
				keystring);
		return 0;
	}
	return code;
}

static void grab_keys()
{
	KeyCode code;
	
	if ((code = grab_key("XF86AudioNext")) != 0)
		map[XF86AUDIO_NEXT] = code;
	if ((code = grab_key("XF86AudioPrev")) != 0)
		map[XF86AUDIO_PREV] = code;
	if ((code = grab_key("XF86AudioPlay")) != 0)
		map[XF86AUDIO_PLAY] = code;
	if ((code = grab_key("XF86AudioStop")) != 0)
		map[XF86AUDIO_STOP] = code;
/*	if ((code = grab_key("XF86AudioPause")) != 0)
		map[XF86AUDIO_PAUSE] = code;
	if ((code = grab_key("XF86AudioRaiseVolume")) != 0)
		map[XF86AUDIO_RAISEVOLUME] = code;
	if ((code = grab_key("XF86AudioLowerVolume")) != 0)
		map[XF86AUDIO_LOWERVOLUME] = code;
	if ((code = grab_key("XF86AudioMute")) != 0)
		map[XF86AUDIO_MUTE] = code;
	if ((code = grab_key("XF86AudioMedia")) != 0)
		map[XF86AUDIO_MEDIA] = code;
*/
}


static void ungrab_key(KeyCode code)
{
	int i;

	gdk_error_trap_push();
	for (i = 0; i < ScreenCount(GDK_DISPLAY()); i++)
		XUngrabKey(GDK_DISPLAY(), code,
				AnyModifier, RootWindow(GDK_DISPLAY(),i));
	gdk_flush();
	if (gdk_error_trap_pop())
		g_warning(_("Couldn't ungrab keycode %d"), code);
}

static void ungrab_keys()
{
	int i;

	for (i = 0; i < XF86AUDIO_MAX; i++)
		if (map[i] != 0) {
			ungrab_key(map[i]);
			map[i] = 0;
		}
}
#endif //WXGTK

