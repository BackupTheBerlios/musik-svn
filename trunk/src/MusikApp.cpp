/*
 *  MusikApp.cpp
 *
 *  Application start point
 *  
 *  Copyright (c) 2003 Casey Langen (casey@bak.rr.com)
 *	Contributors: Simon Windmill, Dustin Carter, Gunnar Roth, Wade Brainerd
 *
 *  See the file "license.txt" for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL WARRANTIES.
*/

//--- For compilers that support precompilation, includes "wx/wx.h". ---//
#include <wx/wxprec.h>
#include <wx/textfile.h>
#include <wx/cmdline.h>
#include <wx/progdlg.h>
#include "MusikApp.h"
/***** the XInitThreads call leads to strange behaviour when doing 
****** drag in a listview. the whole xserver is locked, until the app is 
****** killed in a console. mouse is still movable and windows are updated 
****** but no key or mous input is possible.
#if defined(__WXGTK__) || defined(__WXMOTIF__) || defined(__WXX11__)
IMPLEMENT_APP_NO_MAIN(MusikApp)
#include <X11/Xlib.h>
int main(int argc, char *argv[]) {
	if ( XInitThreads() == 0 ) {
		fprintf( stderr, "%s: Unable to initialize multithreaded X11 code (XInitThreads failed).\n", argv[0] );
		exit( EXIT_FAILURE );
	}
	return wxEntry(argc, argv);
}
#else
IMPLEMENT_APP(MusikApp)
#endif
*/

IMPLEMENT_APP(MusikApp)

//--- related frames ---//
#include "Frames/MusikFrame.h"


//--- globals ---//
#include "MusikGlobals.h"
#include "MusikUtils.h"
#define MUSIK_APP_SERVICE wxT("/tmp/wxMusikApp.server")

class MusikAppConnection: public wxConnection
{
public:
	bool OnPoke(const wxString& topic, const wxString& item, wxChar *data, int size, wxIPCFormat format);
	bool OnDisconnect();
};

class MusikAppClient: public wxClient
{
public:
	wxConnectionBase *OnMakeConnection()
	{
		return new MusikAppConnection;
	}
};


bool MusikAppConnection::OnPoke(const wxString& WXUNUSED(topic), const wxString& item, wxChar *data, int size, wxIPCFormat format)

{
#ifdef wxUSE_UNICODE
	if((item == wxT("PlayFiles")) && (format == wxIPC_UNICODETEXT) )
#else
	if((item == wxT("PlayFiles")) && (format == wxIPC_TEXT) )
#endif
	{
		wxString sData(data,size/sizeof(wxChar));
		wxArrayString aFilelist;
		DelimitStr(sData,wxT("\n"),aFilelist);
		wxGetApp().OnPlayFiles(aFilelist);
	}
	else if(item == wxT("RaiseFrame"))
	{
		g_MusikFrame->Show();
		g_MusikFrame->Raise();
	}
	return TRUE;
}
bool MusikAppConnection::OnDisconnect()
{
	return wxConnection::OnDisconnect();
}
void MusikApp::OnPlayFiles(const wxArrayString &aFilelist)
{
	static bool bRecursiveEntry= false;
	if(bRecursiveEntry)
		return;
	bRecursiveEntry = true;
	if(g_MusikFrame)
	{
		g_MusikFrame->AutoUpdate(aFilelist,MUSIK_UpdateFlags::InsertFilesIntoPlayer|MUSIK_UpdateFlags::PlayFiles);
	}
	bRecursiveEntry = false;
}
bool MusikApp::OnInit()
{
	m_locale.AddCatalogLookupPathPrefix(wxT("locale"));

	const wxLanguageInfo * pLangInfo = wxLocale::FindLanguageInfo(Prefs.sLocale);
	if(pLangInfo == NULL)
		m_locale.Init(wxLANGUAGE_DEFAULT);
	else
		m_locale.Init(pLangInfo->Language);
	m_locale.AddCatalog(MUSIKAPPNAME);

	static const wxCmdLineEntryDesc cmdLineDesc[] =
	{
		{ wxCMD_LINE_PARAM,  NULL, NULL, wxT("mp3/ogg file"), wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_MULTIPLE | wxCMD_LINE_PARAM_OPTIONAL},
		{ wxCMD_LINE_NONE }
	};

	wxCmdLineParser parser(argc, argv);
	parser.SetDesc(cmdLineDesc);

	parser.Parse(false);

	wxArrayString arrParams;
	for (size_t i = 0; i <parser.GetParamCount(); i++)
	{
		arrParams.Add(parser.GetParam(i));
	}
	m_pSingleInstanceChecker = new wxSingleInstanceChecker(GetAppName());
	if ( m_pSingleInstanceChecker->IsAnotherRunning() )
	{
		MusikAppClient client;
		MusikAppConnection *pConn = (MusikAppConnection *)client.MakeConnection(wxT("localhost"),MUSIK_APP_SERVICE,wxT("wxMusikInternal"));
		wxString sData;

		if(arrParams.GetCount())
		{
			for( size_t i = 0; i < arrParams.GetCount(); i++)
			{
				sData += arrParams[i];
				sData += wxT("\n");
			}
	#ifdef wxUSE_UNICODE
			pConn->Poke(wxT("PlayFiles"),sData.GetWriteBuf(sData.Length()),sData.Length()*sizeof(wxChar),wxIPC_UNICODETEXT);
	#else
			pConn->Poke(wxT("PlayFiles"),sData.GetWriteBuf(sData.Length()),sData.Length(),wxIPC_TEXT);
	#endif
		}
		pConn->Poke(wxT("RaiseFrame"),NULL,0,wxIPC_PRIVATE);
		return false;
	}

	wxImage::AddHandler(new wxPNGHandler);
	wxImage::AddHandler(new wxJPEGHandler);
	wxImage::AddHandler( new wxXPMHandler );
	wxImage::AddHandler( new wxBMPHandler );
	wxImage::AddHandler( new wxGIFHandler );



	//--- setup our home dir ---//
	if ( !wxDirExists( MUSIK_HOME_DIR ) )
		wxMkdir( MUSIK_HOME_DIR );

	//-----------------------------------------//
	//--- check to see if a new version has	---//
	//--- been installed. if it has, see	---//
	//--- if any core changes need to be	---//
	//--- made.								---//
	//-----------------------------------------//
	CheckOldVersion();
	
	//--- assure playlists directory exists ---//
	if ( !wxDirExists( MUSIK_PLAYLIST_DIR ) )
		wxMkdir( MUSIK_PLAYLIST_DIR );

	//--- load library and paths ---//
	if(!wxGetApp().Library.Load())
	{
		wxMessageBox( _("Initialization of library failed."), MUSIKAPPNAME_VERSION, wxOK | wxICON_ERROR );
		return FALSE;
	}
	g_Paths.Load();
	Player.Init(arrParams.GetCount() > 0);
	//--- initialize fmod ---//
	if ( wxGetApp().Player.InitializeFMOD( FMOD_INIT_START ) != FMOD_INIT_SUCCESS )
		wxMessageBox( _("Initialization of FMOD sound system failed."), MUSIKAPPNAME_VERSION, wxOK | wxICON_ERROR );


	//-------------------//
	//--- main window ---//
	//-------------------//
    MusikFrame *pMain = new MusikFrame();

	//--- restore placement or use defaults ---//
	if ( !SetFramePlacement( pMain, wxGetApp().Prefs.sFramePlacement ) )
	{
		wxSize Size( 
			wxSystemSettings::GetMetric( wxSYS_SCREEN_X ) * 75 / 100, 
			wxSystemSettings::GetMetric( wxSYS_SCREEN_Y ) * 75 / 100 );
		pMain->SetSize( Size );
		pMain->Center();
	}
	pMain->SetTitle( MUSIKAPPNAME_VERSION );
	pMain->SetSongInfoText(MUSIKAPPNAME);
	new MusikLogWindow(pMain,wxString::Format(_("%s Logging Window"),MUSIKAPPNAME),MUSIK_LW_ClearContentOnClose|MUSIK_LW_ShowOnLog); 
	
	wxLog::SetVerbose(Prefs.bLogVerbose);

	SetTopWindow( pMain );

	//--- start webserver if necessary ---//
	if ( Prefs.bWebServerEnable )
		WebServer.Start();

	//--- autostart stuff ---//
	if ( Prefs.bFirstRun )
	{
		wxCommandEvent dummy_ev;
		pMain->OnSetupPaths(dummy_ev);
	}
	else if (Prefs.bAutoAdd || arrParams.GetCount() > 0)
	{	if(Prefs.bAutoAdd)
			pMain->AutoUpdate();
		if(arrParams.GetCount() > 0)
			pMain->AutoUpdate(arrParams,MUSIK_UpdateFlags::InsertFilesIntoPlayer|MUSIK_UpdateFlags::PlayFiles);
	}
	else
	{	if (Prefs.bShowLibraryOnStart)
			g_SourcesCtrl->SelectLibrary();
		else
			g_SourcesCtrl->SelectNowPlaying();
		g_PlaylistBox->Update();
	}
	pMain->Show();
	//--- startup the crossfader			---//
	g_FaderThread = new MusikFaderThread();
	g_FaderThread->Create();
	g_FaderThread->Run();

	m_pServer = new MusikAppServer;
	if(m_pServer)
		m_pServer->Create(MUSIK_APP_SERVICE);
	return TRUE;
}
int MusikApp::OnExit()
{
	delete m_pServer;
	return 0;
}
void MusikApp::CheckOldVersion()
{
	wxString sVersion;
	if ( wxFileExists( MUSIK_VERSION_FILENAME ) )
	{
		sVersion = ReadOldVersionFile();

		wxRemoveFile(MUSIK_VERSION_FILENAME);
		wxRename(MUSIK_OLD_DB_FILENAME,MUSIK_DB_FILENAME);		
	}
	else
	{
		//-------------------------------------------------//
		//--- if these files exist, but the version.dat	---//
		//--- does not, some version prior to 0.1.3 was	---//
		//--- installed. give user a nice little		---//
		//--- warning.									---//
		//-------------------------------------------------//
		if ( wxFileExists( MUSIK_SOURCES_FILENAME ) && wxFileExists( MUSIK_OLD_DB_FILENAME ) )
		{
			wxMessageBox(wxString(MUSIKAPPNAME) + _(" has detected version 0.1.2 or earlier was previously installed.\n\nDue to the changes from 0.1.2 to the current version, your Sources list and Library must be reset. We apologize for any inconvenience this may cause.") , MUSIKAPPNAME_VERSION, wxICON_INFORMATION );
	
			if ( wxFileExists( MUSIK_SOURCES_FILENAME ) )
				wxRemoveFile( MUSIK_SOURCES_FILENAME );
			if ( wxFileExists( MUSIK_OLD_DB_FILENAME ) )
				wxRemoveFile( MUSIK_OLD_DB_FILENAME );
		}	

	}
}

wxString MusikApp::ReadOldVersionFile()
{
	wxTextFile ver( MUSIK_VERSION_FILENAME );
	wxString sRet;
	if ( ver.Open() )
	{
		if ( ver.GetLineCount() != 0 )
			sRet = ver.GetLine( 0 );
		ver.Close();
	}
	return sRet;
}


wxConnectionBase * MusikAppServer::OnAcceptConnection(const wxString& WXUNUSED(topic))
{

	return new MusikAppConnection;
}



/*
What could be improved here:
-Option to prepend a numerical value to the destination filename to maintain 
the same order as the playlist
-Option to create a directory in the destination directory based on playlist name
-A progress dialog
SiW
*/
void MusikApp::CopyFiles(const CMusikSongArray &songs)
{
	//--------------------------------//
	//--- first choose a directory ---//
	//--------------------------------//
	wxFileName destdir;
	wxDirDialog dirdlg( g_MusikFrame, _("Please choose location to copy songs to:"), wxT(""), wxDD_NEW_DIR_BUTTON );
	if ( dirdlg.ShowModal() == wxID_OK )
		destdir.AssignDir(dirdlg.GetPath());
	else
		return;
	wxLongLong llFree;
	wxGetDiskSpace(destdir.GetFullPath(),NULL,&llFree);
	wxLongLong llNeeded =  GetTotalFilesize(songs);
	if(llFree  < llNeeded)
	{
		wxLongLong_t  ToLessBytes = llNeeded.GetValue() - llFree.GetValue();
		wxString  sToLessBytes = wxString::Format(wxT("%")wxLongLongFmtSpec wxT("d"), ToLessBytes);
		// not enough free space
		wxString errmsg = wxString::Format(_("There is not enough free space in directory \"%s\". You need %s bytes more free. Continue nevertheless?"),(const wxChar *)destdir.GetFullPath(),(const wxChar *)sToLessBytes);
		if(wxMessageBox(errmsg,	_("File copy warning"),wxYES|wxNO|wxCENTER|wxICON_EXCLAMATION ) == wxNO)
		{
			return;
		}
	}


	//-----------------------------------------------------//
	//--- now just loop through the files and copy them ---//
	//-----------------------------------------------------//

	wxProgressDialog dialog(_T("Copy files dialog"),
		_T("An informative message"),
		100,    // range
		g_MusikFrame,   // parent
		wxPD_CAN_ABORT |
		wxPD_APP_MODAL |
		// wxPD_AUTO_HIDE | -- try this as well
		wxPD_ELAPSED_TIME |
		wxPD_ESTIMATED_TIME |
		wxPD_REMAINING_TIME);

	wxLongLong llRemaining = llNeeded;

	for ( size_t n = 0; n < songs.GetCount(); n++ )
	{
		const wxFileName & sourcename = songs[n].MetaData.Filename;
		wxFileName destname( sourcename );
		destname.SetPath(destdir.GetPath(0));   // GetPath(0) because the default is GetPath(int flags = wxPATH_GET_VOLUME,
		destname.SetVolume(destdir.GetVolume());	  // i do it this complicated way, because wxFileName::SetPath() is buggy, as it does not handle the volume of path
		wxLongLong llPercent = ((llNeeded - llRemaining) * wxLL(100) /llNeeded );
		
		if(!dialog.Update(llPercent.ToLong(),wxString::Format(_("copying %s"),(const wxChar *)sourcename.GetFullPath())))
		{
			break;
		}
		if(!wxCopyFile( sourcename.GetFullPath(), destname.GetFullPath()))
		{

			wxString errmsg = wxString::Format(_("Failed to copy file %s. Continue?"),(const wxChar *)sourcename.GetFullPath());
			if(wxMessageBox(errmsg,	_("File copy error"),wxYES|wxNO|wxCENTER|wxICON_ERROR ) == wxNO)
				break;
		}
		llRemaining -= songs[n].MetaData.nFilesize;
	}
	dialog.Update(99,wxT(""));	// this is needed to make the gauge fill the whole area.
	dialog.Update(100,wxT(""));

}
