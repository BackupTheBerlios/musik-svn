/*
 *  NowPlayingCtrl.h
 *
 *  The "Now Playing" information
 *	This is the control visible on the very bottom of the frame
 *	Includes currently playing songs and playlist navigation
 *
 *  Copyright (c) 2003 Casey Langen (casey@bak.rr.com)
 *	Contributors: Simon Windmill, Dustin Carter, Gunnar Roth, Wade Brainerd
 *
 *  See the file "license.txt" for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL WARRANTIES.
*/
// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"
//--- globals ---//
#include "NowPlayingCtrl.h"
#include "Tunage.h"
#include "../MusikGlobals.h"
#include "../MusikUtils.h"

//--- frames ---//
#include "../Frames/MusikFrame.h"
#include "../Frames/MusikVolumeFrame.h"
//--- images ---//
#include "../images/playback/next.xpm"
#include "../images/playback/next_down.xpm"
#include "../images/playback/pause.xpm"
#include "../images/playback/pause_down.xpm"
#include "../images/playback/play.xpm"
#include "../images/playback/play_down.xpm"
#include "../images/playback/prev.xpm"
#include "../images/playback/prev_down.xpm"
#include "../images/playback/stop.xpm"
#include "../images/playback/stop_down.xpm"
#include "../images/playback/volume.xpm"
#include "../images/playback/volume_down.xpm"





BEGIN_EVENT_TABLE(CNowPlayingCtrl, wxPanel)
	EVT_BUTTON	(MUSIK_NOWPLAYINGCTRL_STOP,				CNowPlayingCtrl::PlayerStop				)	// stop button pressed
	EVT_BUTTON	(MUSIK_NOWPLAYINGCTRL_PLAYPAUSE,		CNowPlayingCtrl::PlayerPlayPause		)	// play/pause/resume perssed
	EVT_BUTTON	(MUSIK_NOWPLAYINGCTRL_NEXT,				CNowPlayingCtrl::PlayerNext				)	// next button pressed
	EVT_BUTTON	(MUSIK_NOWPLAYINGCTRL_PREV,				CNowPlayingCtrl::PlayerPrev				)	// prev button pressed
	EVT_BUTTON	(MUSIK_NOWPLAYINGCTRL_VOLUME,			CNowPlayingCtrl::PlayerVolume			)	// prev button pressed
	EVT_TIMER	(MUSIK_NOWPLAYING_TIMER,				CNowPlayingCtrl::OnTimer				)	// timer
	EVT_CHOICE	(MUSIK_NOWPLAYINGCTRL_PLAYMODE,			CNowPlayingCtrl::OnPlayMode) 
	EVT_CHECKBOX(MUSIK_CHK_CROSSFADE,					CNowPlayingCtrl::OnCheckCrossfade)
	
#ifdef wxUSE_HOTKEY
	EVT_HOTKEY(MUSIK_HOTKEYID_STOP, CNowPlayingCtrl::PlayerStopHK)
	EVT_HOTKEY(MUSIK_HOTKEYID_PLAYPAUSE, CNowPlayingCtrl::PlayerPlayPauseHK)
	EVT_HOTKEY(MUSIK_HOTKEYID_PREV, CNowPlayingCtrl::PlayerPrevHK)
	EVT_HOTKEY(MUSIK_HOTKEYID_NEXT, CNowPlayingCtrl::PlayerNextHK)

#endif
END_EVENT_TABLE()

CNowPlayingCtrl::CNowPlayingCtrl( wxWindow *parent )
	: wxPanel( parent, -1, wxDefaultPosition, wxDefaultSize, wxCLIP_CHILDREN| wxTAB_TRAVERSAL |wxFULL_REPAINT_ON_RESIZE)
{
	//-----------------------------//
	//--- title / artist / time ---//
	//-----------------------------//
	//--- now playing static text objects ---//
	stSong			= new wxStaticText_NoFlicker	( this, -1, wxT(""),	wxDefaultPosition, wxDefaultSize, 0 );
	stArtist		= new wxStaticText_NoFlicker	( this, -1, wxT(""),	wxDefaultPosition, wxDefaultSize, 0 );
	stCurtime		= new wxStaticText_NoFlicker	( this,MUSIK_NOWPLAYINGCTRL_TIMEDISPLAY, wxT(""),	wxDefaultPosition, wxDefaultSize, 0 );	

	//--- fonts ---//
	stSong->SetFont		( g_fntSong );
	stArtist->SetFont	( g_fntArtist );
	stCurtime->SetFont	( g_fntTime );

	//---------------------------//
	//--- buttons and bitmaps ---//
	//---------------------------//
	//--- bitmaps ---//
	bmPrev			= wxBitmap( prev_xpm );
	bmPrevDown		= wxBitmap( prev_down_xpm );
	bmNext			= wxBitmap( next_xpm );
	bmNextDown		= wxBitmap( next_down_xpm );
	bmPlay			= wxBitmap( play_xpm );
	bmPlayDown		= wxBitmap( play_down_xpm );
	bmPause			= wxBitmap( pause_xpm );
	bmPauseDown		= wxBitmap( pause_down_xpm );
	bmStop			= wxBitmap( stop_xpm );
	bmStopDown		= wxBitmap( stop_down_xpm );
	bmVolume		= wxBitmap( volume_xpm );
	bmVolumeDown	= wxBitmap( volume_down_xpm );

	//--- buttons ---//
	btnPrev			= new wxBitmapButton( this, MUSIK_NOWPLAYINGCTRL_PREV, bmPrev, wxDefaultPosition, wxSize( 40, -1 ), 0 );
	btnNext			= new wxBitmapButton( this, MUSIK_NOWPLAYINGCTRL_NEXT, bmNext, wxDefaultPosition, wxSize( 40, -1 ), 0 );
	btnPlayPause	= new wxBitmapButton( this, MUSIK_NOWPLAYINGCTRL_PLAYPAUSE, bmPlay, wxDefaultPosition, wxSize( 40, -1 ), 0 );	
	btnStop			= new wxBitmapButton( this, MUSIK_NOWPLAYINGCTRL_STOP, bmStop, wxDefaultPosition, wxSize( 40, -1), 0 );
	btnVolume		= new wxBitmapButton( this, MUSIK_NOWPLAYINGCTRL_VOLUME, bmVolume, wxDefaultPosition, wxSize( 40, -1), 0 );

	//--- events ---//
	btnPrev->SetBitmapLabel				( bmPrev );
	btnNext->SetBitmapLabel				( bmNext );
	btnPlayPause->SetBitmapLabel		( bmPlay );
	btnStop->SetBitmapLabel				( bmStop );
	btnVolume->SetBitmapLabel			( bmVolume );
	#ifdef __WXGTK__
	btnPrev->SetBitmapFocus				( bmPrevDown );
	btnNext->SetBitmapFocus				( bmNextDown );
	btnPlayPause->SetBitmapFocus		( bmPlayDown );
	btnStop->SetBitmapFocus				( bmStopDown );
	btnVolume->SetBitmapFocus			( bmVolumeDown );
	#else // defined __WXMSW__
	pPrevEvt	= new CBtnDownEvt	( btnPrev, &bmPrevDown, &bmPrev );
	pNextEvt	= new CBtnDownEvt	( btnNext, &bmNextDown, &bmNext );
	pPlayEvt	= new CBtnDownEvt	( btnPlayPause, &bmPlayDown, &bmPlay );
	pPauseEvt	= new CBtnDownEvt	( btnPlayPause, &bmPauseDown, &bmPause );
	pStopEvt	= new CBtnDownEvt	( btnStop, &bmStopDown, &bmStop );
	pVolumeEvt	= new CBtnDownEvt	( btnVolume, &bmVolumeDown, &bmVolume );
    btnPrev->PushEventHandler		( pPrevEvt );
	btnNext->PushEventHandler		( pNextEvt );
	btnPlayPause->PushEventHandler	( pPlayEvt );
	btnStop->PushEventHandler		( pStopEvt );
	btnVolume->PushEventHandler		( pVolumeEvt );
	#endif

	//----------------//
	//--- seek bar ---//
	//----------------//
	gSeek		= new wxGauge		( this, MUSIK_NOWPLAYINGCTRL_SEEK, 100, wxDefaultPosition, wxSize( 12* wxSystemSettings::GetMetric( wxSYS_HSCROLL_Y ), wxSystemSettings::GetMetric( wxSYS_HSCROLL_Y ) ), wxGA_SMOOTH | wxGA_HORIZONTAL | wxCLIP_CHILDREN );
	pSeekEvt	= new CGaugeSeekEvt	( gSeek );
	gSeek->PushEventHandler( pSeekEvt );

	//---------------------------------//
	//--- sizer for artist and time ---//
	//---------------------------------//
	hsArtistTime = new wxBoxSizer( wxHORIZONTAL );
	hsArtistTime->Add( stArtist, 0,  wxALIGN_CENTRE_VERTICAL  );
	hsArtistTime->Add( stCurtime, 1,  wxALIGN_CENTRE_VERTICAL|wxLEFT,3 );

	//--- song title, artist and time ---//
	vsLeftCol = new wxBoxSizer( wxVERTICAL );
	vsLeftCol->Add( stSong, 1,  wxBOTTOM|wxEXPAND , 3 );
	vsLeftCol->Add( hsArtistTime, 1, wxEXPAND );


	//--- buttons, seek bar panel ---//
	wxGridSizer *hsButtons = new wxGridSizer(1,5,2,2);
	hsButtons->Add( btnPrev ,0,wxALIGN_CENTRE_HORIZONTAL | wxLEFT ,10);
	hsButtons->Add( btnPlayPause ,0,wxALIGN_CENTRE_HORIZONTAL | wxLEFT ,10);
	hsButtons->Add( btnStop ,0,wxALIGN_CENTRE_HORIZONTAL | wxLEFT ,10);
	hsButtons->Add( btnNext ,0,wxALIGN_CENTRE_HORIZONTAL | wxLEFT ,10);
	hsButtons->Add( btnVolume,0,wxALIGN_CENTRE_HORIZONTAL | wxLEFT ,10);

	vsRightCol = new wxBoxSizer( wxVERTICAL );

	//--- playmode ---//
	wxBoxSizer *vsPlayModeCol = new wxBoxSizer( wxHORIZONTAL );
	vsPlayModeCol->Add( hsButtons, 1,wxEXPAND|wxALL,2 );

//	vsPlayModeCol->Add( new wxStaticText	( this, -1, _( "Play mode" ) ), 1 );
	const wxString playmode_choices[] ={_("Normal"),_("Repeat Song"),_("Repeat List"),_("Shuffle"),_("Auto DJ"),_("Auto DJ Album")};
	
	wxChoice *choicePlaymode = new wxChoice_NoFlicker(this,MUSIK_NOWPLAYINGCTRL_PLAYMODE,wxDefaultPosition,wxDefaultSize,WXSIZEOF(playmode_choices),playmode_choices);
	int playmode = wxGetApp().Prefs.ePlaymode.val;
	choicePlaymode->SetSelection(playmode);
	vsPlayModeCol->Add( choicePlaymode,0, wxRIGHT|wxLEFT|wxALIGN_CENTRE_VERTICAL, 5 ); //-- small top border --//
	wxCheckBox * pCrossfade = new wxCheckBox_NoFlicker( this, MUSIK_CHK_CROSSFADE, _("Crossfade"), wxPoint( -1, -1 ), wxSize( -1, -1 ) );
	vsPlayModeCol->Add( pCrossfade,0, wxALIGN_CENTRE_VERTICAL|wxRIGHT, 2 ); //-- small top border --//
	pCrossfade->SetValue( wxGetApp().Prefs.bGlobalFadeEnable );
  	vsRightCol->Add( vsPlayModeCol, 0 ); //-- small top border --//
	vsRightCol->Add( gSeek, 0, wxTOP|wxEXPAND, 2 ); //-- small top border --//



//	wxGridSizer *hsCols = new wxGridSizer( 1,2,0,0);
	wxBoxSizer *hsCols = new wxBoxSizer(wxHORIZONTAL);
	hsCols->Add( vsLeftCol,	1,  wxALL, 2	);
	//hsCols->Add(-1,-1,1,wxEXPAND);
	hsCols->Add( vsRightCol,	0, wxALIGN_RIGHT| wxALL, 2	);

	SetSizer( hsCols );

	m_pTunage = new CTunage;
	pSecTimer = NULL;
	StartTimer();
	g_TimeSeeking = false;
	
	ResetInfo();
	
	ActivateHotkeys();
}
static bool s_bHotKeyRegistered = false;
void CNowPlayingCtrl::ActivateHotkeys()
{
	if(s_bHotKeyRegistered)
		return;
#ifdef wxUSE_HOTKEY
	if( wxGetApp().Prefs.bEnablePlayerHotkeys == false)
		return;

	DeactivateHotkeys();
	for(int i = MUSIK_HOTKEYID_FIRST; i <= MUSIK_HOTKEYID_LAST;i++)
	{
		int modifier=0;
		int keycode = 0;
		bool bRes = false;
		wxString sHotKey = wxGetApp().Prefs.sPlayerHotkeys[i-MUSIK_HOTKEYID_FIRST];
		wxArrayString ModnKey;
		DelimitStr(sHotKey,wxT("+"),ModnKey);
		if(ModnKey.GetCount() == 2)
		{
			ModnKey[0].Trim();
			ModnKey[0].Trim(FALSE);

			wxArrayString Modn;
			DelimitStr(ModnKey[0],wxT(" "),Modn);
			for(size_t i = 0; i< Modn.GetCount();i++)
			{
				bRes = true;
				if(Modn[i] == wxT("ALT"))
					modifier |= wxMOD_ALT;
				else if(Modn[i] == wxT("SHIFT"))
					modifier |= wxMOD_SHIFT;
				else if(Modn[i] == wxT("CTRL"))
					modifier |= wxMOD_CONTROL;
				else if(Modn[i] == wxT("WIN"))
					modifier |= wxMOD_WIN;
				else 
				{
					bRes = false;
					break;
				}
			}
			if(bRes)
			{
				
				ModnKey[1].Trim();
				ModnKey[1].Trim(FALSE);
				ModnKey[1].MakeUpper();
				if(ModnKey[1].Len() == 1)
				{
					keycode= (int)ModnKey[1].GetChar(0);
				}
				else if(ModnKey[1].Len() > 1)
				{
					if(ModnKey[1] == wxT("SPACE"))
						keycode = WXK_SPACE;
					else if(ModnKey[1].StartsWith(wxT("F")))
					{
						long fkey = 0;
						if(ModnKey[1].Right(ModnKey[1].Len()-1).ToLong(&fkey))
						{
							keycode = fkey + WXK_F1 - 1;
						}
					}
					else
						bRes = false;
				}
				else
					bRes = false;
			}
		}
        
		if(bRes)
			bRes = RegisterHotKey(i,modifier,keycode);
		if(!bRes)
			::wxLogWarning(_("Hotkey %s cannot be registered."),(const wxChar*)sHotKey);
	}
	s_bHotKeyRegistered = true;

#endif
}
void CNowPlayingCtrl::DeactivateHotkeys()
{
	if(!s_bHotKeyRegistered)
		return;
#ifdef wxUSE_HOTKEY

	for(int i = MUSIK_HOTKEYID_FIRST; i <= MUSIK_HOTKEYID_LAST;i++)
		UnregisterHotKey(i);
#endif
}


CNowPlayingCtrl::~CNowPlayingCtrl()
{

	DeactivateHotkeys();
	//--- stop timer ---//
	KillTimer();

	#ifndef __WXGTK__
    btnPrev->PopEventHandler();
	btnNext->PopEventHandler();
	btnPlayPause->PopEventHandler();
	btnStop->PopEventHandler();
	btnVolume->PopEventHandler();
	delete pPrevEvt;
	delete pNextEvt;
	delete pPlayEvt;
	delete pPauseEvt;
	delete pStopEvt;
	delete pVolumeEvt;
	#endif

	gSeek->PopEventHandler();
	delete pSeekEvt;
	delete m_pTunage;
}

//-------------------//
//--- timer stuff ---//
//-------------------//
void CNowPlayingCtrl::OnTimer( wxTimerEvent& WXUNUSED(event) )
{
	if ( wxGetApp().Player.IsPlaying() && !wxGetApp().Player.IsStopping() )
		UpdateTime();
}

void CNowPlayingCtrl::StartTimer()
{
	if ( pSecTimer == NULL )
	{
		pSecTimer = new wxTimer( this, MUSIK_NOWPLAYING_TIMER );
		pSecTimer->Start( 1000, false );
	}
}

void CNowPlayingCtrl::KillTimer()
{
	if ( pSecTimer )
	{
		pSecTimer->Stop();
		delete pSecTimer;
	}
}

void CNowPlayingCtrl::PlayBtnToPauseBtn()
{
	#ifdef __WXGTK__
	btnPlayPause->SetBitmapLabel( bmPause );
	btnPlayPause->SetBitmapFocus( bmPauseDown );
	#else //defined __WXMSW__
	btnPlayPause->SetBitmapLabel( bmPause );
	btnPlayPause->PopEventHandler();
	btnPlayPause->PushEventHandler( pPauseEvt );
	#endif
	btnPlayPause->Refresh();
}

void CNowPlayingCtrl::PauseBtnToPlayBtn()
{
	#ifdef __WXGTK__
	btnPlayPause->SetBitmapLabel( bmPlay );
	btnPlayPause->SetBitmapFocus( bmPlayDown );
	#else //if defined __WXMSW__
	btnPlayPause->SetBitmapLabel( bmPlay );
	btnPlayPause->PopEventHandler();
	btnPlayPause->PushEventHandler( pPlayEvt );
	#endif
	btnPlayPause->Refresh();
}

static int nTimeDisplayMode = 0;

void CNowPlayingCtrl::OnClickTimeDisplay(wxMouseEvent & WXUNUSED(event))
{
	
	//if(stCurtime && stCurtime->GetRect().Inside(event.GetPosition()))
	{
		nTimeDisplayMode++;
		nTimeDisplayMode %= 2;
	}
}
void CNowPlayingCtrl::UpdateTime()
{

	if ( !g_TimeSeeking )
	{
	 	float fPos = (float)100* ( (float)wxGetApp().Player.GetTime( FMOD_SEC ) / (float)wxGetApp().Player.GetDuration( FMOD_SEC ) );
	    
		//--- now, if we're in gtk and we set the wxGauge 	---//
		//--- to a value below 2.0, it changes to 100%		---//
		//--- why? who fucking knows. i hope they fix this	---//
		#ifdef __WXGTK__
		if ( fPos < 2.0f )
			fPos = 2.0f;
		#endif
		
		gSeek->SetValue( (int)fPos );
		
		//--- time label ---//
		stCurtime->SetLabel( wxT( " - " ) + ((nTimeDisplayMode== 0) ? wxGetApp().Player.GetTimeStr():wxGetApp().Player.GetTimeLeftStr()) );
		Layout();
	}
}

void CNowPlayingCtrl::SetTime( wxString sTimeStr )
{
	stCurtime->SetLabel( wxT( " - " ) + sTimeStr );
}

void CNowPlayingCtrl::ResetInfo()
{
	stSong->SetLabel	( _( "Playback Stopped" )	);
	stArtist->SetLabel	( _( "Playback Stopped" )	);
	stCurtime->SetLabel	( wxT( " - 0:00" )			);
	#ifndef __WXGTK__
		gSeek->SetValue( 0 );
	#else
		gSeek->SetValue( 1 );
	#endif

	m_pTunage->Stopped();

	Layout();
}

void CNowPlayingCtrl::UpdateInfo( const CMusikSong &song )
{
	//--- first things first, verify data in song ---//
	wxString sArtist = SanitizedString( ConvFromUTF8( song.MetaData.Artist ));
	wxString sTitle = SanitizedString( ConvFromUTF8( song.MetaData.Title ));
	wxString sAlbum = SanitizedString( ConvFromUTF8( song.MetaData.Album ));
	if ( sArtist.IsEmpty())
		sArtist = _( "Unknown Artist" );
	if ( sTitle.IsEmpty() )
		sTitle = _( "Unknown Song" );

	// tell Tunage to do it's thing if file has changed
	if ( m_LastFile != song.MetaData.Filename )
		m_pTunage->Execute( song );

	m_LastFile = song.MetaData.Filename;

	//--- caption bar title ---//
	g_MusikFrame->SetTitle( sArtist + wxT( " - " ) +  sTitle + (!sAlbum.IsEmpty() ? wxString(wxT( " - " )) + sAlbum : wxString()) );
	g_MusikFrame->SetSongInfoText( song );

	//--- title / artist / time -//
	sTitle.Replace	( wxT( "&" ), wxT( "&&" ), TRUE );
	sArtist.Replace	( wxT( "&" ), wxT( "&&" ), TRUE );

	if ( sTitle != stSong->GetLabel() )
		stSong->SetLabel( sTitle );
	if ( sArtist != stArtist->GetLabel() )
		stArtist->SetLabel( sArtist );
	UpdateTime();

	Layout();
}

void CNowPlayingCtrl::PlayerStop( wxCommandEvent& WXUNUSED(event) )
{	
	wxGetApp().Player.Stop();
}

void CNowPlayingCtrl::PlayerPlayPause( wxCommandEvent& WXUNUSED(event) )	
{	
	wxGetApp().Player.PlayPause();
}

void CNowPlayingCtrl::PlayerNext( wxCommandEvent& WXUNUSED(event) )	
{	
	wxGetApp().Player.NextSong();
}

void CNowPlayingCtrl::PlayerPrev( wxCommandEvent& WXUNUSED(event) )	
{	
	wxGetApp().Player.PrevSong();
}

void CNowPlayingCtrl::PlayerVolume( wxCommandEvent& WXUNUSED(event) )	
{	
	wxPoint pos = wxGetMousePosition();
	wxFrame *pDlg = new MusikVolumeFrame( ( wxFrame* )g_MusikFrame, pos );

	wxSize DlgSize = pDlg->GetSize();
	pos.x -= DlgSize.GetWidth();
	pos.y -= DlgSize.GetHeight();	
	pDlg->Move( pos );
	pDlg->Show();
}

void CNowPlayingCtrl::OnPlayMode( wxCommandEvent&	event )
{
	int modesel = event.GetSelection();
	wxGetApp().Prefs.ePlaymode = (EMUSIK_PLAYMODE)modesel;
	wxGetApp().Player.SetPlaymode(wxGetApp().Prefs.ePlaymode);
}
void CNowPlayingCtrl::OnCheckCrossfade	( wxCommandEvent&	event )
{
	wxGetApp().Prefs.bGlobalFadeEnable = event.IsChecked();
}
