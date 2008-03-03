/*
 *  MusikLibraryThreads.cpp
 *
 *  Threads related to MusikLibraryFrame.h / cpp
 *  
 *  Copyright (c) 2003 Casey Langen (casey@bak.rr.com)
 *	Contributors: Simon Windmill, Dustin Carter, Gunnar Roth, Wade Brainerd
 *
 *  See the file "license.txt" for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL WARRANTIES.
*/

//--- For compilers that support precompilation, includes "wx/wx.h". ---//
#include "myprec.h"

#include "MusikLibraryThreads.h"
#include "Classes/Library/MetaDataHandler.h"
//--- globals ---//
#include "MusikGlobals.h"
#include "MusikUtils.h"

//---------------------------------//
//---	  purge old files		---//
//---------------------------------//

void MusikUpdateLibThread::MusikPurgeLibrary( CMusikLibrary *pLibrary,const wxArrayString &songs,bool & bDatabaseChanged )
{
	//--- setup ---//
	bDatabaseChanged = false;

	float fPos;
	int nLastProg = 0;
	int nCurrProg = 0;
	wxString sProgress;
	size_t nTotal = songs.GetCount();

	wxCommandEvent evtSetProgressType	( wxEVT_COMMAND_MENU_SELECTED, MUSIK_LIBRARY_THREAD_PROG );
	evtSetProgressType.SetInt(SET_PROGRESSTYPE);
	evtSetProgressType.SetExtraLong( MUSIK_LIBRARY_PURGE_THREAD );
	wxPostEvent( Parent(), evtSetProgressType );

	wxCommandEvent evtSetTotalFiles	( wxEVT_COMMAND_MENU_SELECTED, MUSIK_LIBRARY_THREAD_PROG );
	evtSetTotalFiles.SetInt(SET_TOTAL);
	evtSetTotalFiles.SetExtraLong( nTotal );
	wxPostEvent( Parent(), evtSetTotalFiles );

	wxCommandEvent PurgeProgEvt		( wxEVT_COMMAND_MENU_SELECTED, MUSIK_LIBRARY_THREAD_PROG );	
	PurgeProgEvt.SetInt(SET_CURRENT);

	pLibrary->BeginTransaction();
	for ( size_t i = 0; i < nTotal; i++ )
	{
		if(i % 100 == 99)
		{
			pLibrary->EndTransaction();
			pLibrary->BeginTransaction();
		}

		if ( TestDestroy() )
			break;
		else 
		{
			//--- post progress event ---//
			fPos = ( i * 100 ) / nTotal;
			nCurrProg = (int)fPos;
			if ( nCurrProg > nLastProg )
			{
				PurgeProgEvt.SetExtraLong( i );
				wxPostEvent( Parent(), PurgeProgEvt );
			}
			nLastProg = nCurrProg;

			//--- check item and purge ---//
			if(pLibrary->FindOrPurgeMissing( songs.Item( i ) ))
				bDatabaseChanged = true;
		}
	}
	pLibrary->EndTransaction();
}
//---------------------------------//
//---		update library		---//
//---------------------------------//
MusikUpdateLibThread::MusikUpdateLibThread(wxEvtHandler *pParent, wxArrayString* del, wxArrayString & m_refFiles, unsigned long flagsUpdate)
	:MusikScanNewThread(pParent,m_refFiles)
{
	m_pPathesToAdd	= g_Paths.GetList();
	m_pPathesDel	= del;
	m_flagsUpdate   = flagsUpdate;
}

void *MusikUpdateLibThread::Entry()
{
	bool bDatabaseChanged = false;
	//--- events we'll post as we go along ---//
    std::auto_ptr<CMusikLibrary> pSlaveLibrary(wxGetApp().Library.CreateSlave());
    pSlaveLibrary->BeginTransaction();
	//--- remove old songs ---//
    pSlaveLibrary->EndTransaction();

	wxArrayString songsBeforeUpdate;
	pSlaveLibrary->Query( wxT("select filename from songs;"), songsBeforeUpdate);

	//--- search / add new songs ---//
	if(m_refFiles.IsEmpty())
	{// m_refFiles is empty ( maybe because scanning was interrupted before) )so we scan for new songs now
		wxCommandEvent ScanNewStartEvt	( wxEVT_COMMAND_MENU_SELECTED, MUSIK_LIBRARY_THREAD_START );
		ScanNewStartEvt.SetExtraLong(MUSIK_LIBRARY_SCANNEW_THREAD);
		wxPostEvent( Parent(), ScanNewStartEvt );
		GetMusicDirs( *m_pPathesToAdd,m_refFiles);
	}
	wxCommandEvent UpdateLibStartEvt	( wxEVT_COMMAND_MENU_SELECTED, MUSIK_LIBRARY_THREAD_START );
	UpdateLibStartEvt.SetExtraLong(MUSIK_LIBRARY_UPDATE_THREAD);
	wxPostEvent( Parent(), UpdateLibStartEvt );

	float fPos;
	int nLastProg = 0;
	int nCurrProg = 0;
	wxString sProgress;
	size_t nTotal = m_refFiles.GetCount();


	wxCommandEvent evtSetTotalFiles	( wxEVT_COMMAND_MENU_SELECTED, MUSIK_LIBRARY_THREAD_PROG );
	evtSetTotalFiles.SetInt(SET_TOTAL);
	evtSetTotalFiles.SetExtraLong( nTotal );
	wxPostEvent( Parent(), evtSetTotalFiles );

	wxCommandEvent UpdateProgEvt		( wxEVT_COMMAND_MENU_SELECTED, MUSIK_LIBRARY_THREAD_PROG );	
	UpdateProgEvt.SetInt(SET_CURRENT);
	UpdateProgEvt.SetExtraLong( 0 );
	pSlaveLibrary->BeginTransaction();
	for ( size_t i = 0; i < nTotal; i++ )
	{
        if(i % 50 == 49)
        {
            pSlaveLibrary->EndTransaction();
            pSlaveLibrary->BeginTransaction();
        }
		if ( TestDestroy() )
			break;

		else
		{
			//--- figure out progress, then post the event ---//
			fPos = ( i * 100 ) / nTotal;
			nCurrProg = (int)fPos;
			if ( nCurrProg > nLastProg )
			{
				UpdateProgEvt.SetExtraLong(i);
				wxPostEvent( Parent(), UpdateProgEvt );
			}
			nLastProg = nCurrProg;
			//--- add the item ---//
			if (!pSlaveLibrary->FileInLibrary( m_refFiles.Item( i ), true ) )
			{
				if(pSlaveLibrary->AddSongDataFromFile( m_refFiles.Item( i ) ))
				{
					bDatabaseChanged = true;
				}
				
				
			}
			else if(m_flagsUpdate & (MUSIK_UpdateFlags::RebuildTags|MUSIK_UpdateFlags::RebuildTagsForce))
			{
				if(pSlaveLibrary->UpdateSongDataFromFile( m_refFiles.Item( i ),0 != (m_flagsUpdate & MUSIK_UpdateFlags::RebuildTagsForce )))
				{
					bDatabaseChanged = true;
				}
			}


		}

	}
	pSlaveLibrary->EndTransaction();
    bool bDbChangedByPurge = false;
	if(0 == (m_flagsUpdate & MUSIK_UpdateFlags::NoPurge))
	{
		if ( !TestDestroy() )
		{
    		MusikPurgeLibrary(pSlaveLibrary.get(),songsBeforeUpdate,bDbChangedByPurge);

			if (m_pPathesDel &&  m_pPathesDel->GetCount() > 0 )
			{
				for ( size_t i = 0; i < m_pPathesDel->GetCount(); i++ )
				{
					if ( TestDestroy() )
						break;
					if ( m_pPathesDel->Item( i ) != wxT("") )
						pSlaveLibrary->RemoveSongDir( m_pPathesDel->Item( i ) );
				}
				m_pPathesDel->Clear();
			}
			// hack: set progresstypoe back to MUSIK_LIBRARY_UPDATE_THREAD (in case MusikPurgeLibrary() was called)
			wxCommandEvent evtSetProgressType	( wxEVT_COMMAND_MENU_SELECTED, MUSIK_LIBRARY_THREAD_PROG );
			evtSetProgressType.SetInt(SET_PROGRESSTYPE);
			evtSetProgressType.SetExtraLong( MUSIK_LIBRARY_UPDATE_THREAD);
			wxPostEvent( Parent(), evtSetProgressType );

		}
	}

	wxCommandEvent UpdateLibEndEvt	( wxEVT_COMMAND_MENU_SELECTED, MUSIK_LIBRARY_THREAD_END );	
	UpdateLibEndEvt.SetExtraLong(bDatabaseChanged || bDbChangedByPurge ? 1:0);
	wxPostEvent( Parent(), UpdateLibEndEvt );
	return NULL;
}

void MusikUpdateLibThread::OnExit()
{

}
															
//---------------------------------//
//---	scan for new files		---//
//---------------------------------//
MusikScanNewThread::MusikScanNewThread(wxEvtHandler *pParent,wxArrayString & refFiles)
        :wxThread(wxTHREAD_JOINABLE)
		,m_refFiles(refFiles)
		,m_pParent(pParent)
{
}

void *MusikScanNewThread::Entry()
{

	//--- events we'll post as we go along ---//
	wxCommandEvent ScanNewStartEvt	( wxEVT_COMMAND_MENU_SELECTED, MUSIK_LIBRARY_THREAD_START );
	ScanNewStartEvt.SetExtraLong(MUSIK_LIBRARY_SCANNEW_THREAD);
	wxPostEvent( Parent(), ScanNewStartEvt );

	GetMusicDirs(*g_Paths.GetList(),m_refFiles);
	wxCommandEvent ScanNewEndEvt( wxEVT_COMMAND_MENU_SELECTED, MUSIK_LIBRARY_THREAD_END );	
	wxPostEvent( Parent(), ScanNewEndEvt );

	return NULL;
}

void MusikScanNewThread::OnExit()
{
}

class wxMusicTraverser : public wxDirTraverser
{
public:

	wxMusicTraverser(MusikScanNewThread * pThread,wxArrayString& files ) 
		: m_files(files) 
		,m_pThread(pThread)
	{ }

    virtual wxDirTraverseResult OnFile(const wxString& filename)
    {
		if(m_pThread->TestDestroy())
		{
			m_files.Clear();
			return wxDIR_STOP;
		}
		wxFileName fn( filename );
		const tSongClass *psc = CMetaDataHandler::GetSongClass(fn.GetExt());
		if (psc) 
		{
			m_files.Add( filename );
			if ( m_files.GetCount() % 100 == 0 )
			{

				wxCommandEvent UpdateScanProg( wxEVT_COMMAND_MENU_SELECTED, MUSIK_LIBRARY_THREAD_SCAN_PROG );
				UpdateScanProg.SetExtraLong((long)m_files.GetCount());
				wxPostEvent( m_pThread->Parent(), UpdateScanProg );
			}
		}
        
        return wxDIR_CONTINUE;
    }

    virtual wxDirTraverseResult OnDir(const wxString& WXUNUSED(dirname))
    {
 		if(m_pThread->TestDestroy())
		{
			m_files.Clear();
			return wxDIR_STOP;
		}
       return wxDIR_CONTINUE;
    }

private:
	wxArrayString& m_files;
	MusikScanNewThread *m_pThread;
};

void MusikScanNewThread::GetMusicDirs( const wxArrayString & aDirs, wxArrayString & aFiles )
{
	aFiles.Clear();
    std::auto_ptr<CMusikLibrary> pSlaveLibrary(wxGetApp().Library.CreateSlave());

	if ( aDirs.GetCount() > 0 )
	{

		wxString sCurrPath;
		wxCommandEvent evtSetTotalFiles	( wxEVT_COMMAND_MENU_SELECTED, MUSIK_LIBRARY_THREAD_PROG );
		evtSetTotalFiles.SetInt(SET_TOTAL);
		wxCommandEvent ScanNewProgEvt	( wxEVT_COMMAND_MENU_SELECTED, MUSIK_LIBRARY_THREAD_PROG );	
		ScanNewProgEvt.SetInt(SET_CURRENT);
		wxCommandEvent evtSetNewFiles	( wxEVT_COMMAND_MENU_SELECTED, MUSIK_LIBRARY_THREAD_PROG );
		evtSetNewFiles.SetInt(SET_NEW);


		for ( size_t i = 0; i < aDirs.GetCount(); i++ )
		{
			if ( TestDestroy() )
				break;
			else 
			{
					//--- get directory ---//
				int nLastCount = aFiles.GetCount();
				GetMusicDir(  aDirs[i], aFiles );

				//--- do math ---//
				int nTotal		= aFiles.GetCount() - nLastCount;
				evtSetTotalFiles.SetExtraLong( nTotal );
				wxPostEvent( Parent(), evtSetTotalFiles );

				int nCompare	= pSlaveLibrary->GetSongDirCount( sCurrPath );
				int nResult		= nTotal - nCompare;

				//--- post update progress event ---//
				evtSetNewFiles.SetExtraLong( nResult );
				wxPostEvent( Parent(), evtSetNewFiles );
				ScanNewProgEvt.SetExtraLong(i);
				wxPostEvent( Parent(), ScanNewProgEvt );
			}
		}
	}
}

void MusikScanNewThread::GetMusicDir( const wxString & sDir, wxArrayString & aFiles )
{
	wxString sPath = sDir;
	sPath.Replace( wxT( " " ), wxT( "" ), true );
	if ( !sPath.IsEmpty() )
	{
		wxDir dir( sDir );
		if ( dir.IsOpened() )
		{
			wxMusicTraverser traverser(this, aFiles );
			dir.Traverse( traverser,wxEmptyString,wxDIR_FILES | wxDIR_DIRS );

			//-----------------------------------------//
			//--- the traverser will post update	---//
			//--- scan new events					---//
			//-----------------------------------------//
		}
	}
}



