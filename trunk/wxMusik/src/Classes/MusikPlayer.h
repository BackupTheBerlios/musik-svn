/*
 *  MusikPlayer.h
 *
 *  Object which controls audio playback.
 *
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
#include "myprec.h"
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include <wx/dynarray.h>
#include "Classes/MusikPlayerEvt.h"

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


//--- MusikSongIdArray defined here ---//
#include "MusikLibrary.h"


#if defined(USE_FMODEX)
#include "MUSIKEngine/FMODExEngine/inc/fmodexengine.h"
typedef FMODExEngine Engine_t;
#elif defined(USE_FMOD3)
#include "MUSIKEngine/FMODEngine/inc/fmodengine.h"
typedef FMODEngine Engine_t;
#elif defined(USE_XINE)
#include "MUSIKEngine/XineEngine/inc/xineengine.h"
typedef XineEngine Engine_t;
#endif

//--- CMusikStreamArray declaration ---//
WX_DECLARE_OBJARRAY( MUSIKStream*, CMusikStreamArray );



enum EUNIT_TIME
{
	UNIT_SEC = 0,
	UNIT_MILLISEC
};

class CMusikPlayer : public wxEvtHandler,public MUSIKStream::IMetadataCallback
{
public:
	CMusikPlayer();
	~CMusikPlayer();

	void Init(bool bSuppressAutoPlay);
	//--------------------//
	//--- sound system ---//
	//--------------------//
	bool InitializeSndEngine();
	void Init_NetBuffer	( );
	void Init_ProxyServer	( );

	void Shutdown			( bool bClose = true ,bool bNoFade = false);
	void ClearOldStreams	( bool bClearAll = false );
	void SetFrequency		( int freq );

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
	void PlayReplaceList(int nItemToPlay,const MusikSongIdArray & playlist);
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
	size_t	GetCurIndex			( ) { return  m_Playlist.CurrentIndex(); }
	int	 GetDuration			( int nType );
	int	 GetLastTime			( int nType );
	int	 GetTime				( int nType );
	int	 GetTimeLeft			( int nType );
	
	wxString GetTimeStr			( );
	wxString GetTimeLeftStr		( );
	MusikSongId GetCurrentSongid		() { return m_CurrentSong; }
	wxString  GetCurrentFilename() { return m_CurrentSong.Song()->MetaData.Filename.GetFullPath(); }
    void GetSongsToPlay( MusikSongIdArray & arrSongs);
	int GetCrossfadeType		() { return m_CrossfadeType; }
	size_t GetShuffledSong		();
	EMUSIK_PLAYMODE GetPlaymode () { return m_Playmode; }
	MusikSongIdArray &	GetPlaylist	( ) 
	{
		return  m_Playlist;
	}
	MUSIKEngine & SndEngine()
    {
        return m_SndEngine;
    }
	//------------//
	//--- sets ---//
	//------------//
	void SetFadeBegin		( ){ m_Fading = true; }
	void SetFadeStart		( );
	void SetPlaymode		( EMUSIK_PLAYMODE pm );
	void SetVolume			( int vol = -1);
	void SetTime			( int nSec );
	void ClearPlaylist		();
	void SetPlaylist		(const  MusikSongIdArray &playlist ) { m_Playlist = playlist;m_arrHistory.Clear(); }
	void AddToPlaylist		( MusikSongIdArray &songstoadd ,bool bPlayFirstAdded = true);	// NOTE this method, empties the songstoadd array.
	void InsertToPlaylist(	 MusikSongIdArray & songstoadd ,bool bPlayFirstInserted = true);  // NOTE this method, empties the songstoadd array.
	void OnPlaylistEntryRemoving( size_t index );
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
	void MetadataCallback( MUSIKStream * pStream,const char *name,const  char *value);
	void _UpdateNetstreamMetadata( MusikPlayerEvent& event );
	void _AddRandomSongs();
	void _ChooseRandomSongs(int nSongsToAdd,MusikSongIdArray &arrSongs);
	void _ChooseRandomAlbumSongs(int nAlbumsToAdd,MusikSongIdArray &arrAlbumSongs);
	MUSIKEngine::NetStatus _NetStreamStatusUpdate(MUSIKStream * pStream);

	
	MusikSongIdArray m_Playlist;			//--- heart and soul.								---//
	EMUSIK_PLAYMODE	m_Playmode;			//--- repeat, shuffle, etc							---//
	bool			m_Playing;			//--- currently playing?							---//
	bool			m_Paused;			//--- currently paused?								---//
	bool			m_BeginFade;		//--- should we begin fading?						---//
	bool			m_Fading;			//--- currently (cross)fading?						---//
	bool			m_StartingNext;		//--- in the middle of starting up a next song?		---//
	bool			m_Stopping;			//--- is the player currently stopping?				---//
	MusikSongId		m_CurrentSong;		//--- id of current song						---//0
	int				m_CrossfadeType;	
	wxArrayInt		m_arrHistory;		//--- history of songs played, to avoid repeats		---//
	size_t			m_nMaxHistory;

	int m_NETSTREAM_read_percent;
	int m_NETSTREAM_last_read_percent;
	int m_NETSTREAM_bitrate;
	MUSIKEngine::NetStatus m_NETSTREAM_status;
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
	Engine_t m_SndEngine;
protected:

	wxCriticalSection	m_protectingStreamArrays; // to protect access to ActiveStreams and ActiveChannels
	CMusikStreamArray	m_ActiveStreams;
friend class MusikCrossfaderThread;
};

#endif
