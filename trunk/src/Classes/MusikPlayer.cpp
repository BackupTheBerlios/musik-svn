/*
 *  MusikPlayer.cpp
 *
 *  Object which controls audio playback.
 *
 *  Uses FMOD sound API.
 *  Information about FMOD is available at http://www.fmod.org
 *  
 *  Copyright (c) 2003 Casey Langen (casey@bak.rr.com)
 *	Contributors: Simon Windmill, Dustin Carter, Gunnar Roth, Wade Brainerd
 *
 *  See the file "license.txt" for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL WARRANTIES.
*/

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#include "MusikPlayer.h"
#include "../Frames/MusikFrame.h"
//--- globals: library / player / prefs ---//
#include "../MusikGlobals.h"
#include "../MusikUtils.h"

#include "../MusikApp.h"
#include "MUSIKEngine/MUSIKEngine/inc/imusikstreamout.h"
//--- CMusikStreamArray ---//
#include <wx/arrimpl.cpp>
WX_DEFINE_OBJARRAY( CMusikStreamArray )


void * F_CALLBACKAPI dspcallback(void *WXUNUSED(originalbuffer), void *newbuffer, int length, void * WXUNUSED(userdata))
{
	// 2 channels (stereo), 16 bit sound
	g_FX.ProcessSamples( newbuffer, length, 2, 16 );
	return newbuffer;
}


BEGIN_EVENT_TABLE( CMusikPlayer, wxEvtHandler )
	//---------------------------------------------------------//
	//--- threading events.. we use EVT_MENU becuase its	---//
	//--- nice and simple, and gets the job done. this may	---//
	//--- become a little prettier later, but it works.		---//
	//---------------------------------------------------------//
    EVT_MENU			( MUSIK_PLAYER_NEXT_SONG,		CMusikPlayer::OnNextSongEvt		)
    EVT_MENU			( MUSIK_PLAYER_FADE_COMPLETE,	CMusikPlayer::OnFadeCompleteEvt )
    EVT_MENU			( MUSIK_PLAYER_NEW_METADATA,	CMusikPlayer::_UpdateNetstreamMetadata )
	EVT_MENU			( MUSIK_PLAYER_STOP,			CMusikPlayer::OnPlayerStop		)
	EVT_MENU			( MUSIK_PLAYER_PLAY_RESTART,	CMusikPlayer::OnPlayRestart		)
	EVT_MENU			( MUSIK_PLAYER_RESUME,			CMusikPlayer::OnPlayerResume	)
END_EVENT_TABLE()

CMusikPlayer::CMusikPlayer()
	:m_SndEngine(44100,2,0)
	,wxEvtHandler()
	
{
	m_Playing		= false;
	m_Paused		= false;
	m_Fading		= false;
	m_BeginFade		= false;
	m_StartingNext	= false;
	m_Stopping		= false;
	m_SongIndex		= 0;
	m_CrossfadeType	= 0;
	m_DSP			= NULL;
	m_Playmode = MUSIK_PLAYMODE_NORMAL;
	m_NETSTREAM_read_percent = 0;
	m_NETSTREAM_bitrate = 0;
	m_NETSTREAM_status = -1;
	m_NETSTREAM_flags = 0;
	m_b_NETSTREAM_AbortConnect = false;
	m_p_NETSTREAM_Connecting = NULL;
	//--- initialize random playback ---//
	long RandomSeed = wxGetLocalTime();
	SeedRandom( RandomSeed );
	m_arrHistory.Alloc(wxGetApp().Prefs.nMaxShuffleHistory);
	//set stop watch into pause mode.
	m_bStreamIsWorkingStopWatchIsRunning = false ;
	m_NETSTREAM_last_read_percent = 0;
	m_nLastSongTime = 0;
	m_bSuppressAutomaticSongPicking = false;
	m_bPostPlayRestartInProgress = false;
}

void CMusikPlayer::Init(bool bSuppressAutoPlay)
{
	int nPlayStartPos = 0;
// load old playlist
	if(wxFileExists(MUSIK_PLAYERLIST_FILENAME))
	{
		wxTextFile In;
		In.Open(MUSIK_PLAYERLIST_FILENAME);
		if (In.IsOpened() )
		{
			if(In.GetLineCount() > 1)
			{
				wxArrayString arr;
				DelimitStr(In.GetLine( 0 ),wxT(":"),arr);
				if(arr.GetCount()==2)
				{
					arr[0].ToULong((unsigned long *)&m_SongIndex);
					arr[1].ToLong((long*)&nPlayStartPos);
					nPlayStartPos = wxMax(0,(nPlayStartPos - 10 * 1000)/1000);
				}
			}
			wxArrayString aFilelist;
			for ( size_t i = 1; i < In.GetLineCount(); i++ )
			{
				aFilelist.Add( In.GetLine( i ) );	
			}
			In.Close();
			wxGetApp().Library.GetFilelistSongs( aFilelist, m_Playlist );
			if(m_SongIndex > m_Playlist.GetCount() - 1)
			{
				m_SongIndex = 0;
				m_nLastSongTime = 0;
			}
		}
	}
	if(!bSuppressAutoPlay && wxGetApp().Prefs.bAutoPlayOnAppStart && (m_Playlist.GetCount()
		||  (wxGetApp().Prefs.ePlaymode == MUSIK_PLAYMODE_AUTO_DJ) ||(wxGetApp().Prefs.ePlaymode == MUSIK_PLAYMODE_AUTO_DJ_ALBUM) ))
	{
		_PostPlayRestart( nPlayStartPos ); 
		if(m_Playlist.GetCount())// if we have songs in our playlist ( just loaded)
			m_bSuppressAutomaticSongPicking = true; // set this flag , so that auto djing does not interfere with starting of last played song.
	}
}
CMusikPlayer::~CMusikPlayer()
{
		wxRemoveFile( MUSIK_PLAYERLIST_FILENAME );
		wxTextFile Out;
		Out.Create( MUSIK_PLAYERLIST_FILENAME );
		Out.Open();
		if ( Out.IsOpened() )
		{
			if(m_SongIndex > m_Playlist.GetCount() - 1)
			{
				m_SongIndex = 0;
				m_nLastSongTime = 0;
			}
			Out.AddLine( wxString::Format(wxT("%d:%d"),m_SongIndex, m_nLastSongTime));
			for ( size_t i = 0; i < m_Playlist.GetCount(); i++ )
			{
				Out.AddLine( m_Playlist[i].MetaData.Filename.GetFullPath() );
			}
			
			Out.Write( Out.GuessType() );
			Out.Close();
		}


	//---------------------------------------------------------//
	//--- this will only get called when the program exits	---//
	//--- so nothing needs to be done. FMOD will always		---//
	//--- clean up after itself								---//
	//---------------------------------------------------------//
}

void CMusikPlayer::Shutdown( bool bClose )
{
	if ( bClose )
		Stop( true, true );
	else
		Stop();

	g_FX.EndEQ();
}
//--- fmod ---//
#include "fmod.h"

int CMusikPlayer::InitializeFMOD( int nFunction )
{
	if ( ( nFunction == FMOD_INIT_RESTART ) || ( nFunction == FMOD_INIT_STOP ) )
		Shutdown( false );
	FSOUND_Close();

	g_FX.InitEQ();
	g_FX.SetFrequency( 44100 );

	//---------------------//
	//--- setup driver	---//
	//---------------------//
	if ( ( nFunction == FMOD_INIT_START ) || ( nFunction == FMOD_INIT_RESTART ) )
	{
		//-----------------//
		//--- windows	---//
		//-----------------//
		#if defined(__WXMSW__)
			if ( wxGetApp().Prefs.nSndOutput == 0 )
			{
				if( FSOUND_SetOutput( FSOUND_OUTPUT_DSOUND ) == FALSE )
					return FMOD_INIT_ERROR_DSOUND;
			}
			else if ( wxGetApp().Prefs.nSndOutput == 1 )
			{
				if ( FSOUND_SetOutput( FSOUND_OUTPUT_WINMM ) == FALSE )
					return FMOD_INIT_ERROR_WINDOWS_WAVEFORM;
			}
			else if ( wxGetApp().Prefs.nSndOutput == 2 )
			{
				if ( FSOUND_SetOutput( FSOUND_OUTPUT_ASIO ) == FALSE )
					return FMOD_INIT_ERROR_ASIO;
			}
		//-----------------//
		//--- linux		---//
		//-----------------//
		#elif defined (__WXGTK__)
			if ( wxGetApp().Prefs.nSndOutput == 0 )
			{
				if( FSOUND_SetOutput( FSOUND_OUTPUT_OSS ) == FALSE )
					return FMOD_INIT_ERROR_OSS;
			}
			else if ( wxGetApp().Prefs.nSndOutput == 1 )
			{
				if ( FSOUND_SetOutput( FSOUND_OUTPUT_ESD ) == FALSE )
					return FMOD_INIT_ERROR_ESD;
			}
			else if ( wxGetApp().Prefs.nSndOutput == 2 )
			{
				if ( FSOUND_SetOutput( FSOUND_OUTPUT_ALSA ) == FALSE )
					return FMOD_INIT_ERROR_ALSA;
			}
		#elif defined (__WXMAC__)
			if( FSOUND_SetOutput( FSOUND_OUTPUT_MAC ) == FALSE )
				return FMOD_INIT_ERROR_MAC;
		#endif
		if(wxGetApp().Prefs.nSndDevice > 0)
		{
		//---------------------//
		//--- setup device	---//
		//---------------------//
			if (  FSOUND_SetDriver( wxGetApp().Prefs.nSndDevice ) == FALSE )
				return FMOD_INIT_ERROR_DEVICE_NOT_READY;
		}

		// initialize system
		FSOUND_SetBufferSize( 100 );
		InitFMOD_NetBuffer();	
		InitFMOD_ProxyServer();
		if ( FSOUND_Init( wxGetApp().Prefs.nSndRate , wxGetApp().Prefs.nSndMaxChan, 0 ) == FALSE )
			return FMOD_INIT_ERROR_INIT;
//		wxGetApp().Prefs.nSndOutput = FSOUND_GetOutput();
	}
	return FMOD_INIT_SUCCESS;
}
void CMusikPlayer::InitFMOD_NetBuffer	( )
{
	FSOUND_Stream_Net_SetBufferProperties(wxGetApp().Prefs.nStreamingBufferSize , wxGetApp().Prefs.nStreamingPreBufferPercent, wxGetApp().Prefs.nStreamingReBufferPercent);
}
void CMusikPlayer::InitFMOD_ProxyServer	( )
{
	if(wxGetApp().Prefs.bUseProxyServer)
	{
		wxString sProxyString(  wxGetApp().Prefs.sProxyServer );
		if(	!wxGetApp().Prefs.sProxyServerPort.IsEmpty() )
		{
			sProxyString += wxT(":") + 	wxGetApp().Prefs.sProxyServerPort;
		}
		if(	!wxGetApp().Prefs.sProxyServerUser.IsEmpty() )	 
		{
			sProxyString =  wxGetApp().Prefs.sProxyServerUser + wxT(":") + wxGetApp().Prefs.sProxyServerPassword + wxT("@")  +  sProxyString;
		}
		FSOUND_Stream_Net_SetProxy(ConvW2A(sProxyString));
	}
	else
		FSOUND_Stream_Net_SetProxy("");


}
void CMusikPlayer::SetPlaymode(EMUSIK_PLAYMODE pm )
{
	m_Playmode = pm;
}
void CMusikPlayer::PlayPause()
{
	//--- paused, so play ---//
	if ( IsPlaying() && !IsPaused() )
		Pause();

	//--- paused, so resume ---//
	else if ( IsPlaying() && IsPaused() )
		Resume();

	//--- start playing ---//
	else if ( !IsPlaying() )
	{
		if(GetPlaymode() == MUSIK_PLAYMODE_SHUFFLE)
		{
			NextSong();
		}
		else
		{
			PlayByUser( m_SongIndex );
		}
	}
}
void CMusikPlayer::PlayReplaceList(int nItemToPlay,const CMusikSongArray & playlist)
{
	if ( IsPaused() )
	{
		ClearOldStreams();
	}
	
	SetPlaylist( playlist );
	Play( nItemToPlay );
}
signed char F_CALLBACKAPI CMusikPlayer::MetadataCallback(char *name, char *value, void* userdata)
{
 	CMusikPlayer * _this = (CMusikPlayer *)userdata;
	_this->_SetMetaData(name, value);
	wxCommandEvent MetaDataEvt( wxEVT_COMMAND_MENU_SELECTED, MUSIK_PLAYER_NEW_METADATA );	
	wxPostEvent( _this, MetaDataEvt );

    return TRUE;
}
void CMusikPlayer::_SetMetaData(char *name, char *value)
{
	wxCriticalSectionLocker locker( m_critMetadata );
	wxString sMetadataName = ConvA2W(name).MakeUpper();
	
	if (wxT("ARTIST") == sMetadataName)
	{
		m_MetaDataSong.MetaData.Artist = value;
	}
	else if (wxT("TITLE")== sMetadataName)
	{
		m_MetaDataSong.MetaData.Title = value;
	}
	else if (wxT("ALBUM") == sMetadataName)
	{
		m_MetaDataSong.MetaData.Album = value;
	}
	else if (wxT("GENRE") == sMetadataName)
	{
		m_MetaDataSong.MetaData.Genre = value;
	}
	else if (wxT("TRACKNUMBER") == sMetadataName)
	{
		m_MetaDataSong.MetaData.nTracknum = atoi(value);
	}

//	::wxLogDebug(wxT("metadate received: name=%s,value=%s"),(const wxChar*)sMetadataName,(const wxChar*)value);
}
void CMusikPlayer::_UpdateNetstreamMetadata(wxCommandEvent& WXUNUSED(event))
{
	if(_CurrentSongIsNetStream())
	{
		{
			{
				wxCriticalSectionLocker locker( m_critMetadata );
				CMusikSong & cursong = m_Playlist.Item( m_SongIndex );
				cursong.MetaData.Artist = m_MetaDataSong.MetaData.Artist;
				cursong.MetaData.nTracknum = m_MetaDataSong.MetaData.nTracknum;
				cursong.MetaData.Genre = m_MetaDataSong.MetaData.Genre;
				cursong.MetaData.Title = m_MetaDataSong.MetaData.Title;
				cursong.MetaData.Album = m_MetaDataSong.MetaData.Album;
			}
			//wxCriticalSectionLocker locker( m_critInternalData );
			m_CurrentSong = m_Playlist.Item( m_SongIndex );
		}
		UpdateUI();
	}
}
void CMusikPlayer::_PostPlayRestart( int nStartPos )
{
		if(m_bPostPlayRestartInProgress)
			return;
		if(IsPlaying())
			Stop(false);	 // stop current stream
		wxCommandEvent PlayRestartEvt( wxEVT_COMMAND_MENU_SELECTED, MUSIK_PLAYER_PLAY_RESTART );
		PlayRestartEvt.SetInt( nStartPos );
		m_bPostPlayRestartInProgress = true;
		wxPostEvent( this,  PlayRestartEvt);
}   

void CMusikPlayer::OnPlayRestart( wxCommandEvent& event )
{//this method is used in case of a network problem when streaming
		m_bPostPlayRestartInProgress = false;
		Play(  m_SongIndex , event.GetInt() ); // start playing of current stream
}
bool CMusikPlayer::Play( size_t nItem, int nStartPos, int nFadeType )
{
	if((m_bSuppressAutomaticSongPicking == false) 
		&& (m_Playlist.GetCount() < (size_t)wxGetApp().Prefs.nAutoDJChooseSongsToPlayInAdvance) 
		&& ( MUSIK_PLAYMODE_AUTO_DJ == m_Playmode || MUSIK_PLAYMODE_AUTO_DJ_ALBUM == m_Playmode)
		)
	{
		if(m_Playlist.GetCount() == 0)
			nItem = 0;
		_AddRandomSongs();
		if(m_Playlist.GetCount() == 0)   // no songs could be chosen
		{
			m_Playing = false;
			return false;
		}
	}
	m_bSuppressAutomaticSongPicking = false;
	//--- check for an invalid playlist ---//
	if ( ( nItem >= m_Playlist.GetCount() ) || ( m_Playlist.GetCount() == 0 ) )
	{
		m_Playing = false;
		return false;
	}

	if (IsPlaying() && _CurrentSongIsNetStream())
	{	
		if(m_CurrentSong.MetaData.Filename == m_Playlist.Item( nItem ).MetaData.Filename)
		{
			return true; // already playing this stream
		}

//		_PostPlayRestart(); // will restart playing postponed
//		return true;
	}
	else if (_IsNETSTREAMConnecting())
	{
		return true;
	}
	else if((m_Playlist.Item( nItem ).MetaData.eFormat != MUSIK_FORMAT_NETSTREAM) && !m_Playlist.Item( nItem ).MetaData.Filename.FileExists())
	{	// not a network stream
		// and
		// file does not exist
			wxMessageBox( _( "Cannot find file.\n Filename:" ) + m_Playlist.Item( nItem ).MetaData.Filename.GetFullPath(), MUSIKAPPNAME_VERSION, wxICON_STOP );
			Stop();
			return false;
	}

	bool bNewSongStarted = (m_Playlist[nItem].songid != m_CurrentSong.songid) && (nStartPos == 0);
	if(bNewSongStarted && m_CurrentSong.songid != -1 && IsPlaying())
	{
	  // player is currently playing, so we have to record history here, because Stop() is not 
	  // called between the playing of the songs of the playlist.
			wxGetApp().Library.RecordSongHistory(m_CurrentSong,GetTime(FMOD_MSEC));
	}
	//---------------------------------------------//
	//--- start with the basics					---//
	//---------------------------------------------//
	m_Stopping		= false;
	m_SongIndex		= nItem;
	m_CurrentSong	= m_Playlist.Item( m_SongIndex );

	//---------------------------------------------//
	//--- if there is already a fade in			---//
	//--- progress, then we need to abort it	---//
	//---------------------------------------------//
	SetCrossfadeType( nFadeType );

	//---------------------------------------------//
	//--- open a new stream and push it to the	---//
	//--- bottom of the m_ActiveStreams array	---//
	//---------------------------------------------//
	FSOUND_Stream_SetBufferSize( wxGetApp().Prefs.nSndBuffer );
	FMODEngine::eOpenMode om = _CurrentSongNeedsMPEGACCURATE() ? FMODEngine::OpenMode_MPEGACCURATE : FMODEngine::OpenMode_Default;
	m_SndEngine.SetOpenMode(om);
	wxString sFilename;
	sFilename = m_CurrentSong.MetaData.Filename.GetFullPath();

	MUSIKStream* pNewStream = m_SndEngine.OpenMedia( ( const char* )ConvFn2A( sFilename ));
	if(pNewStream == NULL)
	{
		wxMessageBox( _( "Playback will be stopped, because loading failed.\n Filename:" ) + sFilename, MUSIKAPPNAME_VERSION, wxICON_STOP );
		Stop(false);
		return false;
	}
	InitDSP();
	if(_CurrentSongIsNetStream()&& _IsNETSTREAMConnecting() == false)
	{
		ClearOldStreams(true);// clear all streams
		m_MetaDataSong = CMusikSong();
		m_MetaDataSong.MetaData.eFormat = MUSIK_FORMAT_NETSTREAM;
		m_p_NETSTREAM_Connecting = pNewStream;
		m_Playing = true;
		m_b_NETSTREAM_AbortConnect = false;
		bool bExit = false;
		do
		{
			int  openstate = FSOUND_Stream_GetOpenState((FSOUND_STREAM*)pNewStream->GetStreamOut()->STREAM());
			if ((openstate == -1) || (openstate == -3))
			{
				wxMessageBox(_("ERROR: failed to open stream:")+ ConvA2W(FSOUND_Stream_Net_GetLastServerStatus()));
				m_b_NETSTREAM_AbortConnect = true;
				break;
			}
			
		    switch(_NetStreamStatusUpdate(pNewStream))
			{
			case FSOUND_STREAM_NET_READY:
				if(openstate == 0)
					bExit = true;
				break;
			case FSOUND_STREAM_NET_ERROR:
				m_b_NETSTREAM_AbortConnect = true;
				break;
			}
			wxGetApp().Yield();

		} while (!bExit && !m_b_NETSTREAM_AbortConnect);
		m_p_NETSTREAM_Connecting = NULL;
		if( m_b_NETSTREAM_AbortConnect )
		{
			delete pNewStream;
			Stop(false);
			m_b_NETSTREAM_AbortConnect = false;
			return false;
		}
		
	}
	if(_CurrentSongIsNetStream())
	{
		FSOUND_Stream_Net_SetMetadataCallback((FSOUND_STREAM*)pNewStream->GetStreamOut()->STREAM(), MetadataCallback, this);
	}
	pNewStream->SetVolume(  0.0 );
	pNewStream->SetTime( nStartPos * 1000 );
	//---------------------------------------------//
	//--- start playback on the new stream on	---//
	//--- the designated channel.				---//
	//---------------------------------------------//
	int retries = 2;
	bool bPlaySucceeded = false;
	while( retries -- && (!bPlaySucceeded))
	{
		if(!pNewStream->Play())
		{
			wxCriticalSectionLocker lock(m_protectingStreamArrays);
			if(m_ActiveStreams.GetCount())
			{
				delete( m_ActiveStreams[0] );
				m_ActiveStreams.RemoveAt(0);
			}

		}
		else
			bPlaySucceeded = true;
	}
	if(!bPlaySucceeded)
	{
		wxMessageBox(_("Play failed, please try again."));
		wxLogDebug(wxT("play failed:%s"),(const wxChar *) ConvA2W(m_SndEngine.ErrorString()));
		delete pNewStream;
		Stop(false);
		return false;

	}
	m_Playing = true;
	m_Paused = false;
	g_FaderThread->CrossfaderAbort();
	//---------------------------------------------//
	//--- update the global arrays containing	---//
	//--- active channels and streams			---//
	//---------------------------------------------//
	{
		wxCriticalSectionLocker lock(m_protectingStreamArrays);
		m_ActiveStreams.Add( pNewStream );
	}
	

	SetFrequency();

	
	//---------------------------------------------//
	//--- playback has been started, update the	---//
	//--- user interface to reflect it			---//
	//---------------------------------------------//
	g_MusikFrame->m_pNowPlayingCtrl->PlayBtnToPauseBtn();
	UpdateUI();
	//---------------------------------------------//
	//--- record history in database			---//
	//---------------------------------------------//
	if(bNewSongStarted)
	{
		wxGetApp().Library.UpdateItemLastPlayed	( m_CurrentSong );
	}
	//---------------------------------------------//
	//--- if fading is not enabled, shut down	---//
	//--- all of the old channels, and set the	---//
	//--- active stream to full volume			---//
	//---------------------------------------------//
	if ( wxGetApp().Prefs.bFadeEnable == 0 || wxGetApp().Prefs.bGlobalFadeEnable == 0 )
	{
		if(pNewStream)
			pNewStream->SetVolume(1.0);
		ClearOldStreams();
	}

	//---------------------------------------------//
	//--- tell the listening thread its time to	---//
	//--- start fading							---//
	//---------------------------------------------//
	else if ( wxGetApp().Prefs.bFadeEnable && wxGetApp().Prefs.bGlobalFadeEnable )
		SetFadeStart();		
	return true;
}
int CMusikPlayer::_NetStreamStatusUpdate(MUSIKStream * pStream)
{
	int status = 0;
	CMusikSong & song = m_CurrentSong;
	FSOUND_Stream_Net_GetStatus((FSOUND_STREAM*)pStream->GetStreamOut()->STREAM(), &status, &m_NETSTREAM_read_percent, &m_NETSTREAM_bitrate, &m_NETSTREAM_flags);
	if(m_NETSTREAM_status != status)
	{
		m_NETSTREAM_status = status;
		switch(m_NETSTREAM_status)
		{
		case FSOUND_STREAM_NET_NOTCONNECTED:
			break;
		case FSOUND_STREAM_NET_CONNECTING:
		case FSOUND_STREAM_NET_BUFFERING:
		case FSOUND_STREAM_NET_READY:
			g_MusikFrame->m_pNowPlayingCtrl->UpdateInfo(song);
			break;
		case FSOUND_STREAM_NET_ERROR:
			wxMessageBox(_("ERROR: failed to open stream:")+ ConvA2W(FSOUND_Stream_Net_GetLastServerStatus()));
			break;

		}
	}
	if(song.MetaData.nBitrate != m_NETSTREAM_bitrate)
	{
		song.MetaData.nBitrate = m_NETSTREAM_bitrate;
		UpdateUI();
	}
	

	return status;
}
void CMusikPlayer::InitDSP()
{
	if ( !m_DSP )
		m_DSP = FSOUND_DSP_Create( &dspcallback, FSOUND_DSP_DEFAULTPRIORITY_USER, 0 );			

	if ( m_DSP )
		FSOUND_DSP_SetActive( m_DSP, wxGetApp().Prefs.bUseEQ );
}

void CMusikPlayer::ActivateDSP()
{
	FSOUND_DSP_SetActive( m_DSP, wxGetApp().Prefs.bUseEQ );
}

void CMusikPlayer::FreeDSP()
{
	if ( m_DSP )
	{
		FSOUND_DSP_Free( m_DSP );
		m_DSP = NULL;
	}
}

void CMusikPlayer::SetFrequency()
{
	//-------------------------------------------------//
	//--- SiW: this causes problems. mp3s play fine	---//
	//--- but ogg's are slow. or vice versa.		---//
	//--- commented this out for the release.		---//
	//--- See Also: MusikFXDialog::MusikFXDialog()	---//
	//-------------------------------------------------//
	if ( wxGetApp().Prefs.bUsePitch )
		FSOUND_SetFrequency( FSOUND_ALL, (int)g_FX.GetFrequency() );
	
}

void CMusikPlayer::UpdateUI()
{
	CMusikSong song;
	if(_CurrentSongIsNetStream())
	{
		g_PlaylistBox->PlaylistCtrl().ResynchItem		( m_SongIndex,m_CurrentSong);
	}
	else
	{
		g_PlaylistBox->PlaylistCtrl().Update(false);
	}
	if(g_SourcesCtrl->GetSelType() == MUSIK_SOURCES_NOW_PLAYING)
	{
		g_PlaylistBox->PlaylistCtrl().EnsureVisible(m_SongIndex);
	}
	g_MusikFrame->m_pNowPlayingCtrl->UpdateInfo	( m_CurrentSong );
}

void CMusikPlayer::ClearOldStreams( bool bClearAll )
{
	if (!bClearAll && g_FaderThread )
	{
		if ( g_FaderThread->IsCrossfaderActive() )
			return;
	}
	if(g_FaderThread)
	{
		g_FaderThread->CrossfaderAbort();
		g_FaderThread->CrossfaderStop();
	}
	wxCriticalSectionLocker lock(m_protectingStreamArrays);
	int nStreamCount = m_ActiveStreams.GetCount();

	//-------------------------------------------------//
	//--- if we aren't clearing all streams, keep	---//
	//--- the primary one going.					---//
	//-------------------------------------------------//
	if ( !bClearAll )
		nStreamCount--;

	for ( int i = 0; i < nStreamCount; i++ )
	{
		if(m_ActiveStreams.Item( 0 ))
		{
			delete m_ActiveStreams.Item( 0 );
		}
		m_ActiveStreams.RemoveAt( 0 );
	}
}

void CMusikPlayer::Pause( bool bCheckFade )
{
	if(_IsNETSTREAMConnecting())
		return;
	//-------------------------------------------------//
	//--- set the fade type, let the thread worry	---//
	//--- about cleaning any existing fades.		---//
	//-------------------------------------------------//
	SetCrossfadeType( CROSSFADE_PAUSE );

	//-------------------------------------------------//
	//--- update the UI.							---//
	//-------------------------------------------------//
	g_MusikFrame->m_pNowPlayingCtrl->PauseBtnToPlayBtn();
	
	//-------------------------------------------------//
	//--- if this type of crossfade is enabled,		---//
	//--- then just setup a fade signal. an event	---//
	//--- will get posted back to actually pause	---//
	//--- once the fade is complete.				---//
	//-------------------------------------------------//
	if ( bCheckFade && wxGetApp().Prefs.bFadePauseResumeEnable && wxGetApp().Prefs.bGlobalFadeEnable )
		SetFadeStart();
	else
		FinalizePause();
}

void CMusikPlayer::FinalizePause()
{
	m_SndEngine.SetPlayState( MUSIKEngine::Paused );
	m_Paused = true;
}

void CMusikPlayer::Resume( bool bCheckFade )
{
	//-------------------------------------------------//
	//--- set the fade type, let the thread worry	---//
	//--- about cleaning any existing fades.		---//
	//-------------------------------------------------//
	SetCrossfadeType( CROSSFADE_RESUME );

	//-------------------------------------------------//
	//--- update the UI.							---//
	//-------------------------------------------------//
	g_MusikFrame->m_pNowPlayingCtrl->PlayBtnToPauseBtn();
	m_SndEngine.SetPlayState( MUSIKEngine::Playing );
	m_Paused = false;
	//-----------------------------------------------------//
	//--- setup crossfader and return, if	the prefs	---//
	//--- say so.										---//
	//-----------------------------------------------------//
	if ( bCheckFade && wxGetApp().Prefs.bFadePauseResumeEnable && wxGetApp().Prefs.bGlobalFadeEnable )
			SetFadeStart();
	else
		FinalizeResume();
}

void CMusikPlayer::FinalizeResume()
{
	SetVolume();
}

void CMusikPlayer::Stop( bool bCheckFade, bool bExit )
{
	if(m_Stopping) return;
	m_b_NETSTREAM_AbortConnect = true;
	m_Stopping = true;
	//-------------------------------------------------//
	//--- set the fade type, let the thread worry	---//
	//--- about cleaning any existing fades.		---//
	//-------------------------------------------------//
	if ( !bExit )
		SetCrossfadeType( CROSSFADE_STOP );
	else
		SetCrossfadeType( CROSSFADE_EXIT );


	//-------------------------------------------------//
	//--- update the ui.							---//
	//-------------------------------------------------//
	g_MusikFrame->SetTitle( MUSIKAPPNAME_VERSION );
	g_MusikFrame->m_pNowPlayingCtrl->ResetInfo();
	g_MusikFrame->m_pNowPlayingCtrl->PauseBtnToPlayBtn();

	

	//-------------------------------------------------//
	//--- setup crossfader and return, if the prefs	---//
	//--- say so.									---//
	//-------------------------------------------------//
	if ((!_CurrentSongIsNetStream() || bExit) && bCheckFade && wxGetApp().Prefs.bFadeExitEnable && wxGetApp().Prefs.bGlobalFadeEnable )
	{
		//--- use exit duration ---//
		if ( bExit )
		{
			if ( IsPlaying() )
			{
				SetFadeStart();
				return;
			}
		}
		
		//--- use stop duration ---//
		else
		{
			if ( IsPlaying() )
			{
				SetFadeStart();
				return;
			}				
		}
	}
	else
		FinalizeStop();
}

void CMusikPlayer::FinalizeStop()
{
	if(m_CurrentSong.songid >= 0 &&IsPlaying())
	{
		wxGetApp().Library.RecordSongHistory(m_CurrentSong,m_nLastSongTime);
	}
	else
		m_nLastSongTime = 0;
	m_CurrentSong = CMusikSong();
	m_Playing = false;
	m_Paused = false;
	m_Stopping = false;
	g_PlaylistBox->PlaylistCtrl().ResynchItem( m_SongIndex );
	int nStreamCount = m_ActiveStreams.GetCount();
	for ( int i = 0; i < nStreamCount; i++ )
	{
		delete	m_ActiveStreams.Item( ( nStreamCount - 1 ) - i );
	}
	wxCriticalSectionLocker lock(m_protectingStreamArrays);
	m_ActiveStreams.Clear();

	//-----------------------------------------//
	//--- free up the DSP object. FX stuff.	---//
	//-----------------------------------------//
	FreeDSP();
}

size_t CMusikPlayer::GetShuffledSong()
{
	size_t r = 0;

	if( m_Playlist.GetCount() == 0 )
		return 0;

	
	bool repeat = false;
	int nMaxRepeatCount = 30;
	do {
		repeat = false;
		
		r = (size_t) (m_Playlist.GetCount() * (GetRandomNumber() / (RandomMax + 1.0)));  // random range  [0 , m_Playlist.GetCount()-1] 

		if(m_arrHistory.GetCount() == m_Playlist.GetCount())
		{	// history is as large as the playlist
			// clear half of the history 
			for(size_t i = 0; i < m_arrHistory.GetCount()/2;i++)
			{
				m_arrHistory.RemoveAt(0);
			}
			break;
		}
		if(nMaxRepeatCount--) // only check for repeats nMaxRepeatCount times, to prevent endless do loop
		{
			if(m_Playlist.Item( r ).MetaData.Filename.FileExists())
			{
				//--- check for repeats ---//
				for ( size_t j = 0;  j < m_arrHistory.GetCount(); j++ )
				{
					if ( r == (size_t)m_arrHistory[j] )
					{
						repeat = true; 
						break;
					}
				}
			}
			else
			{
				repeat = true; 
			}
		}
	} while ( repeat );

	if( m_arrHistory.GetCount() >= (size_t)wxGetApp().Prefs.nMaxShuffleHistory )
	{
	//--- rotate history ---//
		m_arrHistory.RemoveAt(0);
	}
	m_arrHistory.Add(r);
	wxLogDebug(wxT("songindex %d\n"),r);
	return r;
}

void CMusikPlayer::NextSong()
{
	switch ( m_Playmode )
	{
	case MUSIK_PLAYMODE_AUTO_DJ:
	case MUSIK_PLAYMODE_AUTO_DJ_ALBUM:

		_AddRandomSongs();
		
		// fall through
	case MUSIK_PLAYMODE_NORMAL:
		m_SongIndex++;
		if ( m_SongIndex >= m_Playlist.GetCount() )
		{
			Stop();
			m_SongIndex = 0;
		}
		else
			Play( m_SongIndex );
		break;

	case MUSIK_PLAYMODE_REPEATSONG:
		Play( m_SongIndex );
		break;

	case MUSIK_PLAYMODE_REPEATLIST:
		if ( m_SongIndex == ( m_Playlist.GetCount()-1 ) )
			m_SongIndex = 0;
		else m_SongIndex++;
		Play( m_SongIndex );
		break;

	case MUSIK_PLAYMODE_SHUFFLE:
		{
			if (m_Playlist.GetCount())
			{
				if(m_SongIndex < m_Playlist.GetCount() - 1) 
				{  // check if the following song , should always be played(even if we are in shuffle mode)
					if(m_Playlist[m_SongIndex + 1].bForcePlay)
					{
						m_Playlist[m_SongIndex + 1].bForcePlay = 0; //reset flag
						m_SongIndex++;
						m_arrHistory.Add(m_SongIndex); // add to shuffle history too
						Play(m_SongIndex);
						return;
					}
				}
			   Play( GetShuffledSong() );
			}
		}
		break;
	}
}

void CMusikPlayer::PrevSong()
{
	//---------------------------------------------------------//
	//--- 2  second grace period to go to previous track	---//
	//---------------------------------------------------------//
	if ( m_ActiveStreams.GetCount() && GetTime( FMOD_MSEC ) > 2000 )	
	{
		Play( m_SongIndex );
		return;
	} 
	else
	{
		switch ( m_Playmode )
		{
		case MUSIK_PLAYMODE_AUTO_DJ:
		case MUSIK_PLAYMODE_AUTO_DJ_ALBUM:
		case MUSIK_PLAYMODE_NORMAL:
			if( m_SongIndex > 0 )
				m_SongIndex--;
			Play( m_SongIndex );
			break;
	
		case MUSIK_PLAYMODE_REPEATSONG:
			Play( m_SongIndex );
			break;
	
		case MUSIK_PLAYMODE_REPEATLIST:
			if ( m_SongIndex <= 0 )
				m_SongIndex = ( m_Playlist.GetCount() - 1 );
			else m_SongIndex--;
			Play( m_SongIndex );
			break;
	
		case MUSIK_PLAYMODE_SHUFFLE:
			if(m_arrHistory.GetCount() >= 2)
			{
				m_SongIndex = m_arrHistory[m_arrHistory.GetCount() - 2];
				Play( m_SongIndex );
			}
			break;
		}
	}
}

void CMusikPlayer::_AddRandomSongs()
{
	CMusikSongArray  arrSongs;

	if(m_SongIndex + 1 > m_Playlist.GetCount())
		m_SongIndex = 0;

	int nSongsToAdd = m_Playlist.GetCount() ? (wxGetApp().Prefs.nAutoDJChooseSongsToPlayInAdvance - (m_Playlist.GetCount() - 1 - m_SongIndex))
											:(int)wxGetApp().Prefs.nAutoDJChooseSongsToPlayInAdvance;
	if(nSongsToAdd <= 0)
		return;
	if(MUSIK_PLAYMODE_AUTO_DJ == m_Playmode)
	{

		_ChooseRandomSongs(nSongsToAdd,arrSongs);
	}
	else if(MUSIK_PLAYMODE_AUTO_DJ_ALBUM == m_Playmode)
	{
		_ChooseRandomAlbumSongs(wxGetApp().Prefs.nAutoDJChooseAlbumsToPlayInAdvance,arrSongs);
	}
	for(size_t i = 0; i < arrSongs.GetCount();i++)
		arrSongs[i].bChosenByUser = 0;
	AddToPlaylist(arrSongs,false);
}

void CMusikPlayer::_ChooseRandomSongs(int nSongsToAdd,CMusikSongArray &arrSongs)
{
	if(nSongsToAdd <= 0)
		return;
	wxString sQuerySongs = wxString::Format(wxT(" select autodj_songs.songid from autodj_songs ") 
		wxT("where lastplayed = '' or lastplayed < julianday('now','-%d hours');"),(int)wxGetApp().Prefs.nAutoDjDoNotPlaySongPlayedTheLastNHours);	 //(int) cast to make gcc 2.95 happy
	wxArrayInt arrSongIds;
	wxGetApp().Library.Query(sQuerySongs,arrSongIds);
	if(arrSongIds.GetCount() < (size_t)nSongsToAdd )
	{  // not enough songs found, so relax the DoNotPlaySongPlayedTheLastNHours condition to 2 hours
		sQuerySongs = wxString::Format(wxT(" select autodj_songs.songid from autodj_songs ") 
			wxT("where lastplayed = '' or lastplayed < julianday('now','-2 hours');"));	 //(int) cast to make gcc 2.95 happy
		wxGetApp().Library.Query(sQuerySongs,arrSongIds);
	}
	while (arrSongIds.GetCount() && (arrSongs.GetCount() < (size_t)nSongsToAdd))
	{
		int r = (size_t) (arrSongIds.GetCount() * (GetRandomNumber() / (RandomMax + 1.0)));  // random range  [0 , arrSongIds.GetCount()-1] 
		CMusikSong song;
		if(wxGetApp().Library.GetSongFromSongid(arrSongIds[r],&song))
		{
			if(song.MetaData.Filename.FileExists())
			{
				arrSongs.Add(song);
			}
			arrSongIds.RemoveAt(r); //remove index, so songid cannot be chosen again.
		}
	} 
}
void CMusikPlayer::_ChooseRandomAlbumSongs(int nAlbumsToAdd,CMusikSongArray &arrAlbumSongs)
{
	if(nAlbumsToAdd <= 0)
		return;
	int nMaxRepeatCount = 30;
	int hours = wxGetApp().Prefs.nAutoDjDoNotPlaySongPlayedTheLastNHours;
	char * count_query = sqlite_mprintf("select count(*) from autodj_albums where most_lastplayed = '' or most_lastplayed < julianday('now','-%d hours');",hours);
	int albums_count = wxGetApp().Library.QueryCount(count_query);
	sqlite_freemem( count_query );
	wxArrayString arrAlbums;
	while ( (nMaxRepeatCount > 0) && ( arrAlbums.GetCount() < (size_t)nAlbumsToAdd ))
	{
		int r = (size_t) (albums_count * (GetRandomNumber() / (RandomMax + 1.0)));  // random range  [0 , albums_count-1] 
		wxString sQueryRandomAlbum = wxString::Format(wxT("select album||'|'||artist from autodj_albums where most_lastplayed = '' or most_lastplayed < julianday('now','-%d hours') limit 1 offset %d;") 
											,nMaxRepeatCount < 5 ? 1 : (int)wxGetApp().Prefs.nAutoDjDoNotPlaySongPlayedTheLastNHours,r);

		wxArrayString newAlbums;
		wxGetApp().Library.Query(sQueryRandomAlbum,newAlbums,false);
		bool repeat = false;
		if(newAlbums.GetCount() > 0)
		{
			//--- check for repeats ---//
			for ( size_t j = 0;  j < arrAlbums.GetCount(); j++ )
			{
				if ( newAlbums[0] == arrAlbums[j] )
				{
					repeat = true; 
					break;
				}
			}
		}
		else
			repeat = true;

		if(!repeat)
			arrAlbums.Add(newAlbums[0]);
		else
			nMaxRepeatCount--;

	} 

	
	
	for(size_t i = 0;i < arrAlbums.GetCount();i++)
	{
		wxString sQuery;
		sQuery.Alloc( 80);
		arrAlbums[i].Replace( wxT( "'" ), wxT( "''" ));
		wxArrayString album_artist;
		DelimitStr(arrAlbums[i],wxT("|"),album_artist);
		sQuery+=wxT("album='");
		sQuery+=album_artist[0];
		sQuery+=wxT("' and artist='");
		sQuery+=album_artist[1];
		sQuery+=wxT("' order by tracknum");
		wxGetApp().Library.QuerySongsWhere(sQuery ,arrAlbumSongs,false,false);	
	}
	

}

void CMusikPlayer::SetVolume()
{
	m_SndEngine.SetVolume(wxGetApp().Prefs.nSndVolume / 255.0);
}

int CMusikPlayer::GetDuration( int nType )
{
	if(_CurrentSongIsNetStream())
	{
		return 100;
	}
	if(m_ActiveStreams.GetCount() == 0)
		return 0;
	int nDuration = m_ActiveStreams[m_ActiveStreams.GetCount()-1]->GetLengthMs( );
	if ( nType == FMOD_SEC )
		nDuration /= 1000;

	return nDuration;
}

int CMusikPlayer::GetTime( int nType )
{
	if(_CurrentSongIsNetStream())
	{
		MUSIKStream * p_NETSTREAM = NULL;
		if(m_ActiveStreams.GetCount() && m_ActiveStreams.Item( m_ActiveStreams.GetCount()-1 ))
		{
			p_NETSTREAM = m_ActiveStreams.Item( m_ActiveStreams.GetCount()-1 );	
		}
		else if (m_p_NETSTREAM_Connecting)
		{
			return m_NETSTREAM_read_percent;
		}
		int status = 0;
		if(p_NETSTREAM )
		{
			m_NETSTREAM_last_read_percent =	 m_NETSTREAM_read_percent;
			status = _NetStreamStatusUpdate(p_NETSTREAM);
			int openstate = FSOUND_Stream_GetOpenState((FSOUND_STREAM*)p_NETSTREAM->GetStreamOut()->STREAM());
			if( openstate == 0) 
			{
				
				int off = p_NETSTREAM->GetTime();
				int len = p_NETSTREAM->GetLengthMs();
	              
				if((status == FSOUND_STREAM_NET_ERROR) || (off >= len))
				{// something is wrong, we try to restart the stream
					_PostPlayRestart();
				}
				else
					m_bStreamIsWorkingStopWatchIsRunning = false;// everything is okay
 			}
			else 
			{   // openstate is not okay
				if(m_bStreamIsWorkingStopWatchIsRunning 
						&& m_NETSTREAM_last_read_percent == m_NETSTREAM_read_percent
						&&	m_StreamIsWorkingStopWatch.Time() > 10 * 1000) // 10 sec
				{  // stream is not working ( read percent did not change and FSOUND_Stream_GetOpenState failed)for 10 secs now
				   m_bStreamIsWorkingStopWatchIsRunning = false;
				   _PostPlayRestart();
				}
				else
				{	// stream is disturbed   ,start watch ( if not already running)
					if(!m_bStreamIsWorkingStopWatchIsRunning)					
					{
						m_NETSTREAM_last_read_percent =	 m_NETSTREAM_read_percent;
						m_StreamIsWorkingStopWatch.Start();
						m_bStreamIsWorkingStopWatchIsRunning = true;
					}
				}
			}
		}
		return m_NETSTREAM_read_percent;
	}
	// no critical section here, because crossfader thread is stopped before m_ActiveStreams array is changed(add/remove)
	if(m_ActiveStreams.GetCount() == 0)
		m_nLastSongTime = 0;
	else 
		m_nLastSongTime = m_ActiveStreams.Item( m_ActiveStreams.GetCount()-1 )->GetTime() ;
	if ( nType == FMOD_SEC )
		 m_nLastSongTime /= 1000;
	return  m_nLastSongTime;
}

int CMusikPlayer::GetTimeLeft( int nType )
{
	//wxCriticalSectionLocker locker( m_critInternalData); // lock because this method is called from fader thread , protect m_Playlist
	if(_CurrentSongIsNetStream())
	{
		return 10000000;
	}
	int nTimeLeft = GetDuration( FMOD_MSEC ) - GetTime( FMOD_MSEC ); 
	if ( nType == FMOD_SEC )
		nTimeLeft /= 1000;
	return nTimeLeft;
}

wxString CMusikPlayer::GetTimeStr()
{
	if(_CurrentSongIsNetStream())
	{
		wxString sProgress;
		switch(m_NETSTREAM_status)
		{
		case FSOUND_STREAM_NET_NOTCONNECTED:
			sProgress = _("not connected");
			break;
		case FSOUND_STREAM_NET_CONNECTING:
			sProgress = _("connecting ...");
			break;
		case FSOUND_STREAM_NET_BUFFERING:
			sProgress = _("buffering ...");
			break;
		case FSOUND_STREAM_NET_READY:
			sProgress = _("streaming");
			break;
		case FSOUND_STREAM_NET_ERROR:
			sProgress = _("net error");
			break;
		}

		return wxString::Format( wxT( "%s (%d%%)"), (const wxChar *)sProgress, GetTime( FMOD_SEC ));
		
	}
	else
		return SecToStr( GetTime( FMOD_SEC ) );
}
wxString CMusikPlayer::GetTimeLeftStr()
{
	if(_CurrentSongIsNetStream())
	{
		return GetTimeStr();
	}
	else
		return SecToStr( GetTimeLeft( FMOD_SEC ) );
}
void CMusikPlayer::SetTime( int nSec )
{
	if(_IsSeekCrossFadingDisabled())
	{
		m_ActiveStreams.Item( m_ActiveStreams.GetCount() - 1 )->SetTime( nSec * 1000 );
	}
	else
	{
		Play( m_SongIndex, nSec, CROSSFADE_SEEK );
	}
}

bool CMusikPlayer::_IsSeekCrossFadingDisabled()
{
	return ( wxGetApp().Prefs.bGlobalFadeEnable == 0 || wxGetApp().Prefs.bFadeSeekEnable == 0 || IsPaused() 
		||  _CurrentSongNeedsMPEGACCURATE() // no seek crossfadeing, because _CurrentSongNeedsMPEGACCURATE files are slow in opening
		|| _CurrentSongIsNetStream());
}
bool CMusikPlayer::_CurrentSongNeedsMPEGACCURATE()
{
	return (wxGetApp().Prefs.bUse_MPEGACCURATE_ForMP3VBRFiles
			&&(m_SongIndex <  m_Playlist.GetCount())
			&& m_CurrentSong.MetaData.bVBR 
			&& (m_CurrentSong.MetaData.eFormat == MUSIK_FORMAT_MP3) ); 
	//  mp3 vbr files needs FSOUND_MPEGACCURATE flag, but takes much too long for other filetypes
}
bool CMusikPlayer::_CurrentSongIsNetStream()
{
	return ((m_SongIndex <  m_Playlist.GetCount())
			&& (m_CurrentSong.MetaData.eFormat == MUSIK_FORMAT_NETSTREAM) ); 
}

void CMusikPlayer::OnFadeCompleteEvt( wxCommandEvent& event )
{
	long FadeType = event.GetExtraLong();
	bool bAbort = event.GetInt()!=0;
	//-------------------------------------------------//
	//--- finalize whatever type of fade was going	---//
	//--- on.										---//
	//-------------------------------------------------//
	if ( FadeType == CROSSFADE_STOP || FadeType == CROSSFADE_EXIT )
		FinalizeStop();
	else if ( FadeType == CROSSFADE_PAUSE )
		FinalizePause();
	else if ( FadeType == CROSSFADE_RESUME )
		FinalizeResume();

	m_Fading = false; 
	if(!bAbort)
		ClearOldStreams();
}

void CMusikPlayer::SetFadeStart()
{
	m_BeginFade = true;
	m_Fading = true; 
}

void CMusikPlayer::AddToPlaylist( CMusikSongArray & songstoadd ,bool bPlayFirstAdded )
{
	size_t size = songstoadd.GetCount();
	if(size)
	{
		//wxCriticalSectionLocker locker( m_critInternalData);
		int plsize = m_Playlist.GetCount();
		for(size_t i = 0; i < size ; i++)
		{
			m_Playlist.Add(songstoadd.Detach(0));
		}
		if(bPlayFirstAdded)
		{
			m_SongIndex = plsize;
			m_bSuppressAutomaticSongPicking = true;
			Play(m_SongIndex);
		}
		if(g_SourcesCtrl->GetSelType() == MUSIK_SOURCES_NOW_PLAYING)
		{
			g_Playlist = m_Playlist;
			g_PlaylistBox->Update(false);
		}
	}
}
void CMusikPlayer::InsertToPlaylist( CMusikSongArray & songstoadd ,bool bPlayFirstInserted)
{
	//wxCriticalSectionLocker locker( m_critInternalData);
	size_t size = songstoadd.GetCount();
	for(size_t i = 0; i < size; i++)
		songstoadd[i].bForcePlay=1;// mark each song such as it is played even in shuffle mode

	size_t plsize = m_Playlist.GetCount();
	if(plsize == 0 || !IsPlaying())
	{// list empty, add
		for(size_t i = 0; i < size ; i++)
			m_Playlist.Add(songstoadd.Detach(0));
		if(bPlayFirstInserted)
		{
			m_SongIndex = plsize;
			m_bSuppressAutomaticSongPicking = true;
			Play(m_SongIndex);
		}
		else
			m_SongIndex = plsize;
		return;

	}
// list not empty ,insert
	for(size_t i = 0; i < size ; i++)
	{
		m_Playlist.Insert(songstoadd.Detach(0), m_SongIndex + 1 + i );
	}
	if(bPlayFirstInserted)
	{
		m_SongIndex++;
		m_bSuppressAutomaticSongPicking = true;
		Play(m_SongIndex);
	}
}
void CMusikPlayer::RemovePlaylistEntry( size_t index )
{
	//wxCriticalSectionLocker locker( m_critInternalData);
	wxASSERT(index < m_Playlist.GetCount()); 
	
	if(index < m_SongIndex)
	{
		m_SongIndex--;
	}
	else if(m_SongIndex == index)
	{
		if(IsPlaying() && !IsPaused() && (m_SongIndex < m_Playlist.GetCount() - 1))
			_PostPlayRestart();
		else
			Stop();
	}
	m_Playlist.RemoveAt(index);
}

void  CMusikPlayer::MovePlaylistEntrys(size_t nMoveTo ,const wxArrayInt &arrToMove)
{
	//wxCriticalSectionLocker locker( m_critInternalData);
	wxASSERT(nMoveTo >= 0 && nMoveTo <= m_Playlist.GetCount()); 
	int i = arrToMove.GetCount() - 1;
	// first move all entrys which are behind nMoveTo;
	for(;i >= 0 ; i--)
	{

		if(nMoveTo > (size_t)arrToMove[i])
			break;
		size_t ToMoveIdx = arrToMove[i] + ( arrToMove.GetCount() - 1 - i);
		m_Playlist.Insert(m_Playlist.Detach(ToMoveIdx),nMoveTo);

		if(ToMoveIdx > m_SongIndex && nMoveTo <= m_SongIndex)
			m_SongIndex++;
		else if(ToMoveIdx == m_SongIndex)
			m_SongIndex = nMoveTo;
	}
	// now move all entry which are before
	for(int j = i; j >= 0; j--)
	{
		size_t MoveToIdx = nMoveTo - (i - j) - 1;
		m_Playlist.Insert(m_Playlist.Detach(arrToMove[j]),MoveToIdx);
		if((size_t)arrToMove[j] < m_SongIndex && MoveToIdx  > m_SongIndex)
			m_SongIndex--;
		else if( m_SongIndex  == MoveToIdx)
			m_SongIndex++;
		else if((size_t)arrToMove[j] == m_SongIndex)
			m_SongIndex = MoveToIdx;

	}
}
void CMusikPlayer::RefreshInternalPlaylist()
{

	for(int i = m_Playlist.GetCount() - 1 ;i >= 0 ;i--)
	{
		CMusikSong & refSong = m_Playlist[i];
		if(refSong.MetaData.eFormat == MUSIK_FORMAT_NETSTREAM)
			continue; // skip net streams
		if(false == wxGetApp().Library.GetSongFromSongid(refSong.songid,&refSong))
			m_Playlist.RemoveAt(i);
	}
}
