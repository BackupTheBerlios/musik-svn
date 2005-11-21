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
#include "myprec.h"

#include "MusikPlayer.h"
#include "Frames/MusikFrame.h"
#include "Threads/MusikThreads.h"
#include "Classes/NowPlayingCtrl.h" //TODO: remove the dependancy
#include "Classes/PlaylistCtrl.h" //TODO: remove the dependancy
#include "Classes/SourcesBox.h" //TODO: remove the dependancy
//--- globals: library / player / prefs ---//
#include "MusikGlobals.h"
#include "MusikUtils.h"
#include "MusikApp.h"
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
    EVT_MUSIKPLAYER_SONG_CHANGED( CMusikPlayer::_UpdateNetstreamMetadata )
	EVT_MENU			( MUSIK_PLAYER_STOP,			CMusikPlayer::OnPlayerStop		)
	EVT_MENU			( MUSIK_PLAYER_PLAY_RESTART,	CMusikPlayer::OnPlayRestart		)
	EVT_MENU			( MUSIK_PLAYER_RESUME,			CMusikPlayer::OnPlayerResume	)
END_EVENT_TABLE()

CMusikPlayer::CMusikPlayer()
	:wxEvtHandler()
        ,m_SndEngine(wxGetApp().Prefs.nSndRate,2,0)
	
{
	m_Playing		= false;
	m_Paused		= false;
	m_Fading		= false;
	m_BeginFade		= false;
	m_StartingNext	= false;
	m_Stopping		= false;
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
    int nSongIndex = 0;
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
					arr[0].ToLong((long *)&nSongIndex);
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
            m_Playlist.CurrentIndex(nSongIndex);
			if(m_Playlist.GetCount() == 0)
			{
				m_nLastSongTime = 0;
			}
		}
	}   
    //--- initialize fmod ---//
    if ( InitializeFMOD( FMOD_INIT_START ) != FMOD_INIT_SUCCESS )
        wxMessageBox( _("Initialization of FMOD sound system failed."), MUSIKAPPNAME_VERSION, wxOK | wxICON_ERROR );

    //--- startup the crossfader			---//
    g_FaderThread = new MusikFaderThread();
    g_FaderThread->Create();
    g_FaderThread->Run();

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
			if(!m_Playlist.GetCount())
			{
				m_nLastSongTime = 0;
			}
            Out.AddLine( wxString::Format(wxT("%d:%d"),m_Playlist.GetCount() ? m_Playlist.CurrentIndex():0, m_nLastSongTime));
			for ( size_t i = 0; i < m_Playlist.GetCount(); i++ )
			{
				Out.AddLine( m_Playlist[i].Song()->MetaData.Filename.GetFullPath() );
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
    {
        {
            CThreadController ThreadCtl;
            ThreadCtl.Attach(g_FaderThread);
            g_FaderThread = NULL;
        }
        Stop( true, true );
    }
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
#ifdef __WXMAC__
		FSOUND_SetBufferSize( 1000 );
#else        
		FSOUND_SetBufferSize( 100 );
#endif        
		Init_NetBuffer();	
		Init_ProxyServer();
		if ( FSOUND_Init( wxGetApp().Prefs.nSndRate , wxGetApp().Prefs.nSndMaxChan, 0 ) == FALSE )
			return FMOD_INIT_ERROR_INIT;
//		wxGetApp().Prefs.nSndOutput = FSOUND_GetOutput();
	}
	return FMOD_INIT_SUCCESS;
}
void CMusikPlayer::Init_NetBuffer	( )
{
	FSOUND_Stream_Net_SetBufferProperties(wxGetApp().Prefs.nStreamingBufferSize , wxGetApp().Prefs.nStreamingPreBufferPercent, wxGetApp().Prefs.nStreamingReBufferPercent);
}
void CMusikPlayer::Init_ProxyServer	( )
{
		FSOUND_Stream_Net_SetProxy(ConvW2A(wxGetApp().Prefs.GetProxyServer()));
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
			PlayByUser( m_Playlist.CurrentIndex() );
		}
	}
}
void CMusikPlayer::PlayReplaceList(int nItemToPlay,const MusikSongIdArray & playlist)
{
	if ( IsPaused() )
	{
		ClearOldStreams();
	}
	
	SetPlaylist( playlist );
	Play( nItemToPlay );
}
template < typename T,typename V>
bool PropChange(T & prop,const V &value )
{
    if(prop != value)
    {
        prop = value;
        return true;
    }
    return false;
};
template <  int,const char *>
bool PropChange(int & prop,const char* value )
{
    int nvalue = atoi(value);
    if(prop != nvalue)
    {
        prop = nvalue;
        return true;
    }
    return false;
};
signed char F_CALLBACKAPI CMusikPlayer::MetadataCallback(char *name, char *value, void* userdata)
{
 	CMusikPlayer * _this = (CMusikPlayer *)userdata;
	_this->_SetMetaData(name, value);
    return TRUE;
}
void CMusikPlayer::_SetMetaData(char *name, char *value)
{
	wxCriticalSectionLocker locker( m_critMetadata );
	wxString sMetadataName = ConvA2W(name).MakeUpper();
	    bool bChanged = false;	
	if (wxT("ARTIST") == sMetadataName)
	{
		//m_MetaDataSong.MetaData.Artist = value;
        bChanged = PropChange(m_MetaDataSong.MetaData.Artist,value);
	}
	else if (wxT("TITLE")== sMetadataName)
	{
		 bChanged = PropChange(m_MetaDataSong.MetaData.Title , value);
	}
	else if (wxT("ALBUM") == sMetadataName)
	{
		bChanged = PropChange(m_MetaDataSong.MetaData.Album , value);
	}
	else if (wxT("GENRE") == sMetadataName || wxT("ICY-GENRE") == sMetadataName)
	{
		bChanged = PropChange(m_MetaDataSong.MetaData.Genre , value);
	}
	else if (wxT("TRACKNUMBER") == sMetadataName)
	{
		bChanged = PropChange(m_MetaDataSong.MetaData.nTracknum , atoi(value));
	}
    else if (wxT("ICY-URL") == sMetadataName)
    {
        bChanged = PropChange(m_MetaDataSong.MetaData.Notes , value);
    }
    else if (wxT("ICY-BR") == sMetadataName)
    {
        bChanged = PropChange(m_MetaDataSong.MetaData.nBitrate , atoi(value));
    }
    if(bChanged)
    {
        MusikPlayerEvent ev(this,wxEVT_MUSIKPLAYER_SONG_CHANGED);
        AddPendingEvent( ev );
    }
    ::wxLogDebug(wxT("metadate received: name=%s,value=%s changed = %s"),(const wxChar*)sMetadataName,(const wxChar*)ConvA2W(value),bChanged ? wxT("true"):wxT("false") );
}
void CMusikPlayer::_UpdateNetstreamMetadata(MusikPlayerEvent& event)
{   // called in main thread context
	if(_CurrentSongIsNetStream())
	{
		{
			{
				wxCriticalSectionLocker locker( m_critMetadata );// protect m_MetaDataSong access
				CMusikSong & cursong = m_Playlist.Item( m_Playlist.CurrentIndex() ).SongCopy();
				cursong.MetaData.Artist = m_MetaDataSong.MetaData.Artist;
				cursong.MetaData.nTracknum = m_MetaDataSong.MetaData.nTracknum;
				cursong.MetaData.Genre = m_MetaDataSong.MetaData.Genre;
				cursong.MetaData.Title = m_MetaDataSong.MetaData.Title;
				cursong.MetaData.Album = m_MetaDataSong.MetaData.Album;
			}
			//wxCriticalSectionLocker locker( m_critInternalData );
			m_CurrentSong = m_Playlist.Item( m_Playlist.CurrentIndex() );
		}
		UpdateUI();
	}
    event.Skip();// give other clients a change to get this event
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
		Play(  m_Playlist.CurrentIndex() , event.GetInt() ); // start playing of current stream
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
    CSongPath sFilename;
    std::auto_ptr<CMusikSong> pNewSong = m_Playlist.Item( nItem ).Song();
    sFilename = pNewSong->MetaData.Filename;

	if (IsPlaying() && _CurrentSongIsNetStream())
	{	
		if(sFilename ==  m_CurrentSong.Song()->MetaData.Filename )
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


    if((pNewSong->MetaData.eFormat != MUSIK_FORMAT_NETSTREAM) && !sFilename.FileExists())
	{	// not a network stream
		// and
		// file does not exist
			wxMessageBox( _( "Cannot find file.\n Filename:" ) + sFilename.GetFullPath(), MUSIKAPPNAME_VERSION, wxICON_STOP );
			Stop();
			return false;
	}
	bool bSongChanged = (m_Playlist[nItem] != m_CurrentSong) && IsPlaying() && (nStartPos == 0);
	if(bSongChanged )
	{
	  // player is currently playing, so we have to record history here, because Stop() is not 
	  // called between the playing of the songs of the playlist.
			wxGetApp().Library.RecordSongHistory(m_CurrentSong,GetTime(UNIT_MILLISEC));
	}
	//---------------------------------------------//
	//--- start with the basics					---//
	//---------------------------------------------//
	m_Stopping		= false;
	m_CurrentSong	= m_Playlist.Item( nItem );
    m_Playlist.CurrentIndex(nItem);
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

	MUSIKStream* pNewStream = m_SndEngine.OpenMedia( ( const char* )ConvFn2A( sFilename.GetFullPath() ));
	if(pNewStream == NULL)
	{
		wxMessageBox( _( "Playback will be stopped, because loading failed.\n Filename:" ) + sFilename.GetFullPath(), MUSIKAPPNAME_VERSION, wxICON_STOP );
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
	//---------------------------------------------//
	//--- start playback on the new stream on	---//
	//--- the designated channel.				---//
	//---------------------------------------------//
	int retries = 2;
	bool bPlaySucceeded = false;
	pNewStream->SetTime( nStartPos * 1000 );
    pNewStream->SetVolume(  0.0 );
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
	if(_CurrentSongIsNetStream())
	{
		FSOUND_Stream_Net_SetMetadataCallback((FSOUND_STREAM*)pNewStream->GetStreamOut()->STREAM(), MetadataCallback, this);
	}

	m_Playing = true;
	m_Paused = false;
	if(g_FaderThread)
    	g_FaderThread->CrossfaderAbort();
	//---------------------------------------------//
	//--- update the global arrays containing	---//
	//--- active channels and streams			---//
	//---------------------------------------------//
	{
		wxCriticalSectionLocker lock(m_protectingStreamArrays);
		m_ActiveStreams.Add( pNewStream );
	}
	//---------------------------------------------//
	//--- playback has been started, update the	---//
	//--- user interface to reflect it			---//
	//---------------------------------------------//
    MusikPlayerEvent ev_start(this,wxEVT_MUSIKPLAYER_PLAY_START);
	ProcessEvent(ev_start);
    
	UpdateUI();//TODO: remove this. replace by event handling of playlistctrl etc.
	//---------------------------------------------//
	//--- record history in database			---//
	//---------------------------------------------//
	if(bSongChanged)
	{
        MusikPlayerEvent ev_songchange(this,wxEVT_MUSIKPLAYER_SONG_CHANGED);
        ProcessEvent(ev_songchange);
		wxGetApp().Library.UpdateItemLastPlayed	( m_CurrentSong );//TODO: replace by event handling of  wxEVT_MUSIKPLAYER_SONG_CHANGED in library
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
	CMusikSong & song = m_CurrentSong.SongCopy();
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
            {
                MusikPlayerEvent ev_songchange(this,wxEVT_MUSIKPLAYER_SONG_CHANGED);
                ProcessEvent(ev_songchange);
            }
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
	ActivateDSP();
}

void CMusikPlayer::ActivateDSP()
{
	if ( m_DSP )
	{
		if(wxGetApp().Prefs.bUseEQ)
		{
			g_FX.InitEQ(wxGetApp().Prefs.nSndRate);
		}
		FSOUND_DSP_SetActive( m_DSP, wxGetApp().Prefs.bUseEQ );
	}
}

void CMusikPlayer::FreeDSP()
{
	if ( m_DSP )
	{
		FSOUND_DSP_Free( m_DSP );
		m_DSP = NULL;
	}
}

void CMusikPlayer::SetFrequency(int freq)
{
	//-------------------------------------------------//
	//--- SiW: this causes problems. mp3s play fine	---//
	//--- but ogg's are slow. or vice versa.		---//
	//--- commented this out for the release.		---//
	//--- See Also: MusikFXDialog::MusikFXDialog()	---//
	//-------------------------------------------------//
	if ( wxGetApp().Prefs.bUsePitch )
		FSOUND_SetFrequency( FSOUND_ALL, freq );
	
}

void CMusikPlayer::UpdateUI()
{
	if(_CurrentSongIsNetStream())
	{
		g_PlaylistBox->PlaylistCtrl().ResynchItem( m_Playlist.CurrentIndex(),m_CurrentSong);
	}
	else
	{
		g_PlaylistBox->PlaylistCtrl().Update(false);
	}
	if(g_SourcesCtrl->GetSelType() == MUSIK_SOURCES_NOW_PLAYING)
	{
		g_PlaylistBox->PlaylistCtrl().EnsureVisible(m_Playlist.CurrentIndex());
	}
	

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
    MusikPlayerEvent ev_pause(this,wxEVT_MUSIKPLAYER_PLAY_PAUSE);
    ProcessEvent(ev_pause);

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
	m_SndEngine.SetPlayState( MUSIKEngine::Playing );
	m_Paused = false;
    MusikPlayerEvent ev(this,wxEVT_MUSIKPLAYER_PLAY_RESUME);
    ProcessEvent(ev);
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
//	SetVolume();
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
	if(m_CurrentSong.IsInLibrary() &&IsPlaying())
	{
		wxGetApp().Library.RecordSongHistory(m_CurrentSong,m_nLastSongTime);
	}
	else
		m_nLastSongTime = 0;
	m_CurrentSong = MusikSongId();
	m_Playing = false;
	m_Paused = false;
	m_Stopping = false;
	g_PlaylistBox->PlaylistCtrl().RefreshItem( m_Playlist.CurrentIndex() ); //TODO: replace by handling of MusikPlayerEvent
    
    MusikPlayerEvent ev(this,wxEVT_MUSIKPLAYER_PLAY_STOP);
    ProcessEvent(ev);
    
    wxCriticalSectionLocker lock(m_protectingStreamArrays);
	int nStreamCount = m_ActiveStreams.GetCount();
	for ( int i = 0; i < nStreamCount; i++ )
	{
		delete	m_ActiveStreams.Item( ( nStreamCount - 1 ) - i );
	}
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
			if(m_Playlist.Item( r ).Song()->MetaData.Filename.FileExists())
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
		if ( m_Playlist.CurrentIndex() >= m_Playlist.GetCount() - 1)
		{
			Stop();
            m_Playlist.CurrentIndex(0);
		}
		else
        {
			Play( m_Playlist.CurrentIndex() + 1  );
        }
		break;

	case MUSIK_PLAYMODE_REPEATSONG:
		Play( m_Playlist.CurrentIndex() );
		break;

	case MUSIK_PLAYMODE_REPEATLIST:
		if ( m_Playlist.CurrentIndex() == ( m_Playlist.GetCount()-1 ) )
			m_Playlist.CurrentIndex(0);
		else 
            m_Playlist.IncrCurrentIndex( );
		Play( m_Playlist.CurrentIndex());
		break;

	case MUSIK_PLAYMODE_SHUFFLE:
		{
			if (m_Playlist.GetCount())
			{
				if(m_Playlist.CurrentIndex() < m_Playlist.GetCount() - 1) 
				{  // check if the following song , should always be played(even if we are in shuffle mode)
					if(m_Playlist[m_Playlist.CurrentIndex() + 1].bForcePlay)
					{
						m_Playlist[m_Playlist.CurrentIndex() + 1].bForcePlay = 0; //reset flag
						m_Playlist.IncrCurrentIndex( );
						m_arrHistory.Add(m_Playlist.CurrentIndex()); // add to shuffle history too
						Play(m_Playlist.CurrentIndex());
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
	if ( m_ActiveStreams.GetCount() && GetTime( UNIT_MILLISEC ) > 2000 )	
	{
		Play( m_Playlist.CurrentIndex());
		return;
	} 
	else
	{
		switch ( m_Playmode )
		{
		case MUSIK_PLAYMODE_AUTO_DJ:
		case MUSIK_PLAYMODE_AUTO_DJ_ALBUM:
		case MUSIK_PLAYMODE_NORMAL:
			
			m_Playlist.DecrCurrentIndex( );
			Play( m_Playlist.CurrentIndex( ) );
			break;
	
		case MUSIK_PLAYMODE_REPEATSONG:
			Play( m_Playlist.CurrentIndex() );
			break;
	
		case MUSIK_PLAYMODE_REPEATLIST:
			if ( m_Playlist.CurrentIndex() <= 0 )
				m_Playlist.CurrentIndex(m_Playlist.GetCount() - 1);
            else
                m_Playlist.DecrCurrentIndex();
			Play( m_Playlist.CurrentIndex() );
			break;
	
		case MUSIK_PLAYMODE_SHUFFLE:
			if(m_arrHistory.GetCount() >= 2)
			{
				m_Playlist.CurrentIndex(m_arrHistory[m_arrHistory.GetCount() - 2]);
				Play( m_Playlist.CurrentIndex() );
			}
			break;
		}
	}
}
void CMusikPlayer::GetSongsToPlay( MusikSongIdArray & arrSongs)
{
    size_t cnt = m_Playlist.GetCount() - GetCurIndex();
    arrSongs.Alloc(cnt);
    for(size_t i = GetCurIndex(); i < m_Playlist.GetCount();i++)
    arrSongs.Add(m_Playlist[i]);
}
void CMusikPlayer::_AddRandomSongs()
{
	MusikSongIdArray  arrSongs;

	int nSongsToAdd = m_Playlist.GetCount() ? (wxGetApp().Prefs.nAutoDJChooseSongsToPlayInAdvance - (m_Playlist.GetCount() - 1 - m_Playlist.CurrentIndex()))
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

void CMusikPlayer::_ChooseRandomSongs(int nSongsToAdd,MusikSongIdArray &arrSongs)
{
	if(nSongsToAdd <= 0)
		return;
    MusikSongIdArray arrSongsToPlay;
    GetSongsToPlay(arrSongsToPlay);

    wxString sQueryBase(wxT("select autodj_songs.songid from autodj_songs "));
    wxString sWhereNotSongsToPlay;
    wxString sQuerySum(wxT(" select (sum(duration)/60000) from autodj_songs"));
    if(arrSongsToPlay.GetCount())
    {
        sWhereNotSongsToPlay << wxT(" autodj_songs.songid not in(")
                  << arrSongsToPlay.AsCommaSeparatedString()
                  << wxT(") ");
        sQuerySum << wxT(" where ") << sWhereNotSongsToPlay;

    }
    sQuerySum <<  wxT(";"); 


    int nTotalPlayTimeMinutes = wxGetApp().Library.QueryCount(ConvToUTF8(sQuerySum));
    // we dont want to play songs which have been played the last n minutes.
    // n is a user config option
    // but sometimes our possible songs  have a total playing time which is smaller than n
    // in this case we use the the half of the total playing time of the songlist instead of n

    int nDoNotPlaySongPlayedTheLastNMinutes = wxMin(nTotalPlayTimeMinutes / 2 , (int)wxGetApp().Prefs.nAutoDjDoNotPlaySongPlayedTheLastNHours * 60);

    wxString sWhereNotPlayedTheLastNMinutes;
    sWhereNotPlayedTheLastNMinutes << wxT(" lastplayed = '' or lastplayed < julianday('now','-") << nDoNotPlaySongPlayedTheLastNMinutes << wxT(" minutes') ") ;
    wxString sQuerySongs;
    sQuerySongs << sQueryBase << wxT("where ");
    if(sWhereNotSongsToPlay.IsEmpty())
    {
        sQuerySongs << sWhereNotPlayedTheLastNMinutes;
    }
    else
    {
       sQuerySongs << wxT("(") << sWhereNotSongsToPlay << wxT(") and (") << sWhereNotPlayedTheLastNMinutes << wxT(")");
    }
    sQuerySongs <<  wxT(";"); 

	wxArrayInt arrSongIds;
	wxGetApp().Library.Query(sQuerySongs,arrSongIds);
	while (arrSongIds.GetCount() && (arrSongs.GetCount() < (size_t)nSongsToAdd))
	{
		int r = (size_t) (arrSongIds.GetCount() * (GetRandomNumber() / (RandomMax + 1.0)));  // random range  [0 , arrSongIds.GetCount()-1] 
		CMusikSong song;
		if(wxGetApp().Library.GetSongFromSongid(arrSongIds[r],&song))
		{
			if(song.MetaData.Filename.FileExists())
			{
				arrSongs.Add(MusikSongId(arrSongIds[r]));
			}
			arrSongIds.RemoveAt(r); //remove index, so songid cannot be chosen again.
		}
	} 
}
void CMusikPlayer::_ChooseRandomAlbumSongs(int nAlbumsToAdd,MusikSongIdArray &arrAlbumSongs)
{
	if(nAlbumsToAdd <= 0)
		return;
	int nMaxRepeatCount = 30;
    int nTotalPlayTimeMinutes = wxGetApp().Library.QueryCount(" select (sum(duration)/60000) from autodj_songs;");
    int nDoNotPlaySongPlayedTheLastNMinutes = wxMin(nTotalPlayTimeMinutes / 2 , (int)wxGetApp().Prefs.nAutoDjDoNotPlaySongPlayedTheLastNHours * 60);
	char * count_query = sqlite_mprintf("select count(*) from autodj_albums where most_lastplayed = '' or most_lastplayed < julianday('now','-%d minutes');",nDoNotPlaySongPlayedTheLastNMinutes);
	int albums_count = wxGetApp().Library.QueryCount(count_query);
	sqlite_freemem( count_query );
	wxArrayString arrAlbums;
	while ( (nMaxRepeatCount > 0) && ( arrAlbums.GetCount() < (size_t)nAlbumsToAdd ))
	{
		int r = (size_t) (albums_count * (GetRandomNumber() / (RandomMax + 1.0)));  // random range  [0 , albums_count-1] 
		wxString sQueryRandomAlbum = wxString::Format(wxT("select album||'|'||artist from autodj_albums where most_lastplayed = '' or most_lastplayed < julianday('now','-%d minutes') limit 1 offset %d;") 
											,nMaxRepeatCount < 5 ? 1 : nDoNotPlaySongPlayedTheLastNMinutes,r);

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

void CMusikPlayer::SetVolume(int vol )
{
    if(vol > 255)
        vol = 255;
    if(vol >= 0)
        wxGetApp().Prefs.nSndVolume = vol;
    if ( g_FaderThread && g_FaderThread->IsCrossfaderActive() )
        g_FaderThread->CrossfaderStop();
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
	if ( nType == UNIT_SEC )
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
			{   // openstate is not ready
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
	if ( nType == UNIT_SEC )
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
	int nTimeLeft = GetDuration( UNIT_MILLISEC ) - GetTime( UNIT_MILLISEC ); 
	if ( nType == UNIT_SEC )
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
    default:
      break;
		}

		return wxString::Format( wxT( "%s (%d%%)"), (const wxChar *)sProgress, GetTime( UNIT_SEC ));
		
	}
	else
		return SecToStr( GetTime( UNIT_SEC ) ) + wxT("/") + SecToStr(GetDuration( UNIT_SEC ));
}
wxString CMusikPlayer::GetTimeLeftStr()
{
	if(_CurrentSongIsNetStream())
	{
		return GetTimeStr();
	}
	else
		return SecToStr( GetTimeLeft( UNIT_SEC ) );
}
void CMusikPlayer::SetTime( int nSec )
{
	if(_IsSeekCrossFadingDisabled())
	{
		m_ActiveStreams.Item( m_ActiveStreams.GetCount() - 1 )->SetTime( nSec * 1000 );
	}
	else
	{
		Play( m_Playlist.CurrentIndex(), nSec, CROSSFADE_SEEK );
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
    std::auto_ptr<CMusikSong> pSong = m_CurrentSong.Song();
	return (wxGetApp().Prefs.bUse_MPEGACCURATE_ForMP3VBRFiles
			&&(m_Playlist.CurrentIndex() <  m_Playlist.GetCount())
			&& pSong->MetaData.bVBR 
			&& (pSong->MetaData.eFormat == MUSIK_FORMAT_MP3) ); 
	//  mp3 vbr files needs FSOUND_MPEGACCURATE flag, but takes much too long for other filetypes
}
bool CMusikPlayer::_CurrentSongIsNetStream()
{
	return m_CurrentSong.IsFormat(MUSIK_FORMAT_NETSTREAM); 
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

void CMusikPlayer::ClearPlaylist()
{ 
	if(wxGetApp().Prefs.bStopSongOnNowPlayingClear)
		wxGetApp().Player.Stop();
	SetPlaylist(MusikSongIdArray());	
}

void CMusikPlayer::AddToPlaylist( MusikSongIdArray & songstoadd ,bool bPlayFirstAdded )
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
			m_Playlist.CurrentIndex(plsize);
			m_bSuppressAutomaticSongPicking = true;
			Play(m_Playlist.CurrentIndex());
		}
	}
}
void CMusikPlayer::InsertToPlaylist( MusikSongIdArray & songstoadd ,bool bPlayFirstInserted)
{
	//wxCriticalSectionLocker locker( m_critInternalData);
	size_t size = songstoadd.GetCount();
	for(size_t i = 0; i < size; i++)
		songstoadd[i].bForcePlay=1;// mark each song such as it is played even in shuffle mode

	size_t plsize = m_Playlist.GetCount();
	if(plsize == 0 || !IsPlaying())
	{// list empty, add
		AddToPlaylist(songstoadd,bPlayFirstInserted);
		return;

	}
// list not empty ,insert
    int nSongIndex = m_Playlist.CurrentIndex();
	for(size_t i = 0; i < size ; i++)
	{
		m_Playlist.Insert(songstoadd.Detach(0), nSongIndex + 1 + i );
	}
	if(bPlayFirstInserted)
	{
		m_Playlist.CurrentIndex(nSongIndex + 1);
		m_bSuppressAutomaticSongPicking = true;
		Play(m_Playlist.CurrentIndex());
	}
}
void CMusikPlayer::OnPlaylistEntryRemoving( size_t index )
{
	//wxCriticalSectionLocker locker( m_critInternalData);
	
	if(m_Playlist.CurrentIndex() == index)
	{
		if(IsPlaying() && !IsPaused() )
			_PostPlayRestart();
		else
			Stop();
	}
}


DEFINE_EVENT_TYPE(wxEVT_MUSIKPLAYER_SONG_CHANGED)
DEFINE_EVENT_TYPE(wxEVT_MUSIKPLAYER_PLAY_START)
DEFINE_EVENT_TYPE(wxEVT_MUSIKPLAYER_PLAY_STOP)
DEFINE_EVENT_TYPE(wxEVT_MUSIKPLAYER_PLAY_PAUSE)
DEFINE_EVENT_TYPE(wxEVT_MUSIKPLAYER_PLAY_RESUME)
IMPLEMENT_DYNAMIC_CLASS(MusikPlayerEvent, wxCommandEvent)

MusikPlayerEvent::MusikPlayerEvent(CMusikPlayer *pMP, wxEventType type)
: wxCommandEvent(type)
{
    SetEventObject(pMP);
}

CMusikPlayer & MusikPlayerEvent::MusikPlayer()
{
    return *wxDynamicCast(GetEventObject(),CMusikPlayer); 
}
