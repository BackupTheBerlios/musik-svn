/*
 *  MusikPlayer.h
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

#ifndef MUSIK_PLAYER_H
#define MUSIK_PLAYER_H

//--- wx stuff ---//
#include "wx/wxprec.h"
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include <wx/dynarray.h>

enum EMUSIK_CROSSFADER_TYPE
{
	CROSSFADE_ERROR = 0,
	CROSSFADE_NORMAL,
	CROSSFADE_PAUSE,
	CROSSFADE_RESUME,
	CROSSFADE_STOP,
	CROSSFADE_EXIT,
	CROSSFADE_SEEK
};


//--- CMusikSongArray defined here ---//
#include "MusikLibrary.h"
#include "MUSIKEngine/FMODEngine/inc/fmodengine.h"
//--- CMusikStreamArray declaration ---//
WX_DECLARE_OBJARRAY( MUSIKStream*, CMusikStreamArray );


enum EFMOD_INIT
{
	FMOD_INIT_STOP = 0,
	FMOD_INIT_START,
	FMOD_INIT_RESTART,
	FMOD_INIT_SET_NETBUFFER,
	FMOD_INIT_ERROR_DSOUND,
	FMOD_INIT_ERROR_WINDOWS_WAVEFORM,
	FMOD_INIT_ERROR_ASIO,
	FMOD_INIT_ERROR_DEVICE_NOT_READY,
	FMOD_INIT_ERROR_INIT,
	FMOD_INIT_ERROR_OSS,
	FMOD_INIT_ERROR_ESD,
	FMOD_INIT_ERROR_ALSA,
	FMOD_INIT_ERROR_MAC,
	FMOD_INIT_SUCCESS
};

enum EFMOD_MISC
{
	FMOD_SEC = 0,
	FMOD_MSEC
};

class CMusikPlayer : public wxEvtHandler
{
public:
	CMusikPlayer();
	~CMusikPlayer();

	void Init(bool bSuppressAutoPlay);
	//--------------------//
	//--- sound system ---//
	//--------------------//
	int	 InitializeFMOD		( int nFunction );
	void InitFMOD_NetBuffer	( );
	void InitFMOD_ProxyServer	( );

	void Shutdown			( bool bClose = true );
	void ClearOldStreams	( bool bClearAll = false );
	void SetFrequency		( );
	void InitDSP			( );
	void ActivateDSP		( );
	void FreeDSP			( );

	//------------------------------//
	//--- events threads may use ---//
	//------------------------------//
	void OnNextSongEvt		( wxCommandEvent& WXUNUSED(event) ){ NextSong(); SetStartingNext( false );	}
	void OnFadeCompleteEvt	( wxCommandEvent& event );
	void OnPlayerStop		( wxCommandEvent& WXUNUSED(event) ){ FinalizeStop();						}
	void OnPlayRestart		( wxCommandEvent& event );	
	void OnPlayerResume		( wxCommandEvent& WXUNUSED(event) ){ FinalizeResume();						}

	//-------------------------------------//
	//--- crossfader thread calls		---//
	//--- these functions to clean up	---//
	//-------------------------------------//
	void FinalizePause		( );
	void FinalizeResume		( );
	void FinalizeStop		( );

	//----------------//
	//--- booleans ---//
	//----------------//
	bool IsPlaying			( ) { return ( m_Playing );		}
	bool IsPaused			( ) { return ( m_Paused );		}
	bool IsFading			( ) { return ( m_Fading );		}
	bool IsStartingNext		( ) { return ( m_StartingNext );}
	bool IsStopping			( ) { return ( m_Stopping );	}
	bool IsSeekable			( ) { return IsPlaying() && !_CurrentSongIsNetStream();}
	bool BeginFade			( )	{ return ( m_BeginFade );	}
	void CaughtBeginFade	( )	{ m_BeginFade = false;		}

	//-------------------------//
	//--- playback controls ---//
	//-------------------------//
	void PlayReplaceList(int nItemToPlay,const CMusikSongArray & playlist);
	void PlayPause();
    bool Play		( size_t nItem, int nStartPos = 0, int nFadeType = CROSSFADE_NORMAL );
	bool PlayByUser	( size_t nItem)
	{
		if (m_Playlist.GetCount() && (nItem < m_Playlist.GetCount()) )
				m_Playlist[nItem].bChosenByUser = 1;   // set this flag explicitly
		return Play(nItem);
	}
	void Stop		( bool bFade = true, bool bExit = false );
	void NextSong	( );		
	void PrevSong	( );		
	void Pause		( bool bFade = true );
	void Resume		( bool bFade = true );

	//------------//
	//--- gets ---//
	//------------//
	size_t	GetCurIndex			( ) { return ( m_SongIndex ); }
	int	 GetDuration			( int nType );
	int	 GetLastTime			( int nType );
	int	 GetTime				( int nType );
	int	 GetTimeLeft			( int nType );
	
	wxString GetTimeStr			( );
	wxString GetTimeLeftStr		( );
	int GetCurrentSongid		() { return m_CurrentSong.songid; }
	wxString  GetCurrentFilename() { return m_CurrentSong.MetaData.Filename.GetFullPath(); }
	int GetCrossfadeType		() { return m_CrossfadeType; }
	size_t GetShuffledSong		();
	EMUSIK_PLAYMODE GetPlaymode () { return m_Playmode; }
	const CMusikSongArray &	GetPlaylist	( ) 
	{
		RefreshInternalPlaylist();
		return  m_Playlist;
	}
	void RefreshInternalPlaylist();
	
	//------------//
	//--- sets ---//
	//------------//
	void SetFadeBegin		( ){ m_Fading = true; }
	void SetFadeStart		( );
	void SetPlaymode		( EMUSIK_PLAYMODE pm );
	void SetVolume			( );
	void SetTime			( int nSec );
	void SetPlaylist		(const  CMusikSongArray &playlist ) { m_Playlist = playlist;m_arrHistory.Clear(); }
	void AddToPlaylist		( CMusikSongArray &songstoadd ,bool bPlayFirstAdded = true);	// NOTE this method, empties the songstoadd array.
	void InsertToPlaylist(	 CMusikSongArray & songstoadd ,bool bPlayFirstInserted = true);  // NOTE this method, empties the songstoadd array.
	void RemovePlaylistEntry( size_t index );
	void MovePlaylistEntrys	(size_t nMoveTo ,const wxArrayInt &arrToMove);
	void SetStartingNext	( bool bStart = true ){ m_StartingNext = bStart; }
	void SetCrossfadeType	( int nType ){ m_CrossfadeType = nType; }

	//------------//
	//--- misc ---//
	//------------//
	void UpdateUI		( );
	
	DECLARE_EVENT_TABLE()
	
private:
	void _PostPlayRestart( int nStartPos = 0 );
	bool _IsSeekCrossFadingDisabled();
	bool _CurrentSongNeedsMPEGACCURATE();
	bool _CurrentSongIsNetStream();
	bool _IsNETSTREAMConnecting() { return (m_p_NETSTREAM_Connecting != NULL);}
	static signed char F_CALLBACKAPI MetadataCallback(char *name, char *value, void * userdata);
	void _SetMetaData(char *name, char *value);
	void _UpdateNetstreamMetadata( wxCommandEvent& event );
	void _AddRandomSongs();
	void _ChooseRandomSongs(int nSongsToAdd,CMusikSongArray &arrSongs);
	void _ChooseRandomAlbumSongs(int nAlbumsToAdd,CMusikSongArray &arrAlbumSongs);
	int _NetStreamStatusUpdate(MUSIKStream * pStream);

	
	CMusikSongArray m_Playlist;			//--- heart and soul.								---//
	EMUSIK_PLAYMODE	m_Playmode;			//--- repeat, shuffle, etc							---//
	size_t			m_SongIndex;		//--- current index in playlist						---//
	bool			m_Playing;			//--- currently playing?							---//
	bool			m_Paused;			//--- currently paused?								---//
	bool			m_BeginFade;		//--- should we begin fading?						---//
	bool			m_Fading;			//--- currently (cross)fading?						---//
	bool			m_StartingNext;		//--- in the middle of starting up a next song?		---//
	bool			m_Stopping;			//--- is the player currently stopping?				---//
	CMusikSong		m_CurrentSong;		//--- copy of current song						---//0
	int				m_CrossfadeType;	
	wxArrayInt		m_arrHistory;		//--- history of songs played, to avoid repeats		---//
	size_t			m_nMaxHistory;
	FSOUND_DSPUNIT	*m_DSP;

	int m_NETSTREAM_read_percent;
	int m_NETSTREAM_last_read_percent;
	int m_NETSTREAM_bitrate;
	int m_NETSTREAM_status;
	unsigned int m_NETSTREAM_flags;
	bool m_b_NETSTREAM_AbortConnect;
	bool m_bSuppressAutomaticSongPicking;
	MUSIKStream * m_p_NETSTREAM_Connecting;
	CMusikSong m_MetaDataSong;
	wxCriticalSection m_critMetadata;
	//wxCriticalSection m_critInternalData;
	wxStopWatch m_StreamIsWorkingStopWatch;
	bool m_bStreamIsWorkingStopWatchIsRunning;
	int m_nLastSongTime;

	bool m_bPostPlayRestartInProgress;
	FMODEngine m_SndEngine;
protected:

	wxCriticalSection	m_protectingStreamArrays; // to protect access to ActiveStreams and ActiveChannels
	CMusikStreamArray	m_ActiveStreams;
friend class MusikCrossfaderThread;
};

#endif
