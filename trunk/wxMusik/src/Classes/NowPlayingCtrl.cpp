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
#include "myprec.h"
//--- globals ---//
#include "NowPlayingCtrl.h"
#include "Tunage.h"
#include "MusikGlobals.h"
#include "Playlist.h"

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
	
#if wxUSE_HOTKEY
	EVT_HOTKEY(MUSIK_HOTKEYID_STOP, CNowPlayingCtrl::PlayerStopHK)
	EVT_HOTKEY(MUSIK_HOTKEYID_PLAYPAUSE, CNowPlayingCtrl::PlayerPlayPauseHK)
	EVT_HOTKEY(MUSIK_HOTKEYID_PREV, CNowPlayingCtrl::PlayerPrevHK)
	EVT_HOTKEY(MUSIK_HOTKEYID_NEXT, CNowPlayingCtrl::PlayerNextHK)

#endif
END_EVENT_TABLE()

#define LOAD_BUTTONPNG(Name) \
    if(!bm##Name.LoadFile(MusikGetStaticDataPath() + wxT("playbackart/") MUSIK_STRINGIZE_T(Name) wxT(".png"),wxBITMAP_TYPE_PNG))\
        bm##Name = wxBitmap( Name##_xpm);\
    if(!bm##Name##Down.LoadFile(MusikGetStaticDataPath() + wxT("playbackart/") MUSIK_STRINGIZE_T(Name) wxT("_down.png"),wxBITMAP_TYPE_PNG))\
        bm##Name = wxBitmap( Name##_down_xpm);

CNowPlayingCtrl::CNowPlayingCtrl( wxWindow *parent ,CMusikPlayer & refMusikPlayer)
	: wxPanel( parent, -1, wxDefaultPosition, wxDefaultSize, wxCLIP_CHILDREN| wxTAB_TRAVERSAL )
    ,m_MusikPlayer(refMusikPlayer)
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
    {
         wxLogNull lognull; // disable logging in this scope
         LOAD_BUTTONPNG(Prev);
         LOAD_BUTTONPNG(Play);
         LOAD_BUTTONPNG(Pause);
         LOAD_BUTTONPNG(Stop);
         LOAD_BUTTONPNG(Next);
         LOAD_BUTTONPNG(Volume);
    }

    wxPanel * pPlayPanel = new wxPanel(this);

	//--- buttons ---//
	btnPrev			= new wxBitmapButton( pPlayPanel, MUSIK_NOWPLAYINGCTRL_PREV, bmPrev ,wxDefaultPosition,wxDefaultSize,0);
	btnNext			= new wxBitmapButton( pPlayPanel, MUSIK_NOWPLAYINGCTRL_NEXT, bmNext,wxDefaultPosition,wxDefaultSize,0);
	btnPlayPause	= new wxBitmapButton( pPlayPanel, MUSIK_NOWPLAYINGCTRL_PLAYPAUSE, bmPlay,wxDefaultPosition,wxDefaultSize,0);	
	btnStop			= new wxBitmapButton( pPlayPanel, MUSIK_NOWPLAYINGCTRL_STOP, bmStop,wxDefaultPosition,wxDefaultSize,0);
	btnVolume		= new wxBitmapButton( pPlayPanel, MUSIK_NOWPLAYINGCTRL_VOLUME, bmVolume,wxDefaultPosition,wxDefaultSize,0);

	//--- events ---//
	#ifndef __WXMSW__
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
	gSeek		= new wxGauge		( pPlayPanel, MUSIK_NOWPLAYINGCTRL_SEEK, 100, wxDefaultPosition, wxSize( 4* wxSystemSettings::GetMetric( wxSYS_HSCROLL_Y ), wxSystemSettings::GetMetric( wxSYS_HSCROLL_Y ) ), wxGA_SMOOTH | wxGA_HORIZONTAL | wxCLIP_CHILDREN );
	pSeekEvt	= new CGaugeSeekEvt	( gSeek );
	gSeek->PushEventHandler( pSeekEvt );

    gSeek->Connect(wxEVT_RIGHT_DOWN,wxMouseEventHandler(CNowPlayingCtrl::OnClickTimeDisplay),NULL,this);
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
	wxGridSizer *hsButtons = new wxGridSizer(5);
	hsButtons->Add( btnPrev );
	hsButtons->Add( btnPlayPause );
	hsButtons->Add( btnStop );
	hsButtons->Add( btnNext );
	hsButtons->Add( btnVolume);

	vsRightCol = new wxBoxSizer( wxVERTICAL );

	//--- playmode ---//
	wxBoxSizer *vsPlayModeCol = new wxBoxSizer( wxHORIZONTAL );
	vsPlayModeCol->Add( hsButtons);

//	vsPlayModeCol->Add( new wxStaticText	( this, -1, _( "Play mode" ) ), 1 );
	const wxString playmode_choices[] ={_("Normal"),_("Repeat Song"),_("Repeat List"),_("Shuffle"),_("Auto DJ"),_("Auto DJ Album")};
	
	wxChoice *choicePlaymode = new wxChoice_NoFlicker(pPlayPanel,MUSIK_NOWPLAYINGCTRL_PLAYMODE,wxDefaultPosition,wxDefaultSize,WXSIZEOF(playmode_choices),playmode_choices);
	int playmode = wxGetApp().Prefs.ePlaymode.val;
	choicePlaymode->SetSelection(playmode);
	vsPlayModeCol->Add( choicePlaymode,0, wxRIGHT|wxLEFT|wxALIGN_CENTRE_VERTICAL, 5 ); //-- small top border --//
	wxCheckBox * pCrossfade = new wxCheckBox_NoFlicker( pPlayPanel, MUSIK_CHK_CROSSFADE, _("Crossfade"));
	vsPlayModeCol->Add( pCrossfade,0, wxALIGN_CENTRE_VERTICAL|wxRIGHT, 2 ); //-- small top border --//
	pCrossfade->SetValue( wxGetApp().Prefs.bGlobalFadeEnable );
  	vsRightCol->Add( vsPlayModeCol, 0 ); //-- small top border --//
	vsRightCol->Add( gSeek, 0, wxTOP|wxEXPAND, 2 ); //-- small top border --//

    pPlayPanel->SetSizer(vsRightCol);

//	wxGridSizer *hsCols = new wxGridSizer( 1,2,0,0);
	wxBoxSizer *hsCols = new wxBoxSizer(wxHORIZONTAL);
	hsCols->Add( vsLeftCol,	1,  wxALL, 2	);
	//hsCols->Add(-1,-1,1,wxEXPAND);
	hsCols->Add(pPlayPanel,	0, wxALIGN_RIGHT| wxALL, 2	);

	SetSizer( hsCols );

	m_pTunage = new CTunage;
	pSecTimer = NULL;
	StartTimer();
	g_TimeSeeking = false;
	
	ResetInfo();
	
    m_MusikPlayer.Connect(wxEVT_MUSIKPLAYER_SONG_CHANGED,MusikPlayerEventHandler(CNowPlayingCtrl::OnSongChanged),NULL,this);
    m_MusikPlayer.Connect(wxEVT_MUSIKPLAYER_PLAY_START,MusikPlayerEventHandler(CNowPlayingCtrl::OnPlayStart),NULL,this);
    m_MusikPlayer.Connect(wxEVT_MUSIKPLAYER_PLAY_STOP,MusikPlayerEventHandler(CNowPlayingCtrl::OnPlayStop),NULL,this);
    m_MusikPlayer.Connect(wxEVT_MUSIKPLAYER_PLAY_PAUSE,MusikPlayerEventHandler(CNowPlayingCtrl::OnPlayPause),NULL,this);
    m_MusikPlayer.Connect(wxEVT_MUSIKPLAYER_PLAY_RESUME,MusikPlayerEventHandler(CNowPlayingCtrl::OnPlayResume),NULL,this);

	ActivateHotkeys();
}
static bool s_bHotKeyRegistered = false;
void CNowPlayingCtrl::ActivateHotkeys()
{
	if(s_bHotKeyRegistered)
		return;
#if wxUSE_HOTKEY
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
#if wxUSE_HOTKEY

	for(int i = MUSIK_HOTKEYID_FIRST; i <= MUSIK_HOTKEYID_LAST;i++)
		UnregisterHotKey(i);
#endif
}


CNowPlayingCtrl::~CNowPlayingCtrl()
{

	DeactivateHotkeys();
	//--- stop timer ---//
	KillTimer();

    m_MusikPlayer.Disconnect(wxID_ANY,wxEVT_MUSIKPLAYER_SONG_CHANGED,MusikPlayerEventHandler(CNowPlayingCtrl::OnSongChanged),NULL,this);
    m_MusikPlayer.Disconnect(wxID_ANY,wxEVT_MUSIKPLAYER_PLAY_START,MusikPlayerEventHandler(CNowPlayingCtrl::OnPlayStart),NULL,this);
    m_MusikPlayer.Disconnect(wxID_ANY,wxEVT_MUSIKPLAYER_PLAY_STOP,MusikPlayerEventHandler(CNowPlayingCtrl::OnPlayStop),NULL,this);
    m_MusikPlayer.Disconnect(wxID_ANY,wxEVT_MUSIKPLAYER_PLAY_PAUSE,MusikPlayerEventHandler(CNowPlayingCtrl::OnPlayPause),NULL,this);
    m_MusikPlayer.Disconnect(wxID_ANY,wxEVT_MUSIKPLAYER_PLAY_RESUME,MusikPlayerEventHandler(CNowPlayingCtrl::OnPlayResume),NULL,this);

	#ifdef __WXMSW__
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
	if ( m_MusikPlayer.IsPlaying() && !m_MusikPlayer.IsStopping() )
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
	#ifndef __WXMSW__
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
	#ifndef __WXMSW__
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
		++nTimeDisplayMode %= 2;
}
void CNowPlayingCtrl::UpdateTime()
{

	if ( !g_TimeSeeking )
	{
        int duration = m_MusikPlayer.GetDuration( UNIT_SEC ); 
        float fPos = duration ? (float)100* ( (float)m_MusikPlayer.GetTime( UNIT_SEC ) / duration) : 0;
	    
		//--- now, if we're in gtk and we set the wxGauge 	---//
		//--- to a value below 2.0, it changes to 100%		---//
		//--- why? who fucking knows. i hope they fix this	---//
		#ifdef __WXGTK__
		if ( fPos < 2.0f )
			fPos = 2.0f;
		#endif
		
		gSeek->SetValue( (int)fPos );
		
		//--- time label ---//
		stCurtime->SetLabel( wxT( " - " ) + ((nTimeDisplayMode== 0) ? m_MusikPlayer.GetTimeStr():m_MusikPlayer.GetTimeLeftStr()) );
		Layout();
	}
}

void CNowPlayingCtrl::SetTime( wxString sTimeStr )
{
	stCurtime->SetLabel( wxT( " - " ) + sTimeStr );
}

void CNowPlayingCtrl::ResetInfo()
{
    m_CurrSong = CMusikSong();
	stSong->SetLabel	( _( "Playback Stopped" )	);
	stArtist->SetLabel	( _( "Playback Stopped" )	);
	stCurtime->SetLabel	( wxT( " - 0:00" )			);
	#ifndef __WXGTK__
		gSeek->SetValue( 0 );
	#else
		gSeek->SetValue( 1 );
	#endif
	Layout();
}

void CNowPlayingCtrl::UpdateInfo( const MusikSongId &songid )
{
    std::auto_ptr<CMusikSong> pSong = songid.Song();
	//--- first things first, verify data in song ---//
	wxString sArtist = SanitizedString( ConvFromUTF8( pSong->MetaData.Artist ));
	wxString sTitle = SanitizedString( ConvFromUTF8( pSong->MetaData.Title ));
	wxString sAlbum = SanitizedString( ConvFromUTF8( pSong->MetaData.Album ));
	if ( sArtist.IsEmpty())
		sArtist = _( "Unknown Artist" );
	if ( sTitle.IsEmpty() )
		sTitle = _( "Unknown Song" );

	if (!(m_CurrSong.MetaData.Filename == pSong->MetaData.Filename 
        && m_CurrSong.MetaData.Artist == pSong->MetaData.Artist
        && m_CurrSong.MetaData.Album == pSong->MetaData.Album
        && m_CurrSong.MetaData.Title == pSong->MetaData.Title))
    {
        // song info has changed
        m_CurrSong = *pSong;
        m_pTunage->Started( *pSong );

        //--- title / artist / time -//
        sTitle.Replace	( wxT( "&" ), wxT( "&&" ), TRUE );
        sArtist.Replace	( wxT( "&" ), wxT( "&&" ), TRUE );

        if ( sTitle != stSong->GetLabel() )
            stSong->SetLabel( sTitle );
        if ( sArtist != stArtist->GetLabel() )
            stArtist->SetLabel( sArtist );
    }
	UpdateTime();
}

void CNowPlayingCtrl::PlayerStop( wxCommandEvent& WXUNUSED(event) )
{	
	m_MusikPlayer.Stop();
}

void CNowPlayingCtrl::PlayerPlayPause( wxCommandEvent& WXUNUSED(event) )	
{	
	m_MusikPlayer.PlayPause();
}

void CNowPlayingCtrl::PlayerNext( wxCommandEvent& WXUNUSED(event) )	
{	
	m_MusikPlayer.NextSong();
}

void CNowPlayingCtrl::PlayerPrev( wxCommandEvent& WXUNUSED(event) )	
{	
	m_MusikPlayer.PrevSong();
}

void CNowPlayingCtrl::PlayerVolume( wxCommandEvent& WXUNUSED(event) )	
{	
	wxPoint pos = btnVolume->GetParent()->ClientToScreen(btnVolume->GetPosition());
	wxFrame *pDlg = new MusikVolumeFrame( ( wxFrame* )g_MusikFrame, pos );
    
	wxSize DlgSize = pDlg->GetSize();
	pos.y -= DlgSize.GetHeight();
    if(pos.y < 0)
    {
        pos = btnVolume->GetParent()->ClientToScreen(wxPoint(btnVolume->GetPosition().x,btnVolume->GetPosition().y + btnVolume->GetSize().GetHeight()));
    }
	pDlg->Move( pos );
	pDlg->Show();
}

void CNowPlayingCtrl::OnPlayMode( wxCommandEvent&	event )
{
	int modesel = event.GetSelection();
	wxGetApp().Prefs.ePlaymode = (EMUSIK_PLAYMODE)modesel;
	m_MusikPlayer.SetPlaymode(wxGetApp().Prefs.ePlaymode);
}
void CNowPlayingCtrl::OnCheckCrossfade	( wxCommandEvent&	event )
{
	wxGetApp().Prefs.bGlobalFadeEnable = event.IsChecked();
}

void CNowPlayingCtrl::OnSongChanged(MusikPlayerEvent & ev)
{
    UpdateInfo(ev.MusikPlayer().GetCurrentSongid());
    ev.Skip();
}

void CNowPlayingCtrl::OnPlayStart(MusikPlayerEvent & ev)
{
    UpdateInfo(ev.MusikPlayer().GetCurrentSongid());
    PlayBtnToPauseBtn();
    ev.Skip();
}

void CNowPlayingCtrl::OnPlayPause(MusikPlayerEvent & ev)
{
	m_pTunage->Paused();
    PauseBtnToPlayBtn();
    ev.Skip();
}
void CNowPlayingCtrl::OnPlayResume(MusikPlayerEvent & ev)
{
	 m_pTunage->Resumed();
	PlayBtnToPauseBtn();
    ev.Skip();
}

void CNowPlayingCtrl::OnPlayStop(MusikPlayerEvent & ev)
{
	m_pTunage->Stopped();

	PauseBtnToPlayBtn();
    ResetInfo();
	ev.Skip();
}
