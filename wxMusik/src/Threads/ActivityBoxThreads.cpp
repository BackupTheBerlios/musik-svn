/*
 *  ActivityBoxThreads.cpp
 *
 *  Threads related to ActivityBox.h / cpp
 *  
 *  Copyright (c) 2003 Casey Langen (casey@bak.rr.com)
 *	Contributors: Simon Windmill, Dustin Carter, Gunnar Roth, Wade Brainerd
 *
 *  See the file "license.txt" for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL WARRANTIES.
*/

//--- For compilers that support precompilation, includes "wx/wx.h". ---//
#include "wx/wxprec.h"

#include "ActivityBoxThreads.h"

//--- globals ---//
#include "../MusikGlobals.h"
#include "../MusikUtils.h"

#include "../Frames/MusikFrame.h"

MusikActivityRenameThread::MusikActivityRenameThread( CActivityBox* parent_box, int mode, wxString newvalue )
	:wxThread(wxTHREAD_JOINABLE)
{
	m_ParentBox	= parent_box;
	m_Mode		= mode;
	m_Type		= m_ParentBox->GetActivityType();
	m_TypeStr	= m_ParentBox->GetActivityTypeStr();
	m_Replace	= ConvToUTF8(newvalue);
}

void* MusikActivityRenameThread::Entry()
{
	//--- setup thread to begin in g_MusikFrame ---//
	g_MusikFrame->SetActiveThread	( this );
	g_MusikFrame->SetProgressType	( MUSIK_ACTIVITY_RENAME_THREAD );
	g_MusikFrame->SetProgress		( 0 );
	//----------------------------------------//
	//--- events we'll post as we go along ---//
	//----------------------------------------//
	wxCommandEvent RenameStartEvt	( wxEVT_COMMAND_MENU_SELECTED, MUSIK_ACTIVITY_RENAME_THREAD_START );
	wxCommandEvent RenameProgEvt	( wxEVT_COMMAND_MENU_SELECTED, MUSIK_ACTIVITY_RENAME_THREAD_PROG );	

	wxPostEvent( m_ParentBox, RenameStartEvt );

	if ( m_Mode == ACTIVITY_RENAME_ACTIVITY )
		m_ParentBox->GetSelectedSongs( m_Songs );
	else
		g_PlaylistBox->PlaylistCtrl().GetSelectedSongs( m_Songs );

	float fPos = 0;
	int nLastProg = 0;
	int nCurrProg = 0;

	for ( size_t i = 0; i < m_Songs.GetCount(); i++ )
	{
		//-----------------------//
		//--- update progress ---//
		//-----------------------//
		fPos = (float)( i * 100 ) / (float)m_Songs.GetCount();
		nCurrProg = (int)fPos;
		if ( nCurrProg > nLastProg )
		{
			m_ParentBox->SetProgress( nCurrProg );
			wxPostEvent( m_ParentBox, RenameProgEvt );
		}
		nLastProg = nCurrProg;

		if ( TestDestroy() )
			break;

		else
		{
			//-------------------------------//
			//--- update the current item ---//
			//-------------------------------//
			switch( m_Type )
			{
			case MUSIK_LBTYPE_ARTISTS:
				m_Songs.Item( i ).MetaData.Artist = m_Replace;
				break;
			case MUSIK_LBTYPE_ALBUMS:
				m_Songs.Item( i ).MetaData.Album	= m_Replace;
				break;
			case MUSIK_LBTYPE_GENRES:
				m_Songs.Item( i ).MetaData.Genre = m_Replace;
				break;
			case MUSIK_LBTYPE_YEARS:
				m_Songs.Item( i ).MetaData.Year = m_Replace;
				break;
			}

			//--------------------------//
			//--- write tags to file ---//
			//--------------------------//
			if ( wxGetApp().Prefs.bActBoxWrite == 1 )
				wxGetApp().Library.WriteTag( m_Songs.Item( i ), (bool)wxGetApp().Prefs.bActBoxClear );

			//-------------------//
			//--- rename file ---//
			//-------------------//
			if ( wxGetApp().Prefs.bActBoxRename == 1 )
				if(false == wxGetApp().Library.RenameFile( m_Songs.Item( i ) ))
					::wxLogWarning(_("Renaming of file %s failed."),(const wxChar *)m_Songs.Item( i ).MetaData.Filename.GetFullPath());

			//----------------------------------//
			//--- if not writing, flag dirty ---//
			//----------------------------------//
			wxGetApp().Library.UpdateItem(m_Songs.Item( i ), wxGetApp().Prefs.bActBoxWrite == 0 );

		}
	}

	g_Playlist = m_Songs;
	return NULL;
}

void MusikActivityRenameThread::OnExit()
{
	wxCommandEvent RenameEndEvt( wxEVT_COMMAND_MENU_SELECTED, MUSIK_ACTIVITY_RENAME_THREAD_END );
	wxPostEvent( m_ParentBox, RenameEndEvt );
}

