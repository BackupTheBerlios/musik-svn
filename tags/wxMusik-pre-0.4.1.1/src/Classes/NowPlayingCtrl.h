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

#ifndef MUSIK_NOWPLAYING_CTRL
#define MUSIK_NOWPLAYING_CTRL

#include "wx/wxprec.h"
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif 
#include "../Classes/MusikLibrary.h"


enum EMUSIK_NOWPLAYINGCTRL_ID
{
	MUSIK_NOWPLAYINGCTRL_PLAYPAUSE = 0,
	MUSIK_NOWPLAYINGCTRL_NEXT,
	MUSIK_NOWPLAYINGCTRL_PREV,
	MUSIK_NOWPLAYINGCTRL_STOP,
	MUSIK_NOWPLAYINGCTRL_VOLUME,
	MUSIK_NOWPLAYINGCTRL_PLAYMODE,
	MUSIK_NOWPLAYINGCTRL_TIMEDISPLAY,
	MUSIK_NOWPLAYINGCTRL_SEEK

};

class CTunage;

//--- bitmap button and seek bar event handler ---//
#include "BtnDown.h"
#include "GaugeSeek.h"

class CNowPlayingCtrl : public wxPanel
{
public:
	CNowPlayingCtrl( wxWindow *pParent );
	~CNowPlayingCtrl();
	void ActivateHotkeys();
	void DeactivateHotkeys();
	//--- events ---//
	void PlayerStop			( wxCommandEvent&	WXUNUSED(event) );
	void PlayerPlayPause	( wxCommandEvent&	WXUNUSED(event) );	
	void PlayerNext			( wxCommandEvent&	WXUNUSED(event) );
	void PlayerPrev			( wxCommandEvent&	WXUNUSED(event) );	
	void PlayerStopHK			( wxKeyEvent&	WXUNUSED(event) ){ wxCommandEvent dummy; PlayerStop(dummy);}
	void PlayerPlayPauseHK	( wxKeyEvent&		WXUNUSED(event) ){ wxCommandEvent dummy; PlayerPlayPause(dummy);}
	void PlayerNextHK			( wxKeyEvent&	WXUNUSED(event) ){ wxCommandEvent dummy; PlayerNext(dummy);}
	void PlayerPrevHK			( wxKeyEvent&	WXUNUSED(event) ){ wxCommandEvent dummy; PlayerPrev(dummy);}	

	void PlayerVolume		( wxCommandEvent&	WXUNUSED(event) );	
	void OnTimer			( wxTimerEvent&		WXUNUSED(event) );
	void OnPlayMode			( wxCommandEvent&	event );
	void OnClickTimeDisplay		( wxMouseEvent&	event );
	void OnCheckCrossfade	( wxCommandEvent&	event );
	//--- play control bitmaps ---//
	wxBitmap	bmPrev,		bmPrevDown;
	wxBitmap	bmNext,		bmNextDown;
	wxBitmap	bmPlay,		bmPlayDown;
	wxBitmap	bmPause,	bmPauseDown;
	wxBitmap	bmStop,		bmStopDown;
	wxBitmap	bmVolume,	bmVolumeDown;

	//--- playlist buttons ---//
	wxBitmapButton	*btnPlayPause;
	wxBitmapButton	*btnNext;
	wxBitmapButton	*btnPrev;
	wxBitmapButton	*btnStop;
	wxBitmapButton	*btnVolume;

	//--- seek bar ---//
	wxGauge *gSeek;

	//--- seek bar custom event ---//
	CGaugeSeekEvt *pSeekEvt;

	//--- now playing text objects ---//
	wxStaticText_NoFlicker *stSong;
	wxStaticText_NoFlicker *stArtist;
	wxStaticText_NoFlicker *stCurtime;


	//--- sizers ---//
	wxBoxSizer *hsArtistTime;
	wxBoxSizer *vsLeftCol;
	wxBoxSizer *vsRightCol;

	//--- button down events (win32) ---//
	#ifndef __WXGTK__
	CBtnDownEvt *pPrevEvt;
	CBtnDownEvt *pNextEvt;
	CBtnDownEvt *pPlayEvt;
	CBtnDownEvt *pPauseEvt;
	CBtnDownEvt *pStopEvt;
	CBtnDownEvt *pVolumeEvt;
	#endif

	//--- funcs ---//
	void PlayBtnToPauseBtn();
	void PauseBtnToPlayBtn();
	void UpdateInfo( const CMusikSong &song );
	void SetTime( wxString sTimeStr );
	void ResetInfo();

	DECLARE_EVENT_TABLE()
private:
	void UpdateTime();
	void StartTimer();
	void KillTimer();
	wxTimer *pSecTimer;

	wxFileName m_LastFile;
	CTunage* m_pTunage;
};

#endif
