/*
 *  PlaylistCtrlThreads.h
 *
 *  Threads that relates to PlaylistCtrl.h / cpp
 *  
 *  Copyright (c) 2003 Casey Langen (casey@bak.rr.com)
 *	Contributors: Simon Windmill, Dustin Carter, Gunnar Roth, Wade Brainerd
 *
 *  See the file "license.txt" for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL WARRANTIES.
*/

#ifndef MUSIK_PLAYLISTCTRL_THREADS_H
#define MUSIK_PLAYLISTCTRL_THREADS_H

//--- wx ---//
#include "wx/wxprec.h"
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif 

#include "wx/thread.h"

#include "../Classes/MusikLibrary.h"

//-------------------------------------------------//
//--- NOTE: post all events to g_PlaylistCtrl.	---//
//--- It will relay events to g_MusikFrame		---//
//-------------------------------------------------//
class MusikPlaylistRenameThread : public wxThread
{
public:
	MusikPlaylistRenameThread( wxEvtHandler * pEvtHandler, const  CMusikSongArray & songs );

	virtual void *Entry();
	virtual void OnExit();

private:

	CMusikSongArray m_Songs;
	wxEvtHandler * m_pEvtHandler;
};

class MusikPlaylistRetagThread : public wxThread
{
public:
	MusikPlaylistRetagThread(wxEvtHandler * pEvtHandler , const wxString &TagMask, const  CMusikSongArray & songs );

	virtual void *Entry();
	virtual void OnExit();
	const CMusikSongArray & GetReTaggedSongs(){return m_Songs;}
private:
	CMusikSongArray m_Songs;
	wxString m_sTagMask;
	wxEvtHandler * m_pEvtHandler;
};

#endif

