/*
 *  PlaylistCtrlThreads.cpp
 *
 *  Threads that relates to PlaylistCtrl.h / cpp
 *  
 *  Copyright (c) 2003 Casey Langen (casey@bak.rr.com)
 *	Contributors: Simon Windmill, Dustin Carter, Gunnar Roth, Wade Brainerd
 *
 *  See the file "license.txt" for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL WARRANTIES.
*/

//--- For compilers that support precompilation, includes "wx/wx.h". ---//
#include "myprec.h"
#include "PlaylistCtrlThreads.h"
#include "../Classes/PlaylistCtrl.h"
//--- globals ---//
#include "../MusikGlobals.h"
#include "../MusikUtils.h"

#include "../Frames/MusikFrame.h"

MusikPlaylistRenameThread::MusikPlaylistRenameThread(wxEvtHandler * pEvtHandler, const  MusikSongIdArray & songs )
	:wxThread(wxTHREAD_JOINABLE)
	,m_pEvtHandler(pEvtHandler)
{
	m_Songs = songs;
}

void* MusikPlaylistRenameThread::Entry()
{
	//--- setup thread to begin in g_MusikFrame ---//
	g_MusikFrame->SetActiveThread	( this );
	g_MusikFrame->SetProgressType	( MUSIK_PLAYLIST_RENAME_THREAD );
	g_MusikFrame->SetProgress		( 0 );
	//----------------------------------------//
	//--- events we'll post as we go along ---//
	//----------------------------------------//
	wxCommandEvent RenameStartEvt	( wxEVT_COMMAND_MENU_SELECTED, MUSIK_PLAYLIST_THREAD_START );
	RenameStartEvt.SetExtraLong(MUSIK_PLAYLIST_RENAME_THREAD);
	wxCommandEvent RenameProgEvt	( wxEVT_COMMAND_MENU_SELECTED, MUSIK_PLAYLIST_THREAD_PROG );	

	wxPostEvent( m_pEvtHandler, RenameStartEvt );
    std::auto_ptr<CMusikLibrary> pSlaveLibrary (wxGetApp().Library.CreateSlave());

    float fPos = 0;
	int nLastProg = 0;
	int nCurrProg = 0;
	for ( size_t i = 0; i < m_Songs.GetCount(); i++ )
	{
        if(i % 100 == 99)
        {
            pSlaveLibrary->EndTransaction();
            pSlaveLibrary->BeginTransaction();
        }
	//-----------------------//
		//--- update progress ---//
		//-----------------------//
		fPos = (float)( i * 100 ) / (float)m_Songs.GetCount();
		nCurrProg = (int)fPos;
		if ( nCurrProg > nLastProg )
		{
			RenameProgEvt.SetExtraLong( nCurrProg );
			wxPostEvent( m_pEvtHandler, RenameProgEvt );
		}
		nLastProg = nCurrProg;

		if ( TestDestroy() )
			break;
        std::auto_ptr<CMusikSong> pSong(m_Songs.Item( i ).Song());
		if(false == pSlaveLibrary->RenameFile( *pSong ))
			::wxLogWarning(_("Renaming of file %s failed."),(const wxChar *)pSong->MetaData.Filename.GetFullPath());

	}
	return NULL;
}

void MusikPlaylistRenameThread::OnExit()
{
	wxCommandEvent RenameEndEvt( wxEVT_COMMAND_MENU_SELECTED, MUSIK_PLAYLIST_THREAD_END );
	wxPostEvent( m_pEvtHandler, RenameEndEvt );
}

MusikPlaylistRetagThread::MusikPlaylistRetagThread(wxEvtHandler * pEvtHandler, const wxString &TagMask, const  MusikSongIdArray & songs )
	:wxThread(wxTHREAD_JOINABLE)
	,m_pEvtHandler(pEvtHandler)
{
	m_Songs = songs;
	m_sTagMask = TagMask;
}

void* MusikPlaylistRetagThread::Entry()
{
	//--- setup thread to begin in g_MusikFrame ---//
	g_MusikFrame->SetActiveThread	( this );
	g_MusikFrame->SetProgressType	( MUSIK_PLAYLIST_RETAG_THREAD );
	g_MusikFrame->SetProgress		( 0 );
	//----------------------------------------//
	//--- events we'll post as we go along ---//
	//----------------------------------------//
	wxCommandEvent RetagStartEvt	( wxEVT_COMMAND_MENU_SELECTED, MUSIK_PLAYLIST_THREAD_START );
	RetagStartEvt.SetExtraLong(MUSIK_PLAYLIST_RETAG_THREAD);
	wxCommandEvent RetagProgEvt		( wxEVT_COMMAND_MENU_SELECTED, MUSIK_PLAYLIST_THREAD_PROG );	

	wxPostEvent( m_pEvtHandler, RetagStartEvt );

    float fPos = 0;
	int nLastProg = 0;
	int nCurrProg = 0;
//	wxString sMask	= wxGetApp().Prefs.sAutoTag;
	CMusikTagger tagger(m_sTagMask,wxGetApp().Prefs.bAutoTagConvertUnderscoresToSpaces);
//////////////////////////////////////////////////////////////////////////////
    std::auto_ptr<CMusikLibrary> pSlaveLibrary(wxGetApp().Library.CreateSlave());
    pSlaveLibrary->BeginTransaction();
	for ( size_t i = 0; i < m_Songs.GetCount(); i++ )
	{
        if(i % 100 == 99)
        {
            pSlaveLibrary->EndTransaction();
            pSlaveLibrary->BeginTransaction();
        }

		//-----------------------//
		//--- update progress ---//
		//-----------------------//
		fPos = (float)( i * 100 ) / (float)m_Songs.GetCount();
		nCurrProg = (int)fPos;
		if ( nCurrProg > nLastProg )
		{
			RetagProgEvt.SetExtraLong( nCurrProg );
			wxPostEvent( m_pEvtHandler, RetagProgEvt );
		}
		nLastProg = nCurrProg;

		if ( TestDestroy() )
			break;

		pSlaveLibrary->RetagFile(tagger, *m_Songs.Item( i ).Song() );
		Yield();
	}
	pSlaveLibrary->EndTransaction();
	return NULL;
}

void MusikPlaylistRetagThread::OnExit()
{
	wxCommandEvent RetagEndEvt( wxEVT_COMMAND_MENU_SELECTED, MUSIK_PLAYLIST_THREAD_END );
	wxPostEvent( m_pEvtHandler, RetagEndEvt );
}
