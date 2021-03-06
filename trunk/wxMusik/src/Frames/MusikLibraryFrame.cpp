/*
 *  MusikLibraryDialog.cpp
 *
 *  Library setup frame
 *  
 *  Copyright (c) 2003 Casey Langen (casey@bak.rr.com)
 *	Contributors: Simon Windmill, Dustin Carter, Gunnar Roth, Wade Brainerd
 *
 *  See the file "license.txt" for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL WARRANTIES.
*/

//--- For compilers that support precompilation, includes "wx/wx.h". ---//
#include "myprec.h"
#include "Classes/PlaylistCtrl.h" //TODO: remove the dependancy
#include "Classes/ActivityAreaCtrl.h" //TODO: remove the dependancy
#include "Classes/SourcesBox.h" //TODO: remove the dependancy
#include "MusikLibraryFrame.h"

//--- globals ---//
#include "MusikGlobals.h"
#include "MusikUtils.h"
#include "Prefs/PrefUtil.h"
//--- frames ---//
#include "MusikFrame.h"

//--- threads ---//
#include "Threads/MusikLibraryThreads.h"

IMPLEMENT_DYNAMIC_CLASS(MusikLibraryDialog,MusikDialog)

BEGIN_EVENT_TABLE(MusikLibraryDialog, MusikDialog)
	EVT_CONTEXT_MENU	( 									MusikLibraryDialog::PathsPopupMenu			)
#ifdef 	__WXMSW__
	EVT_CHAR_HOOK		(									MusikLibraryDialog::TranslateKeys			)
#else	
	EVT_CHAR		(									MusikLibraryDialog::TranslateKeys			)
#endif	
	EVT_MENU			( MUSIK_PATHS_MENU_ADD,				MusikLibraryDialog::OnClickAdd				)
	EVT_MENU			( MUSIK_PATHS_MENU_REMOVESEL,		MusikLibraryDialog::OnClickRemoveSel			)
	EVT_MENU			( MUSIK_PATHS_MENU_REMOVEALL,		MusikLibraryDialog::OnClickRemoveAll			)
	EVT_UPDATE_UI		( MUSIK_PATHS_MENU_REMOVESEL,		MusikLibraryDialog::OnUpdateUIRemoveSel	)
    EVT_BUTTON			( MUSIK_PATHS_MENU_ADD,				MusikLibraryDialog::OnClickAdd				)
    EVT_BUTTON			( MUSIK_PATHS_MENU_REMOVESEL,		MusikLibraryDialog::OnClickRemoveSel			)
    EVT_BUTTON			( MUSIK_PATHS_MENU_REMOVEALL,		MusikLibraryDialog::OnClickRemoveAll			)
	EVT_BUTTON			( MUSIK_PATHS_CLEAR_LIBRARY,	MusikLibraryDialog::OnClickClearLibrary		)
	EVT_BUTTON			( MUSIK_PATHS_REBUILD_LIBRARY,	MusikLibraryDialog::OnRebuildAll				)
	EVT_BUTTON			( MUSIK_PATHS_UPDATE_LIBRARY,	MusikLibraryDialog::OnUpdateAll				)
	EVT_BUTTON			( wxID_OK,							MusikLibraryDialog::OnClickOK				)
	EVT_BUTTON			( wxID_CANCEL,						MusikLibraryDialog::OnClickCancel			)
	EVT_LIST_KEY_DOWN	( MUSIK_PATHS_LIST,					MusikLibraryDialog::OnListKeyDown				)
	EVT_CLOSE			(									MusikLibraryDialog::OnClose					)
	EVT_SIZE			(									MusikLibraryDialog::OnSize					)

	//---------------------------------------------------------//
	//--- threading events.. we use EVT_MENU because its	---//
	//--- nice and simple, and gets the job done. this may	---//
	//--- become a little prettier later, but it works.		---//
	//---------------------------------------------------------//
    EVT_MENU			( MUSIK_LIBRARY_THREAD_START,		MusikLibraryDialog::OnThreadStart		)
	EVT_MENU			( MUSIK_LIBRARY_THREAD_END,			MusikLibraryDialog::OnThreadEnd			)
	EVT_MENU			( MUSIK_LIBRARY_THREAD_PROG,		MusikLibraryDialog::OnThreadProg			)
	EVT_MENU			( MUSIK_LIBRARY_THREAD_SCAN_PROG,	MusikLibraryDialog::OnThreadScanProg		)
END_EVENT_TABLE()

void MusikLibraryDialog::Init()
{
    m_FirstStart	= false;
    paths_context_menu = NULL;
    m_bRebuild		= false;
    m_flagsUpdate =  0;
    m_Close			= false;
    m_AutoStart		= false;

}
//-----------------------------------//
//---     default constructor     ---//
//---  gets called automatically  ---//
//--- on startup to add new files ---//
//-----------------------------------//
bool MusikLibraryDialog::Create( wxWindow* pParent ,const wxArrayString &arrFilenamesToScan,unsigned long flags)
{
    if(!MusikDialog::Create( pParent, -1, _("Searching for and Adding New Files"), wxDefaultPosition, wxSize( 500, 48 )))
        return false;
	m_arrScannedFiles = arrFilenamesToScan;
	//------------------------------//
	//--- initialize needed vars ---//
	//------------------------------//
	m_flagsUpdate =  flags;
	m_Close			= true;
	m_AutoStart		= true;

	//-----------------------//
	//--- create controls ---//
	//-----------------------//
	CreateControls();
	PathsLoad();

	//------------------------------------------//
	//--- load paths and hide unneeded stuff ---//
	//------------------------------------------//
	vsTopSizer->Show( sizerPaths, false );
	vsTopSizer->Show( hsSysButtons, true );
	vsTopSizer->Show(hsLibraryButtons,false);
    Layout();
//	#ifdef __WXGTK__
	wxSize size = vsTopSizer->GetMinSize();
	SetClientSize( 600, size.GetHeight() );
//	#endif
    return true;
}

//---------------------------------//
//---     main constructor      ---//
//---  gets called from menu    ---//
//--- or at program's first run ---//
//---------------------------------//
bool MusikLibraryDialog::Create( wxWindow* pParent, const wxPoint &pos, const wxSize & ) 
{
    if(!MusikDialog::Create( pParent, -1, wxString(MUSIKAPPNAME) + _(" Library Setup"), pos, wxSize( 500, 300 ), wxCLIP_CHILDREN|wxMINIMIZE_BOX ))
        return false;
	//--------------------//
	//--- Context menu ---//
	//--------------------//
	paths_context_menu = new wxMenu;
    paths_context_menu->Append( MUSIK_PATHS_MENU_ADD, _("&Add Directory") );
	paths_context_menu->Append( MUSIK_PATHS_MENU_REMOVESEL, _("&Remove") );
	paths_context_menu->Append( MUSIK_PATHS_MENU_REMOVEALL, _("Remove All Directories") );
	
	//-----------------------//
	//--- create controls ---//
	//-----------------------//
	CreateControls();
	vsTopSizer->Show( gProgress, false );

	//-----------------------------------//
	//--- load paths, initialize vars ---//
	//-----------------------------------//
	PathsLoad();
	m_FirstStart	= true;
    PathsResize();
	//--------------------//
	//--- center frame ---//
	//--------------------//

    return true;
}

void MusikLibraryDialog::CreateControls()
{
	//--------------//
	//--- colors ---//
	//--------------//
	static wxColour cBtnFace = wxSystemSettings::GetColour( wxSYS_COLOUR_3DFACE );
	this->SetBackgroundColour ( cBtnFace );

	//------------//
	//--- icon ---//
	//------------//
	#if defined (__WXMSW__)
		SetIcon( wxICON( musicbox ) );
	#endif


	//----------------------//
	//--- Paths ListCtrl ---//
	//----------------------//	
	lcPaths  = new wxListCtrl( this, MUSIK_PATHS_LIST, wxPoint( 0, 0 ), wxSize( 0, 0 ), wxLC_REPORT | wxSIMPLE_BORDER );
	lcPaths->InsertColumn( 0, _("Path"), wxLIST_FORMAT_LEFT, -1 );
	lcPaths->InsertColumn( 1, _("Total"), wxLIST_FORMAT_RIGHT, 60 );
	lcPaths->InsertColumn( 2, _("New"), wxLIST_FORMAT_RIGHT, 60 );


	wxButton *btnAddDir  =	new wxButton( this, MUSIK_PATHS_MENU_ADD,_("&Add") );
	wxButton *btnRemDir  =	new wxButton( this, MUSIK_PATHS_MENU_REMOVESEL, _("&Remove") );
	wxButton *btnRemAll  =	new wxButton( this, MUSIK_PATHS_MENU_REMOVEALL, _("Remove All")) ;

	sizerPaths = new wxBoxSizer(wxHORIZONTAL);
	sizerPaths->Add(lcPaths,1,wxEXPAND);
	wxSizer *sizerPathsButtons = new wxBoxSizer(wxVERTICAL);
	
	sizerPathsButtons->Add(btnAddDir,0,wxALL,2);
	sizerPathsButtons->Add(btnRemDir,0,wxALL,2);
	sizerPathsButtons->Add(btnRemAll,0,wxALL,2);

	sizerPaths->Add(sizerPathsButtons);


	wxButton *btnUPDATE_LIBRARY  =	new wxButton( this, MUSIK_PATHS_UPDATE_LIBRARY, _("&Update Library") );
	wxButton *btnREBUILD_LIBRARY =	new wxButton( this, MUSIK_PATHS_REBUILD_LIBRARY, _("&Rebuild Library") );
	PREF_CREATE_CHECKBOX(AllowTagGuessing,_("Allow tag guessing from filename"));
	PREF_CREATE_CHECKBOX(ScanMP3VBRQuick,_("Only quick scan for MP3-VBR files (faster)"));
	wxButton *btnCLEAR_LIBRARY   =	new wxButton( this, MUSIK_PATHS_CLEAR_LIBRARY, _("Clear &Library") );

	hsLibraryButtons = new wxBoxSizer(wxHORIZONTAL);
	hsLibraryButtons->Add(btnUPDATE_LIBRARY,0,wxEXPAND);
	hsLibraryButtons->Add(btnREBUILD_LIBRARY,0,wxEXPAND);
	hsLibraryButtons->Add(btnCLEAR_LIBRARY,0,wxEXPAND);

	//--------------------//
	//--- progress bar ---//
	//--------------------//
	gProgress 	= new wxGauge( this, -1, 100, wxPoint( 0, 0 ), wxSize( 0, 18 ), wxGA_SMOOTH );


	//----------------------//
	//--- system buttons ---//
	//----------------------//
	hsSysButtons = new wxStdDialogButtonSizer;
	hsSysButtons->AddButton( new wxButton( this, wxID_CANCEL,	_("Cancel")) );
	hsSysButtons->AddButton( new wxButton( this, wxID_OK,		_("OK")) );
	hsSysButtons->Realize();

	//-----------------//
	//--- top sizer ---//
	//-----------------//
	vsTopSizer = new wxBoxSizer( wxVERTICAL );
	vsTopSizer->Add( sizerPaths, 1, wxEXPAND | wxALL, 2 );
    vsTopSizer->Add(chkAllowTagGuessing,0,wxALIGN_CENTER_VERTICAL|wxALL,4);
	vsTopSizer->Add(chkScanMP3VBRQuick,0,wxALIGN_CENTER_VERTICAL|wxALL,4);
	vsTopSizer->Add(hsLibraryButtons,0,wxALL,2);
	vsTopSizer->Add( gProgress, 0, wxEXPAND | wxALL, 2 );
	vsTopSizer->Add( hsSysButtons, 0, wxEXPAND | wxALL, 2 );

	SetSizer( vsTopSizer );
}

void MusikLibraryDialog::OnUpdateAll( wxCommandEvent& WXUNUSED(event) )	
{ 
	PathsSave(); 
	UpdateLibrary( false,MUSIK_UpdateFlags::RebuildTags );	
}

void MusikLibraryDialog::OnRebuildAll( wxCommandEvent& WXUNUSED(event) )	
{ 
	PathsSave(); 
	if ( wxMessageBox( _( "This will completely rebuild the database and can be slow. Only use this if the normal update does not work correctly. Do you really want to do this?" ), MUSIKAPPNAME_VERSION, wxYES_NO | wxICON_QUESTION ) == wxYES )
	{
		UpdateLibrary( false ,MUSIK_UpdateFlags::RebuildTagsForce);	
	}
}
bool MusikLibraryDialog::Show( bool show )
{
	bool bRet = MusikDialog::Show( show );

	//---------------------------//
	//--- kill first run pref ---//
	//---------------------------//
	if ( wxGetApp().Prefs.bFirstRun )
	{
		wxString sMessage = 	_( "This is the first time %s has been run.\n\nTo begin, you must first add directories to the database. Select \"Add\" from the upper right corner to add directories\n, then press the \"OK\" button to rebuild the library.\n\nTo display this window again, press CTRL+L in the main window, or select \"Library Setup\" from the \"Library\" menu.");
							
		wxMessageBox( wxString::Format(sMessage,MUSIKAPPNAME), MUSIKAPPNAME_VERSION, wxICON_INFORMATION );
	
		wxGetApp().Prefs.bFirstRun = false;
		PathsListAdd();

	}
	
	//--- auto start ---//
	if ( m_AutoStart )
	{
		m_AutoStart	= false;
		UpdateLibrary( false );
	}

	//--- non-autstart ---//
	else if ( m_FirstStart )
	{
		m_FirstStart = false;
		ScanNew();
	}

	return bRet;
}
//------------------------------------------//
//--- Loading / Saving Paths in ListCtrl ---//
//------------------------------------------//
void MusikLibraryDialog::PathsLoad()
{
	for ( long i = 0; i < (long)g_Paths.GetCount(); i++ )
	{
		if ( !g_Paths.Item( i ).IsEmpty() )
		{
			lcPaths->InsertItem( i, g_Paths.Item( i ) );
			lcPaths->SetItem( i, 1, wxT("-"), -1 );
			lcPaths->SetItem( i, 2, wxT("-"), -1 );
		}
	}
}

void MusikLibraryDialog::PathsSave()
{
	g_Paths.Clear();
	for ( int i = 0; i < lcPaths->GetItemCount(); i++ )
		g_Paths.Add( lcPaths->GetItemText( i ) );

	g_Paths.Save();
}

//---------------------//
//--- Dialog Events ---//
//---------------------//
void MusikLibraryDialog::Close( bool bCancel )
{
	if ( m_ActiveThreadController.IsAlive() )
		m_ActiveThreadController.Cancel();
	//---------------------------------//
	//--- if cancel / ok is pressed ---//
	//---------------------------------//
	if ( !bCancel )
	{
		PathsSave();
		if ( m_bRebuild )
		{
			//-----------------------------------------------------//
			//--- setting m_Close allows the thread to close	---//
			//--- the dialog upon completion.					---//
			//-----------------------------------------------------//
			m_Close = true;
			UpdateLibrary( true );
			
			//-----------------------------------------------------//
			//--- we need to return prematurely, to allow the	---//
			//--- thread to process. it will handle dialog		---//
			//--- clean up										---//
			//-----------------------------------------------------//
			return;
		}
	}

	delete paths_context_menu;
	paths_context_menu = NULL;
	//g_MusikFrame->Enable( TRUE );
	Destroy();
}

void MusikLibraryDialog::PathsResize()
{
	Layout();

	int nW, nH;
	lcPaths->GetClientSize( &nW, &nH );

	int nTotal = 50;
	int nDelta = 50;
	int nDir = nW - 100;
	lcPaths->SetColumnWidth( 0, nDir );
	lcPaths->SetColumnWidth( 1, nTotal );
	lcPaths->SetColumnWidth( 2, nDelta );
}

void MusikLibraryDialog::PathsGetSel( wxArrayString &aReturn )
{
	aReturn.Clear();
	for ( int i = 0; i < lcPaths->GetItemCount(); i++ )
	{
		if ( lcPaths->GetItemState( i, wxLIST_STATE_SELECTED ) > 2 )
			aReturn.Add( lcPaths->GetItemText( i ) );
	}
	return;
}

void MusikLibraryDialog::PathsPopupMenu( wxContextMenuEvent& WXUNUSED(event) )
{
	if(paths_context_menu)
		PopupMenu( paths_context_menu);
}

//-----------------------//
//--- ListCtrl Events ---//
//-----------------------//

void MusikLibraryDialog::OnUpdateUIRemoveSel(wxUpdateUIEvent & event)
{
	int nSel = lcPaths->GetSelectedItemCount();
	event.Enable( nSel > 0 );
}

void MusikLibraryDialog::PathsListRemoveSel()
{
	for ( int i = 0; i < lcPaths->GetItemCount(); i++ )
	{
		if ( lcPaths->GetItemState( i, wxLIST_STATE_SELECTED ) > 2 )	//THIS NEEDS TO BE FIXED?
		{
			lcPaths->DeleteItem( i );
			i--;
			m_arrScannedFiles.Clear(); // we have to rescan the files
		}
	}

	PathsSave();
	m_bRebuild = true;
}

void MusikLibraryDialog::PathsListRemoveAll()
{
	if ( lcPaths->GetItemCount() )
	{
		lcPaths->DeleteAllItems();
		PathsSave();
		m_bRebuild = true;
	}
	m_arrScannedFiles.Clear(); // we have to rescan the files
}

void MusikLibraryDialog::OnListKeyDown( wxListEvent & event )
{
	if ( event.GetKeyCode() == WXK_DELETE || event.GetKeyCode() == WXK_BACK )
		PathsListRemoveSel();
	else if ( event.GetKeyCode() == 65 )
		wxListCtrlSelAll( lcPaths );
	else if ( event.GetKeyCode() == 68 )
		wxListCtrlSelNone( lcPaths );
	else 
		event.Skip();
}

void MusikLibraryDialog::PathsListAdd()
{
	wxDirDialog *dlgBrowse = new wxDirDialog( this, _("Please choose music directory."));
	if ( dlgBrowse->ShowModal() == wxID_OK )
	{
		wxString sPath = dlgBrowse->GetPath();
		if ( ( sPath != wxT("") ) && ( sPath != wxT("\\") ) )
		{	
			#if defined (__WXMSW__)
				if ( sPath.Right( 1 ) != wxT("\\") )
					sPath = sPath + wxT("\\");
			#else
				if ( sPath.Right( 1 ) != wxT("/") )
					sPath = sPath + wxT("/");
			#endif
			if(g_bPortableAppMode)
			{
				wxFileName dir(sPath);
				wxStandardPathsBase & stdpaths = wxStandardPaths::Get();
				dir.MakeRelativeTo(wxFileName(stdpaths.GetExecutablePath()).GetPath());
				sPath = dir.GetPath(true);
			}
			if ( ValidatePath( sPath ) )
			{
				lcPaths->InsertItem( lcPaths->GetItemCount(), sPath );
				lcPaths->SetItem( lcPaths->GetItemCount()-1, 1, wxT("-"), -1 );
				lcPaths->SetItem( lcPaths->GetItemCount()-1, 2, wxT("-"), -1 );

				PathsSave();
				m_bRebuild = true;
				m_arrScannedFiles.Clear(); // we have to rescan the files
			}
		}
	}
	delete dlgBrowse;
}

//------------//
//--- Misc ---//
//------------//
bool MusikLibraryDialog::ValidatePath( wxString sPath )
{
	if ( g_Paths.GetCount() == 0 )
		return true;

	//--- loop through other paths, seeing if there is a conflict ---//
	wxString	sOldPath;
	wxString	sConflicts;
	wxArrayInt	aConflicts;

	#ifdef __WXMSW___
		sPath.MakeLower();
	#endif

	for ( size_t i = 0; i < g_Paths.GetCount(); i++ )
	{
		sOldPath = g_Paths.Item( i );

		#ifdef __WXMSW___
			sOldPath.MakeLower();
		#endif

		//---------------------------------------------------------//
		//--- if the new path equals the old path then return	---//
		//---------------------------------------------------------//
		if ( sPath == sOldPath )
		{
			wxMessageBox( _( "The path entered already exists." ), MUSIKAPPNAME_VERSION, wxICON_INFORMATION );
			return false;
		}

		//---------------------------------------------------------//
		//--- if new path is longer than path we're checking	---//
		//--- against, it will be a child folder. see if they	---//
		//--- have the same root								---//
		//---------------------------------------------------------//
		if ( sPath.Length() > sOldPath.Length() )
		{
			if ( sPath.Find( sOldPath ) > -1 )
			{
				wxMessageBox( _( "The path entered is already contained within the following path's scope:\n\n" ) + g_Paths.Item( i ), MUSIKAPPNAME_VERSION, wxICON_INFORMATION );
				return false;
			}
		}

		//---------------------------------------------------------//
		//--- if the old path is longer than the path we're		---//
		//--- checking against, it may be a path's parent dir	---//
		//---------------------------------------------------------//
		else 
		{
			if ( sOldPath.Find( sPath ) > -1 )
			{
				sConflicts += g_Paths.Item( i ) + wxT( "\n" );
				aConflicts.Add( i );
			}
		}

	}

	//-----------------------------------------------------//
	//--- display conflicts and ask user what to do		---//
	//-----------------------------------------------------//
	if ( sConflicts.Length() > 0 )
	{
		if ( wxMessageBox( _( "The path entered conflicts with the following paths:\n\n" ) + sConflicts + _( "\nDo you want me to fix this conflict for you?" ), MUSIKAPPNAME_VERSION, wxICON_INFORMATION | wxYES_NO ) == wxYES )
		{
			long nCount = (long)g_Paths.GetCount();
		
			for ( long i = 0; i < nCount; i++ )
				lcPaths->DeleteItem( aConflicts.Item( i ) - i );

			return true;
		}
		else 
			return false;
	}
	

	return true;
}

void MusikLibraryDialog::ClearLibrary()
{
	if ( wxMessageBox( _("This will wipe the library clean. Are you ABSOLUTELY SURE you want to do this?"), MUSIKAPPNAME_VERSION, wxYES_NO|wxICON_QUESTION  ) == wxYES )
	{
		wxGetApp().Library.RemoveAll();
		g_ActivityAreaCtrl->ResetAllContents();
		g_thePlaylist.Clear();
		g_PlaylistBox->SetPlaylist(&g_thePlaylist);

		ScanNew();
	}
}

void MusikLibraryDialog::ScanNew()
{
	if ( g_Paths.GetCount() == 0 )
		return;
	m_arrScannedFiles.Clear();
	if ( !m_ActiveThreadController.IsAlive())
	{
		m_ActiveThreadController.AttachAndRun( new MusikScanNewThread(this,m_arrScannedFiles) );
	}
	else
		InternalErrorMessageBox(wxT("Previous thread not terminated correctly."));
}

void MusikLibraryDialog::UpdateLibrary( bool bConfirm ,unsigned long flags)
{
    flags |= m_flagsUpdate; // add default flags
	if ( bConfirm )
	{	
		if ( wxMessageBox(wxString(MUSIKAPPNAME) + _(" has detected that your library configuration has changed.\n\nIt is suggested that you update the internal library, which includes adding the new songs. Proceed?"), MUSIKAPPNAME_VERSION, wxYES_NO|wxICON_QUESTION  ) == wxNO )
			return;
	}
    
	if ( !m_ActiveThreadController.IsAlive())
	{
		m_ActiveThreadController.AttachAndRun( new MusikUpdateLibThread(this,m_arrScannedFiles ,flags) );
		if(flags & MUSIK_UpdateFlags::WaitUntilDone)
			m_ActiveThreadController.Join();
	}
	else
		InternalErrorMessageBox(wxT("Previous thread not terminated correctly."));
}
void MusikLibraryDialog::EnableProgress( bool enable )
{
	vsTopSizer->Show( gProgress, enable );

	wxWindowList & children = GetChildren();
	for ( wxWindowList::Node *node = children.GetFirst(); node; node = node->GetNext() )
	{
		wxWindow *current = (wxWindow *)node->GetData();
		if(enable == true && current->GetId() == wxID_CANCEL)
			continue;
		current->Enable(!enable);
	}	
	Layout();
#ifndef __WXGTK__	
     wxYieldIfNeeded();
#endif    
}

void MusikLibraryDialog::TranslateKeys( wxKeyEvent& event )
{
 	if ( event.GetKeyCode() == WXK_ESCAPE )
	{
		if ( m_ActiveThreadController.IsAlive() )
			m_ActiveThreadController.Cancel();
		else
			Close( true );
	}
	
	event.Skip(); // propagate esc to other frames too
}

//-----------------------------------------------------------//
//--- got a new thread start event, figure out what to do ---//
//-----------------------------------------------------------//
void MusikLibraryDialog::OnThreadStart( wxCommandEvent& event )
{
    SetProgress		( 0 );
    SetProgressType	( event.GetExtraLong() );
	EnableProgress( true );

	m_StopWatch.Start();
}

//-----------------------------------------------------//
//--- got a thread end event, figure out what to do ---//
//-----------------------------------------------------//
void MusikLibraryDialog::OnThreadEnd( wxCommandEvent& event )
{
	m_ActiveThreadController.Join();
	EnableProgress( false );

	if ( GetProgressType() == MUSIK_LIBRARY_SCANNEW_THREAD )
	{
	}

	else if ( GetProgressType() == MUSIK_LIBRARY_UPDATE_THREAD )
	{
		if ( m_Close )
			Close( true );
		
		bool bDatabaseChanged = event.GetExtraLong()?true:false;
		if(bDatabaseChanged)
			g_ActivityAreaCtrl->ReloadAllContents();

		if((m_flagsUpdate & (MUSIK_UpdateFlags::InsertFilesIntoPlayer | MUSIK_UpdateFlags::EnquequeFilesIntoPlayer)) && m_arrScannedFiles.GetCount())
		{
			MusikSongIdArray songs;
			wxGetApp().Library.GetFilelistSongs( m_arrScannedFiles, songs );
			if(	songs.GetCount())
			{
				if(m_flagsUpdate & MUSIK_UpdateFlags::InsertFilesIntoPlayer )
					wxGetApp().Player.InsertToPlaylist(songs,(MUSIK_UpdateFlags::PlayFiles & m_flagsUpdate) == MUSIK_UpdateFlags::PlayFiles);
				else if(m_flagsUpdate & MUSIK_UpdateFlags::EnquequeFilesIntoPlayer )
					wxGetApp().Player.AddToPlaylist(songs,(MUSIK_UpdateFlags::PlayFiles & m_flagsUpdate) == MUSIK_UpdateFlags::PlayFiles);
				g_SourcesCtrl->SelectNowPlaying();
				g_PlaylistBox->Update();
			}
		}
		else if (bDatabaseChanged && wxGetApp().Prefs.bShowAllSongs == 1 && (g_SourcesCtrl->GetSelType() == MUSIK_SOURCES_LIBRARY))
		{
			wxGetApp().Library.GetAllSongs( g_thePlaylist );
			g_PlaylistBox->SetPlaylist(&g_thePlaylist);
		}
		m_bRebuild = false;
	}

	SetProgress	( 0 );
	SetProgressType	( 0 );
	wxTimeSpan ts(0,0,0,m_StopWatch.Time());

	SetTitle(wxString(MUSIKAPPNAME) + _( " Library Setup - Elapsed Time: ") + ts.Format(wxT("%H:%M:%S")) );
}

//---------------------------------------------------------//
//--- got a thread process event, figure out what to do ---//
//---------------------------------------------------------//
void MusikLibraryDialog::OnThreadProg( wxCommandEvent& event )
{	
	
	if( SET_PROGRESSTYPE == event.GetInt())
	{
		SetProgressType	( event.GetExtraLong() );
		return;
	}
	if( SET_TOTAL == event.GetInt())
	{
		SetTotal(  event.GetExtraLong());
		return;
	}
	else if(SET_NEW == event.GetInt())
	{
		SetNew(event.GetExtraLong());
		return;
	}
	if ( GetProgressType() == MUSIK_LIBRARY_SCANNEW_THREAD )
	{
		if(SET_CURRENT == event.GetInt())
		{
			//----------------------------------------------------------//
			//--- MusikLibraryDialog::OnThreadScanProg will set title ---//
			//----------------------------------------------------------//
			lcPaths->SetItem( event.GetExtraLong(), 1, IntTowxString( m_Total ));
			lcPaths->SetItem( event.GetExtraLong(), 2, IntTowxString( m_New ));
			if(g_Paths.GetCount())
                gProgress->SetValue( ( event.GetExtraLong() * 100 ) /  (long)g_Paths.GetCount() );
		}

	}
	else if ( GetProgressType() == MUSIK_LIBRARY_UPDATE_THREAD )
	{
		if(SET_CURRENT == event.GetInt())
		{
			m_Title.sprintf( _( "Scanning for and adding new files: %d / %d (ESC to abort)" ), event.GetExtraLong(), GetTotal() );
			SetTitle( m_Title );
			gProgress->SetValue( ( event.GetExtraLong() * 100 ) / GetTotal() );

		}

	}

	else if ( GetProgressType() == MUSIK_LIBRARY_PURGE_THREAD )
	{
		if(SET_CURRENT == event.GetInt())
		{
			m_Title.sprintf( _( "Scanning library for obsolete files: %d / %d (ESC to abort)" ), event.GetExtraLong(), GetTotal() );
			SetTitle( m_Title );
            size_t total = GetTotal();
			if(total) 
                gProgress->SetValue( ( event.GetExtraLong() * 100 ) / total );

		}
	}
}

//-------------------------------------------------------------------//
//--- got a scan prog event, update to show users x files scanned ---//
//-------------------------------------------------------------------//
void MusikLibraryDialog::OnThreadScanProg( wxCommandEvent& event )
{
	m_Title.sprintf( _( "Scanning directory for audio files: %d files scanned" ), event.GetExtraLong() );
	SetTitle( m_Title );
}



