/*
 *  MusikTagThreads.cpp
 *
 *  Threads related to MusikTagFrame.h / cpp
 *  
 *  Copyright (c) 2003 Casey Langen (casey@bak.rr.com)
 *	Contributors: Simon Windmill, Dustin Carter, Gunnar Roth, Wade Brainerd
 *
 *  See the file "license.txt" for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL WARRANTIES.
*/

//--- For compilers that support precompilation, includes "wx/wx.h". ---//
#include "wx/wxprec.h"

#include "MusikTagThreads.h"

//--- globals ---//
#include "../MusikGlobals.h"
#include "../MusikUtils.h"

//-------------------------//
//---	 apply thread	---//
//-------------------------//
MusikTagApplyThread::MusikTagApplyThread(wxEvtHandler *dest ,const CMusikSongArray & songs)
        : wxThread(wxTHREAD_JOINABLE),m_Songs(songs),m_pPostDest(dest)
{
}

void *MusikTagApplyThread::Entry()
{
	//----------------------------------------//
	//--- events we'll post as we go along ---//
	//----------------------------------------//
	wxCommandEvent TagStartEvt		( wxEVT_COMMAND_MENU_SELECTED, MUSIK_TAG_THREAD_START );
	wxCommandEvent TagProgEvt		( wxEVT_COMMAND_MENU_SELECTED, MUSIK_TAG_THREAD_PROG );	

	wxPostEvent( m_pPostDest, TagStartEvt );

	float fPos = 0;
	int nLastProg = 0;
	int nCurrProg = 0;
	bool bRenameOK;
	wxGetApp().Library.BeginTransaction();
	for( size_t i = 0; i < m_Songs.GetCount(); i++ )
	{
		//-----------------------//
		//--- update progress ---//
		//-----------------------//
		fPos = (float)( i * 100 ) / (float)m_Songs.GetCount();
		nCurrProg = (int)fPos;
		if ( nCurrProg > nLastProg )
		{
			TagProgEvt.SetExtraLong( nCurrProg );
			wxPostEvent( m_pPostDest, TagProgEvt );
		}
		nLastProg = nCurrProg;

		if ( TestDestroy() )
			break;
		else
		{
			if ( m_Songs.Item( i ).Check1 == 1 )
			{
				//-----------------------//
				//--- rename the file ---//
				//-----------------------//
				bRenameOK = true;
				if ( wxGetApp().Prefs.bTagDlgRename == 1 )
				{
					bRenameOK = wxGetApp().Library.RenameFile( m_Songs.Item( i ));
					if(false == bRenameOK)
						::wxLogWarning(_("Renaming of file %s failed."),(const wxChar *)m_Songs.Item( i ).MetaData.Filename.GetFullPath());

					m_Songs.Item( i ).Check1 = 0;
				}

				//--------------------------//
				//--- write tags to file ---//
				//--------------------------//
				if ( bRenameOK )
				{
					if ( wxGetApp().Prefs.bTagDlgWrite == 1 )
					{
						//-----------------------------------------//
						//--- rename will update the lib, so if	---//
						//--- we're not renaming, update db too	---//
						//-----------------------------------------//
						wxGetApp().Library.WriteTag( m_Songs.Item( i ),wxGetApp().Prefs.bTagDlgClear ,wxGetApp().Prefs.bTagDlgRename == false );
					}

					//-----------------------------//
					//--- write tag for db only ---//
					//-----------------------------//
					if ( wxGetApp().Prefs.bTagDlgWrite == false && wxGetApp().Prefs.bTagDlgRename == false )
					{
						wxGetApp().Library.UpdateItem( m_Songs.Item( i ), true );
					}
				}
			}
		}
	}
	wxGetApp().Library.EndTransaction();
	return NULL;
}

void MusikTagApplyThread::OnExit()
{
	wxCommandEvent TagEndEvt( wxEVT_COMMAND_MENU_SELECTED, MUSIK_TAG_THREAD_END );	
	wxPostEvent( m_pPostDest, TagEndEvt );
}

