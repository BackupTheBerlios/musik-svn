/*
 *  MusikPrefsDialog.cpp
 *
 *  Preferences frame
 *  
 *  Copyright (c) 2003 Casey Langen (casey@bak.rr.com)
 *	Contributors: Simon Windmill, Dustin Carter, Gunnar Roth, Wade Brainerd
 *
 *  See the file "license.txt" for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL WARRANTIES.
*/

//--- For compilers that support precompilation, includes "wx/wx.h". ---//
#include "wx/wxprec.h"
#ifdef __WXMSW__
#include <shlobj.h>
#endif
#include "MusikPrefsFrame.h"

//--- globals ---//
#include "../MusikGlobals.h"
#include "../MusikUtils.h"
#include "../PrefUtil.h"
//--- classes ---//
#include "../Classes/MusikPlayer.h"
#include "../Classes/Library/MetaDataHandler.h"
//--- related frames ---//
#include "../Frames/MusikFrame.h"

#include <wx/colordlg.h>
#include <wx/valgen.h>
#include <wx/mimetype.h>

BEGIN_EVENT_TABLE(MusikPrefsDialog, wxDialog)
#ifdef __WXMSW__
	EVT_CHAR_HOOK			(											MusikPrefsDialog::OnTranslateKeys	)
#else
	EVT_CHAR	(											MusikPrefsDialog::OnTranslateKeys	)
#endif
	EVT_TREE_SEL_CHANGED	(MUSIK_PREFERENCES_TREE,					MusikPrefsDialog::OnTreeChange		)
	EVT_BUTTON				(wxID_OK,						MusikPrefsDialog::OnClickOK			)
	EVT_BUTTON				(wxID_APPLY,					MusikPrefsDialog::OnClickApply		)
	EVT_BUTTON				(wxID_CANCEL,					MusikPrefsDialog::OnClickCancel		)
	EVT_BUTTON				(MUSIK_PREFERENCES_PLAYLIST_STRIPE_COLOUR,	MusikPrefsDialog::OnClickColour		)
	EVT_BUTTON				(MUSIK_PREFERENCES_ACTIVITY_STRIPE_COLOUR,	MusikPrefsDialog::OnClickColour		)
	EVT_BUTTON				(MUSIK_PREFERENCES_SOURCES_STRIPE_COLOUR,	MusikPrefsDialog::OnClickColour		)
	EVT_BUTTON				(MUSIK_PREFERENCES_PLAYLIST_BORDER_COLOUR,	MusikPrefsDialog::OnClickColour		)
	EVT_COMBOBOX			(MUSIK_PREFERENCES_OUTPUT_DRV,				MusikPrefsDialog::OnOutputChanged	)
	EVT_CLOSE				(											MusikPrefsDialog::OnClose			)
END_EVENT_TABLE()


MusikPrefsDialog::MusikPrefsDialog( wxWindow *pParent, const wxString &sTitle ) 
	: wxDialog( pParent, -1, sTitle, wxDefaultPosition, wxSize(700,700), wxDEFAULT_FRAME_STYLE|wxRESIZE_BORDER|wxCAPTION | wxTAB_TRAVERSAL | wxFRAME_FLOAT_ON_PARENT | wxFRAME_NO_TASKBAR )
{
	//---------------//
	//--- colours ---//
	//---------------//
	static wxColour cBtnFace = wxSystemSettings::GetColour( wxSYS_COLOUR_3DFACE );
	this->SetBackgroundColour ( cBtnFace );

	//------------//
	//--- icon ---//
	//------------//
	#if defined (__WXMSW__)
		SetIcon( wxICON( musicbox ) );
	#endif


	//------------------------------//
	//--- Selection Tree Control ---//
	//------------------------------//
	tcPreferencesTree	= new wxTreeCtrl( this, MUSIK_PREFERENCES_TREE, wxDefaultPosition, wxSize(-1,-1), wxTR_HAS_BUTTONS | wxSIMPLE_BORDER );
	//--- root nodes ---//
	nRootID			=	tcPreferencesTree->AddRoot		( _( "Preferences" )		);
	nOptionsRootID	=	tcPreferencesTree->AppendItem	( nRootID, _( "Options" )	);
	nTagRootID		=	tcPreferencesTree->AppendItem	( nRootID, _( "Tagging" )	);
	nSoundRootID	=	tcPreferencesTree->AppendItem	( nRootID, _( "Sound" )		);	
	wxTreeItemId nStreamingRootID	=	tcPreferencesTree->AppendItem	( nRootID, _( "Streaming" )	);
	//--- child nodes ---//
	nInterfaceID	=	tcPreferencesTree->AppendItem	( nOptionsRootID,	_( "General" )		);
	nSelectionsID	=	tcPreferencesTree->AppendItem	( nOptionsRootID,	_( "Selections" )	);
	nPlaylistID		=	tcPreferencesTree->AppendItem	( nOptionsRootID,	_( "Playlist" )		);
	nFileAssocsID	=	tcPreferencesTree->AppendItem	( nOptionsRootID,	_( "File Associations"));
	nTunageID		=	tcPreferencesTree->AppendItem	( nOptionsRootID,	_( "Tunage" )		);
	nAutoDjID		= 	tcPreferencesTree->AppendItem	( nOptionsRootID,	wxString(_("Shuffle")) +wxT("/") + _( "Auto DJ" ));
	nGeneralTagID	=	tcPreferencesTree->AppendItem	( nTagRootID,		_( "General" )		);
	nAutoTagID		=	tcPreferencesTree->AppendItem	( nTagRootID,		_( "Auto Rename" )		);
	nDriverID		=	tcPreferencesTree->AppendItem	( nSoundRootID,		_( "Driver" )		);
	nPlaybackID		=	tcPreferencesTree->AppendItem	( nSoundRootID,		_( "Crossfader" )	);
	nStreamingBufferID	=	tcPreferencesTree->AppendItem	( nStreamingRootID, _( "Buffer" )	);
   	nStreamingProxyServerID	=	tcPreferencesTree->AppendItem	( nStreamingRootID, _( "Proxy server" )	);

	//--- expand all the root nodes ---//
	tcPreferencesTree->Expand( nOptionsRootID );
	tcPreferencesTree->Expand( nTagRootID );
	tcPreferencesTree->Expand( nSoundRootID );
 	tcPreferencesTree->Expand( nSoundRootID );
	tcPreferencesTree->Expand( nStreamingRootID );

	//--------------------------//
	//--- Sound -> Crossfader ---//
	//---------------------------//
	PREF_CREATE_CHECKBOX(FadeEnable, _("Crossfade on new song (seconds)"));
	PREF_CREATE_CHECKBOX(FadeSeekEnable, _("Crossfade on track seek (seconds)"));
	PREF_CREATE_CHECKBOX(FadePauseResumeEnable, _("Crossfade on pause or resume (seconds)"));
	PREF_CREATE_CHECKBOX(FadeStopEnable, _("Crossfade on stop (seconds)"));
	PREF_CREATE_CHECKBOX(FadeExitEnable, _("Crossfade on program exit (seconds)"));


	tcDuration 				= new wxTextCtrl_NoFlicker( this, -1);
	tcSeekDuration 			= new wxTextCtrl_NoFlicker( this, -1);
	tcPauseResumeDuration	= new wxTextCtrl_NoFlicker( this, -1);
	tcStopDuration			= new wxTextCtrl_NoFlicker( this, -1);
	tcExitDuration			= new wxTextCtrl_NoFlicker( this, -1);
	//--- crossfader sizer ---//
	wxFlexGridSizer	*fsCrossfader = new wxFlexGridSizer( 5, 2, 2, 2 );
	fsCrossfader->Add( chkFadeEnable				);
	fsCrossfader->Add( tcDuration 				);
	fsCrossfader->Add( chkFadeSeekEnable		);
	fsCrossfader->Add( tcSeekDuration		 	);
	fsCrossfader->Add( chkFadePauseResumeEnable	);
	fsCrossfader->Add( tcPauseResumeDuration	);
	fsCrossfader->Add( chkFadeStopEnable		);
	fsCrossfader->Add( tcStopDuration			);
	fsCrossfader->Add( chkFadeExitEnable		);
	fsCrossfader->Add( tcExitDuration			);

	//---------------------------------//
	//--- Sound -> Crossfader Sizer ---//
	//---------------------------------//
	vsSound_Crossfader = new wxBoxSizer( wxVERTICAL );
	vsSound_Crossfader->Add( fsCrossfader,	0, wxALL, 4  );

	//-----------------------//
	//--- Sound -> Driver ---//
	//-----------------------//
	//--- output driver ---//
	
	cmbOutputDrv = new wxComboBox ( this, MUSIK_PREFERENCES_OUTPUT_DRV, wxT(""), wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_READONLY );
	#if defined (__WXMSW__)
		cmbOutputDrv->Append ( wxT("Direct Sound") );
		cmbOutputDrv->Append ( wxT("Windows Multimedia") );
		cmbOutputDrv->Append ( wxT("ASIO") );
	#elif defined (__WXMAC__)
		//--- mac sound stuff ---//
		cmbOutputDrv->Append ( wxT("MAC") );
	#elif defined (__WXGTK__)
		cmbOutputDrv->Append ( wxT("OSS") );
		cmbOutputDrv->Append ( wxT("ESD") );
		cmbOutputDrv->Append ( wxT("ALSA 0.9") );
	#endif
	//--- sound device ---//
	
	cmbSndDevice	= new wxComboBox ( this, MUSIK_PREFERENCES_SND_DEVICE, wxT(""), wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_READONLY );	
	//--- playrate ---//
	cmbPlayRate  = new wxComboBox( this, -1, wxT(""), wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_READONLY );
	cmbPlayRate->Append ( wxT("48000") );
	cmbPlayRate->Append ( wxT("44100") );
	cmbPlayRate->Append ( wxT("22050") );
	cmbPlayRate->Append ( wxT("11025") );
	cmbPlayRate->Append ( wxT("8000") );
	//--- buffer length ---//
	tcBufferLength = new wxTextCtrl_NoFlicker	( this, -1, wxT("") );
	//--- max channels ---//
	PREF_CREATE_SPINCTRL2(SndMaxChan,4,32,4);
	
	//Use_MPEGACCURATE_ForMP3VBRFiles
	PREF_CREATE_CHECKBOX(Use_MPEGACCURATE_ForMP3VBRFiles,_("Use MPEGACCURATE for Mp3-VBR Files"));
	
	//-----------------------------//
	//--- Sound -> Driver Sizer ---//
	//-----------------------------//

	wxFlexGridSizer *fsSound_Driver = new wxFlexGridSizer( 6, 2, 2, 2 );
	fsSound_Driver->Add( PREF_STATICTEXT(_("Ouput Driver:")), 0, wxCENTER | wxRIGHT | wxALIGN_CENTER_VERTICAL, 0 );
	fsSound_Driver->Add( cmbOutputDrv, 1, 0, 0 );
	fsSound_Driver->Add( PREF_STATICTEXT( _("Sound Device:")), 0, wxCENTER | wxRIGHT | wxALIGN_CENTER_VERTICAL, 0 );
	fsSound_Driver->Add( cmbSndDevice, 1, 0, 0 );
	fsSound_Driver->Add( PREF_STATICTEXT(_("Playback Rate (hz):")), 0, wxCENTER | wxRIGHT | wxALIGN_CENTER_VERTICAL, 0 );
	fsSound_Driver->Add( cmbPlayRate, 1, 0, 0 );	
	fsSound_Driver->Add( PREF_STATICTEXT(_(" second buffer length") ), 0, wxCENTER | wxRIGHT | wxALIGN_CENTER_VERTICAL, 0 );
	fsSound_Driver->Add( tcBufferLength, 1, 0, 0 );
	fsSound_Driver->Add( PREF_STATICTEXT(_("Maximum sound channels:")), 0, wxCENTER | wxRIGHT | wxALIGN_CENTER_VERTICAL, 0 );
	fsSound_Driver->Add( sc2SndMaxChan, 1, 0, 0 );

	fsSound_Driver->AddGrowableCol(1);
	vsSound_Driver = new wxBoxSizer( wxVERTICAL );
	vsSound_Driver->Add( fsSound_Driver,	0, wxALL, 4  );
	vsSound_Driver->Add( chkUse_MPEGACCURATE_ForMP3VBRFiles );

	//-----------------------------------//
	//--- Options -> Selections Sizer ---//
	//-----------------------------------//
	vsOptions_Selections = new wxFlexGridSizer ( 4, 2, 2, 2 );

	//----------------------------//
	//--- Options -> Selection ---//
	//----------------------------//
	const wxString arrSelectionBoxes[] = 
		{
			_("None")  , 
			_("Artist"),
			_("Album") ,
			_("Genre") ,
			_("Year")  
		};

	//--- activity boxes ---//
	for(size_t i = 0; i < WXSIZEOF(cmbActivityBoxes);i++)
	{
		vsOptions_Selections->Add(  PREF_STATICTEXT( wxString::Format(_("Selection Box %d:"),i + 1)),	0, wxCENTER | wxRIGHT | wxALIGN_CENTER_VERTICAL, 0 );
		cmbActivityBoxes[i] = new wxComboBox( this, -1, wxEmptyString, wxDefaultPosition, wxDefaultSize, WXSIZEOF(arrSelectionBoxes), arrSelectionBoxes, wxCB_READONLY );
		vsOptions_Selections->Add( cmbActivityBoxes[i],	1, wxCENTER, 0 );
	}
	//--- selection style ---//
	cmbSelStyle = new wxComboBox( this, -1, wxT(""), wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_READONLY );
	cmbSelStyle->Append( _( "Standard" ) );
	cmbSelStyle->Append( _( "Sloppy" ) );
	cmbSelStyle->Append( _( "Highlight") );

	vsOptions_Selections->Add( PREF_STATICTEXT(_("Selection Style:" )),		0, wxCENTER | wxRIGHT | wxALIGN_CENTER_VERTICAL, 0 );
	vsOptions_Selections->Add( cmbSelStyle,		1, wxCENTER, 0 );

	//--------------------------//
	//--- Options -> General ---//
	//--------------------------//
	PREF_CREATE_CHECKBOX(AutoPlayOnAppStart,_("Automatically play song on startup"));
	PREF_CREATE_CHECKBOX(ShowLibraryOnStart,_("Show Library on startup"));
	PREF_CREATE_CHECKBOX(DoubleClickReplacesPlaylist,_("Double click replaces playlist"));
	
	PREF_CREATE_CHECKBOX(AutoPlayOnDropFilesInPlaylist,	_("Automatically play songs, dropped into playlist"));
	PREF_CREATE_CHECKBOX(StopSongOnNowPlayingClear,	_("Stop song, if Now Playing is cleared"));
#ifdef wxHAS_TASK_BAR_ICON
	PREF_CREATE_CHECKBOX(HideOnMinimize,	_("Hide Window on Minimize"));
#ifdef __WXMSW__
	PREF_CREATE_CHECKBOX(EnableBalloonSongInfo,	_("Enable balloon song info"));
#endif
#endif
	PREF_CREATE_CHECKBOX(AutoAdd,_("Automatically scan for new songs on startup"));
	PREF_CREATE_CHECKBOX(ShowAllSongs,	_("Selecting library shows all songs in playlist"));
	PREF_CREATE_CHECKBOX(BlankSwears,_("Censor common swearwords"));

	chkSortArtistWithoutPrefix	=	new wxCheckBox_NoFlicker( this, -1,	_("Sort Artist without prefix"),wxDefaultPosition,wxDefaultSize,0 );
	chkPlaylistStripes		=	new wxCheckBox_NoFlicker( this, -1,	_("Show \"stripes\" in playlist"),wxDefaultPosition,wxDefaultSize,0 );
	chkActivityBoxStripes	=	new wxCheckBox_NoFlicker( this, -1,	_("Show \"stripes\" in selection boxes"),wxDefaultPosition,wxDefaultSize,0 );
	chkSourcesBoxStripes	=	new wxCheckBox_NoFlicker( this, -1,	_("Show \"stripes\" in sources box"),wxDefaultPosition,wxDefaultSize,0 );
	chkPlaylistBorder		=   new wxCheckBox_NoFlicker( this, -1,	_("Use selected border colour"),wxDefaultPosition,wxDefaultSize,0 );
	btnPlaylistStripeColour	=	new wxButton_NoFlicker( this, MUSIK_PREFERENCES_PLAYLIST_STRIPE_COLOUR,	_("Set Color") );
	btnActivityStripeColour	=	new wxButton_NoFlicker( this, MUSIK_PREFERENCES_ACTIVITY_STRIPE_COLOUR,	_("Set Color") );
	btnSourcesStripeColour	=	new wxButton_NoFlicker( this, MUSIK_PREFERENCES_SOURCES_STRIPE_COLOUR,	_("Set Color") );
	btnPlaylistBorderColour =   new wxButton_NoFlicker( this, MUSIK_PREFERENCES_PLAYLIST_BORDER_COLOUR,	_("Set Color") );
	//--------------------------------//
	//--- Options -> General Sizer ---//
	//--------------------------------//
	vsOptions_Interface = new wxBoxSizer( wxVERTICAL );
	vsOptions_Interface->Add( chkAutoPlayOnAppStart,	0, wxALL, 4 );
	vsOptions_Interface->Add( chkShowLibraryOnStart,	0, wxALL, 4 );
	vsOptions_Interface->Add( chkDoubleClickReplacesPlaylist,	0, wxALL, 4 );

	vsOptions_Interface->Add( chkAutoPlayOnDropFilesInPlaylist,	0, wxALL, 4 );
	vsOptions_Interface->Add( chkStopSongOnNowPlayingClear,	0, wxALL, 4 );
#ifdef wxHAS_TASK_BAR_ICON
	vsOptions_Interface->Add( chkHideOnMinimize,		0, wxALL, 4 );
#ifdef __WXMSW__
	vsOptions_Interface->Add( chkEnableBalloonSongInfo,		0, wxALL, 4 );
#endif
#endif
	vsOptions_Interface->Add( chkAutoAdd,				0, wxALL, 4 );
	vsOptions_Interface->Add( chkShowAllSongs,			0, wxALL, 4 );
	vsOptions_Interface->Add( chkBlankSwears,			0, wxALL, 4 );
	vsOptions_Interface->Add( chkSortArtistWithoutPrefix,0, wxALL, 4 );
	vsOptions_Interface->Add( chkPlaylistStripes,		0, wxALL, 4 );
	vsOptions_Interface->Add( btnPlaylistStripeColour,	0, wxALL, 4 );
	vsOptions_Interface->Add( chkActivityBoxStripes,	0, wxALL, 4 );
	vsOptions_Interface->Add( btnActivityStripeColour,	0, wxALL, 4 );
	vsOptions_Interface->Add( chkSourcesBoxStripes,		0, wxALL, 4 );
	vsOptions_Interface->Add( btnSourcesStripeColour,	0, wxALL, 4 );
	vsOptions_Interface->Add( chkPlaylistBorder,		0, wxALL, 4 );
	vsOptions_Interface->Add( btnPlaylistBorderColour,	0, wxALL, 4 );

	//---------------------------------//
	//--- Options -> Playlist Sizer ---//
	//---------------------------------//
	vsOptions_Playlist = new wxBoxSizer( wxVERTICAL );
	
	wxFlexGridSizer *vsOptions_Playlist_Columns = new wxFlexGridSizer( NPLAYLISTCOLUMNS, 2, 2, 2 );
	vsOptions_Playlist->Add(vsOptions_Playlist_Columns);
	//---------------------------//
	//--- Options -> Playlist ---//
	//---------------------------//
	const wxString choicesCMBStatic[] = {  _("Static"),	 _("Dynamic") };

	for(size_t i = 0; i < NPLAYLISTCOLUMNS; i++)
	{
		chkPLColumnEnable[i] = new  wxCheckBox_NoFlicker(	this, -1, wxString(wxGetTranslation(g_PlaylistColumnLabels[i]))+wxT(":"));
		vsOptions_Playlist_Columns->Add(chkPLColumnEnable[i],		0, wxALIGN_CENTER_VERTICAL | wxADJUST_MINSIZE );
		cmbPLColumnStatic[i] = new wxComboBox( this, -1, wxT(""), wxDefaultPosition, wxDefaultSize, WXSIZEOF(choicesCMBStatic), choicesCMBStatic, wxCB_READONLY );
		vsOptions_Playlist_Columns->Add( cmbPLColumnStatic[i],		0 );
	}
	PREF_CREATE_CHECKBOX(DisplayEmptyPlaylistColumnAsUnkown,_("Display <unknown> in empty colums"));
	vsOptions_Playlist->Add( chkDisplayEmptyPlaylistColumnAsUnkown,		0, wxTOP, 5 );
	//------------------------------------//
	//--- options -> file associations ---//
	//------------------------------------//
	vsOptions_FileAssoc = new wxBoxSizer( wxVERTICAL );
	
	chklbFileAssocs = new wxCheckListBox(this,MUSIK_PREFERENCES_FILE_ASSOC_CHKLB);
	vsOptions_FileAssoc->Add(chklbFileAssocs,1,wxALL,5);
	
	for(int i = 0; i < COUNT_MUSIK_FORMAT ; i++)
	{
		const tSongClass *pSongClass = CMetaDataHandler::GetSongClass((EMUSIK_FORMAT_TYPE)i);
		if(pSongClass)
		{
			bool bRegisterdForwxMusik = false;
			wxString sDesc = wxGetTranslation(pSongClass->szDescription);
			GetFileTypeAssociationInfo(pSongClass->szExtension,NULL,&bRegisterdForwxMusik);
			chklbFileAssocs->Append(wxString::Format(wxT("%s %s"),pSongClass->szExtension,sDesc.c_str()));
			if(bRegisterdForwxMusik)
				chklbFileAssocs->Check(chklbFileAssocs->GetCount()-1);
		}
	}
	
	

	//-------------------------//
	//--- options -> tunage ---//
	//-------------------------//
// IMPORTANT! Create wxStaticBox BEFORE creation of
// controls which should be placed inside
// or else they wont be displayed on wxGTK		
	sbTunageFile = new wxStaticBox( this, -1, _("Write to file") );
	sbTunageURL = new wxStaticBox( this, -1, _("Post to a website") );
	sbTunageApp = new wxStaticBox( this, -1, _("Run a program") );
	sbTunageMisc = new wxStaticBox( this, -1, _("Misc.") );

	PREF_CREATE_CHECKBOX(TunageWriteFile,_("Enable"));
	PREF_CREATE_CHECKBOX(TunageAppendFile, _("Append to file"));
	PREF_CREATE_CHECKBOX(TunagePostURL, _("Enable"));
	PREF_CREATE_CHECKBOX(TunageRunApp, _("Enable"));
	PREF_CREATE_CHECKBOX(TunageRunOnStop,  _("Update when playing is stopped"));
	
	PREF_CREATE_TEXTCTRL(TunageFilename,wxFILTER_NONE);
	PREF_CREATE_TEXTCTRL(TunageFileLine,wxFILTER_NONE);
	PREF_CREATE_TEXTCTRL(TunageURL,wxFILTER_NONE);
	PREF_CREATE_TEXTCTRL(TunageCmdLine,wxFILTER_NONE);
	PREF_CREATE_TEXTCTRL(TunageStoppedText,wxFILTER_NONE);

	wxBoxSizer *hsTunageFilename = new wxBoxSizer( wxHORIZONTAL );
	hsTunageFilename->Add( PREF_STATICTEXT( _("Filename:")), 0, wxRIGHT | wxALIGN_CENTER_VERTICAL, 2 );
	hsTunageFilename->Add( tcTunageFilename, 1, 0, 0 );
	wxBoxSizer *hsTunageFileLine = new wxBoxSizer( wxHORIZONTAL );
	hsTunageFileLine->Add( PREF_STATICTEXT(_("Line to write:")), 0, wxRIGHT | wxALIGN_CENTER_VERTICAL, 2 );
	hsTunageFileLine->Add( tcTunageFileLine, 1, 0, 0 );
	wxBoxSizer *hsTunageURL = new wxBoxSizer( wxHORIZONTAL );
	hsTunageURL->Add( PREF_STATICTEXT(_("URL:")), 0, wxRIGHT | wxALIGN_CENTER_VERTICAL, 2 );
	hsTunageURL->Add( tcTunageURL, 1, 0, 0 );
	wxBoxSizer *hsTunageCmdLine = new wxBoxSizer( wxHORIZONTAL );
	hsTunageCmdLine->Add( PREF_STATICTEXT( _("Command line:")), 0, wxRIGHT | wxALIGN_CENTER_VERTICAL, 2 );
	hsTunageCmdLine->Add( tcTunageCmdLine, 1, 0, 0 );
	wxBoxSizer *hsTunageStoppedText = new wxBoxSizer( wxHORIZONTAL );
	hsTunageStoppedText->Add( PREF_STATICTEXT(_("Stopped Text:")), 0, wxRIGHT | wxALIGN_CENTER_VERTICAL, 2 );
	hsTunageStoppedText->Add( tcTunageStoppedText, 1, 0, 0 );

	

	wxStaticBoxSizer *vsTunageFile = new wxStaticBoxSizer( sbTunageFile, wxVERTICAL );
	vsTunageFile->Add( chkTunageWriteFile,		0, wxALL, 2 );
	vsTunageFile->Add( hsTunageFilename,		0, wxALL | wxEXPAND, 2 );
	vsTunageFile->Add( hsTunageFileLine,		0, wxALL | wxEXPAND, 2 );
	vsTunageFile->Add( chkTunageAppendFile,		0, wxALL, 2 );
	
	wxStaticBoxSizer *vsTunageURL = new wxStaticBoxSizer( sbTunageURL, wxVERTICAL );
	vsTunageURL->Add( chkTunagePostURL,		0, wxALL, 2 );
	vsTunageURL->Add( hsTunageURL,				0, wxALL | wxEXPAND, 2 );
	
	wxStaticBoxSizer *vsTunageApp = new wxStaticBoxSizer( sbTunageApp, wxVERTICAL );
	vsTunageApp->Add( chkTunageRunApp,			0, wxALL, 2 );
	vsTunageApp->Add( hsTunageCmdLine,			0, wxALL | wxEXPAND, 2 );
	
	wxStaticBoxSizer *vsTunageMisc = new wxStaticBoxSizer( sbTunageMisc, wxVERTICAL );
	vsTunageMisc->Add( chkTunageRunOnStop,		0, wxALL, 2 );
	vsTunageMisc->Add( hsTunageStoppedText,		0, wxALL | wxEXPAND, 2 );

	vsOptions_Tunage = new wxBoxSizer( wxVERTICAL );
	vsOptions_Tunage->Add( vsTunageFile,		0, wxALL | wxEXPAND, 2 );
	vsOptions_Tunage->Add( vsTunageURL,			0, wxALL | wxEXPAND, 2 );
	vsOptions_Tunage->Add( vsTunageApp,			0, wxALL | wxEXPAND, 2 );
	vsOptions_Tunage->Add( vsTunageMisc,		0, wxALL | wxEXPAND, 2 );


	

	//--- information ---//
	wxString sTunageInfo = 
		wxString( _("These variables can be used in file line, URL or commandline")) + 
		wxT(":                                                             \n\n")+
		wxT("$ARTIST     \t\t\t") + _("Artist")				+  _(" of current song\n") +
		wxT("$ALBUM      \t\t\t") + _("Album")				+  _(" of current song\n") +
		wxT("$TITLE      \t\t\t") + _("Title")				+  _(" of current song\n") +
		wxT("$YEAR      \t\t\t") + _("Year")				+  _(" of current song\n") +
		wxT("$TRACKNUM   \t\t\t") + _("Track number")			+  _(" of current song\n") +
		wxT("$TRACKLENGTH\t\t") + _("Track length in mm:ss")			+  _(" of current song\n") +
		wxT("$FILENAME   \t\t\t") + _("Filename")				+  _(" of current song\n") +
		wxT("$FILESIZE   \t\t\t") + _("Filesize")				+  _(" of current song\n") +
		wxT("$BITRATE    \t\t\t") + _("Bitrate")				+  _(" of current song\n") +
		wxT("$TIMESPLAYED\t\t") + _("Times played")				+  _(" of current song\n") +
		wxT("$NAME       \t\t\t") + _("Results in \"Artist - Title\", or in Stopped Text")	+  wxT("\n");

	vsOptions_Tunage->Add( PREF_STATICTEXT(sTunageInfo),		1, wxADJUST_MINSIZE| wxALL , 2 );

	//-------------------------//
	//--- options -> Auto DJ ---//
	//-------------------------//
	sbAutoDj = new wxStaticBox( this, -1, _("Auto DJ") );// IMPORTANT! Create wxStaticBox BEFORE creation of
							     // controls which should be placed inside
							     // or else they wont be displayed on wxGTK						

	PREF_CREATE_MULTILINETEXTCTRL2(AutoDjFilter,80,wxFILTER_NONE);
	
	PREF_CREATE_SPINCTRL(AutoDjDoNotPlaySongPlayedTheLastNHours,1,2000000,1);
	PREF_CREATE_SPINCTRL(AutoDJChooseSongsToPlayInAdvance,1,1000,1);
	PREF_CREATE_SPINCTRL(AutoDJChooseAlbumsToPlayInAdvance,1,20,1);
	wxBoxSizer *hsAutoDjFilter = new wxBoxSizer( wxVERTICAL );
	hsAutoDjFilter->Add( PREF_STATICTEXT( _("Use this filter to select songs:")), 0, wxRIGHT | wxALIGN_CENTER_VERTICAL, 2 );
	hsAutoDjFilter->Add( tc2AutoDjFilter, 1, wxEXPAND, 0 );
	wxBoxSizer *hsAutoDjDoNotPlaySongPlayedTheLastNHours = new wxBoxSizer( wxHORIZONTAL );
	hsAutoDjDoNotPlaySongPlayedTheLastNHours->Add( PREF_STATICTEXT(_("Number of hours a song must not have been played to be chosen:")), 0, wxRIGHT | wxALIGN_CENTER_VERTICAL, 2 );
	hsAutoDjDoNotPlaySongPlayedTheLastNHours->Add( scAutoDjDoNotPlaySongPlayedTheLastNHours, 1, 0, 0 );
	wxBoxSizer *hsAutoDJChooseSongsToPlayInAdvance = new wxBoxSizer( wxHORIZONTAL );
	hsAutoDJChooseSongsToPlayInAdvance->Add( PREF_STATICTEXT(_("Number of songs to choose in advance:")), 0, wxRIGHT | wxALIGN_CENTER_VERTICAL, 2 );
	hsAutoDJChooseSongsToPlayInAdvance->Add( scAutoDJChooseSongsToPlayInAdvance, 1, 0, 0 );
	wxBoxSizer *hsAutoDJChooseAlbumsToPlayInAdvance = new wxBoxSizer( wxHORIZONTAL );
	hsAutoDJChooseAlbumsToPlayInAdvance->Add( PREF_STATICTEXT(_("Number of albums to choose in advance:")), 0, wxRIGHT | wxALIGN_CENTER_VERTICAL, 2 );
	hsAutoDJChooseAlbumsToPlayInAdvance->Add( scAutoDJChooseAlbumsToPlayInAdvance, 1, 0, 0 );


	wxStaticBoxSizer *vsAutoDj = new wxStaticBoxSizer( sbAutoDj, wxVERTICAL );
	vsAutoDj->Add( hsAutoDjFilter,								0, wxALL | wxEXPAND, 2 );
	vsAutoDj->Add( hsAutoDjDoNotPlaySongPlayedTheLastNHours,	0, wxALL | wxEXPAND, 2 );
	vsAutoDj->Add( hsAutoDJChooseSongsToPlayInAdvance,			0, wxALL | wxEXPAND, 2 );
	vsAutoDj->Add( hsAutoDJChooseAlbumsToPlayInAdvance,			0, wxALL | wxEXPAND, 2 );


	sbShuffle =	 new wxStaticBox( this, -1, _("Shuffle") );
	PREF_CREATE_SPINCTRL(MaxShuffleHistory,0,10000,0);
	wxBoxSizer *hsMaxShuffleHistory = new wxBoxSizer( wxHORIZONTAL );
	hsMaxShuffleHistory->Add( PREF_STATICTEXT(_("Shuffle history size:")), 0, wxRIGHT | wxALIGN_CENTER_VERTICAL, 2 );
	hsMaxShuffleHistory->Add( scMaxShuffleHistory, 1, 0, 0 );


	wxStaticBoxSizer *vsShuffle = new wxStaticBoxSizer( sbShuffle, wxVERTICAL );
	vsShuffle->Add( hsMaxShuffleHistory,								0, wxALL | wxEXPAND, 2 );

	vsOptions_AutoDj = new wxBoxSizer( wxVERTICAL );

	vsOptions_AutoDj->Add( vsShuffle,		0, wxALL | wxEXPAND, 2 );
	vsOptions_AutoDj->Add( vsAutoDj,		0, wxALL | wxEXPAND, 2 );
	

	//---------------------------//
	//--- Streaming -> Buffer ---//
	//---------------------------//

	
	PREF_CREATE_SPINCTRL2(StreamingBufferSize,64000,256000,64000);
	PREF_CREATE_SPINCTRL2(StreamingPreBufferPercent,10,99,10);
	PREF_CREATE_SPINCTRL2(StreamingReBufferPercent,10,99,10);

	vsStreaming_Buffer = new wxFlexGridSizer( 3,2,2,2 );

	vsStreaming_Buffer->Add( PREF_STATICTEXT( _("Buffer Size (bytes):")) );
	vsStreaming_Buffer->Add( sc2StreamingBufferSize );
	vsStreaming_Buffer->Add( PREF_STATICTEXT( _("Prebuffering (%):")) );
	vsStreaming_Buffer->Add( sc2StreamingPreBufferPercent );
	vsStreaming_Buffer->Add( PREF_STATICTEXT(_("Rebuffering (%):")) );
	vsStreaming_Buffer->Add( sc2StreamingReBufferPercent );

 	//--------------------------------//
	//--- Streaming -> ProxyServer ---//
	//--------------------------------//
	chkUseProxyServer		= new wxCheckBox_NoFlicker	( this, -1,	_("Use Proxy server") );

	tcProxyServer			= new wxTextCtrl_NoFlicker( this, -1);
	tcProxyServerPort		= new wxTextCtrl_NoFlicker( this, -1);
	tcProxyServerUser		= new wxTextCtrl_NoFlicker( this, -1);
	tcProxyServerPassword	= new wxTextCtrl_NoFlicker( this, -1);
	
	wxFlexGridSizer * fsProxySizer		= new wxFlexGridSizer( 4,2,2,2 );

	fsProxySizer->Add(PREF_STATICTEXT(_("Proxy server address:")));
	fsProxySizer->Add(tcProxyServer);
	fsProxySizer->Add(PREF_STATICTEXT(_("Proxy server port:")));
	fsProxySizer->Add(tcProxyServerPort);
	fsProxySizer->Add(PREF_STATICTEXT(_("Proxy server user:")));
	fsProxySizer->Add(tcProxyServerUser);
	fsProxySizer->Add(PREF_STATICTEXT(_("Proxy server password:")));
	fsProxySizer->Add(tcProxyServerPassword);

	vsStreaming_ProxyServer	= new wxBoxSizer( wxVERTICAL );

	vsStreaming_ProxyServer->Add( chkUseProxyServer,		0, wxALL, 2 );
	vsStreaming_ProxyServer->Add( fsProxySizer,		0, wxALL | wxEXPAND, 4 );
	//--------------------------//
	//--- Tagging -> General ---//
	//--------------------------//
	PREF_CREATE_CHECKBOX(ActBoxWrite,_("Write tag to file"));
	PREF_CREATE_CHECKBOX(ActBoxClear,_("Clear old tag"));
	PREF_CREATE_CHECKBOX(ActBoxRename,_("Automatically rename file"));
	PREF_CREATE_CHECKBOX(TagDlgWrite,_("Write tag to file"));
	PREF_CREATE_CHECKBOX(TagDlgClear,_("Clear old tag"));
	PREF_CREATE_CHECKBOX(TagDlgRename,_("Automatically rename file"));
	//--------------------------------//
	//--- Tagging -> General Sizer ---//
	//--------------------------------//
	vsTagging_General = new wxBoxSizer( wxVERTICAL );
	vsTagging_General->Add	( PREF_STATICTEXT( _("Selection Boxes (artist/album/etc):")),		0, wxALL | wxEXPAND, 4 );
	vsTagging_General->Add	( chkActBoxWrite,		0, wxALL | wxEXPAND, 4 );
	vsTagging_General->Add	( chkActBoxClear,		1, wxALL | wxEXPAND, 4 );
	vsTagging_General->Add	( chkActBoxRename,	1, wxALL | wxEXPAND, 4 );
	vsTagging_General->Add	( PREF_STATICTEXT(_("\nTag Dialog Box:")),			0, wxALL | wxEXPAND, 4 );
	vsTagging_General->Add	( chkTagDlgWrite,		0, wxALL | wxEXPAND, 4 );
	vsTagging_General->Add	( chkTagDlgClear,		1, wxALL | wxEXPAND, 4 );
	vsTagging_General->Add	( chkTagDlgRename,		1, wxALL | wxEXPAND, 4 );

	//---------------------------//
	//--- Tagging -> Auto Tag ---//
	//---------------------------//
	//--- rename options and sizer ---//
	PREF_CREATE_TEXTCTRL(  AutoRename,wxFILTER_NONE);
	wxBoxSizer *hsRename	= new wxBoxSizer	( wxHORIZONTAL );
	hsRename->Add ( PREF_STATICTEXT( _("Rename:")), 0, wxRIGHT | wxALIGN_CENTER_VERTICAL, 2 );
	hsRename->Add ( tcAutoRename, 1, wxEXPAND, 0 );
	//--- information ---//
	wxString sRenameInfo = 
		wxString( _("\nAuto Rename syntax:\n\n") ) + 
		wxString(wxFileName::GetPathSeparator()) + _( " - Directory Separator\n" )  +
		wxString( _("%1 - Song Title\n")				) + 
		wxString( _("%2 - Artist Name\n")				) +
		wxString( _("%3 - Album Name\n")				) +
		wxString( _("%4 - Genre\n")						) +
		wxString( _("%5 - Year\n")						) +
		wxString( _("%6 - Track Number\n\n")			) +
		wxString(MUSIKAPPNAME)							  +	
		wxString( _(" will not delete empty directories!" ) );

	//---------------------------------//
	//--- Tagging -> Auto Tag Sizer ---//
	//---------------------------------//
	vsTagging_Auto = new wxBoxSizer( wxVERTICAL );
	vsTagging_Auto->Add	( hsRename, 0, wxALL | wxEXPAND, 4 );
	vsTagging_Auto->Add ( PREF_STATICTEXT(sRenameInfo), 0, wxADJUST_MINSIZE | wxALL | wxEXPAND, 4 );

	//----------------------//
	//--- System Buttons ---//
	//----------------------//
	wxButton* btnCancel =	new wxButton_NoFlicker( this, wxID_CANCEL,	_("Cancel"),	wxDefaultPosition, wxDefaultSize );
	wxButton* btnApply =	new wxButton_NoFlicker( this, wxID_APPLY,	_("Apply"),		wxDefaultPosition, wxDefaultSize );
	wxButton* btnOK =		new wxButton_NoFlicker( this, wxID_OK,		_("OK"),		wxDefaultPosition, wxDefaultSize );

	//----------------------------//
	//--- System Buttons Sizer ---//
	//----------------------------//
	wxBoxSizer *hsSysButtons = new wxBoxSizer( wxHORIZONTAL );
	hsSysButtons->Add( btnCancel,	0, wxALIGN_LEFT		);
	hsSysButtons->Add( -1,-1,	1, wxEXPAND			);
	hsSysButtons->Add( btnApply,	0, wxALIGN_RIGHT	);
	hsSysButtons->Add( btnOK,		0, wxALIGN_RIGHT | wxLEFT, 4);

	//-------------------------//
	//--- Hide / Show Sizer ---//
	//-------------------------//
	hsSplitter = new wxBoxSizer( wxHORIZONTAL );
	hsSplitter->Add( tcPreferencesTree,		1, wxEXPAND | wxRIGHT, 8 );
	hsSplitter->Add( vsOptions_Selections,	3 );
	hsSplitter->Add( vsOptions_Interface,	3 );
	hsSplitter->Add( vsOptions_Playlist,	3 );
	hsSplitter->Add( vsOptions_FileAssoc,	3 );
	hsSplitter->Add( vsOptions_Tunage,		3 );
	hsSplitter->Add( vsOptions_AutoDj,		3 );
	hsSplitter->Add( vsSound_Crossfader,	3 );
	hsSplitter->Add( vsSound_Driver,		3 );
	hsSplitter->Add( vsTagging_General,		3 );
	hsSplitter->Add( vsTagging_Auto,		3 );
	hsSplitter->Add( vsStreaming_Buffer,	3 );
	hsSplitter->Add( vsStreaming_ProxyServer,	3 );
	
	//-----------------//
	//--- Top Sizer ---//
	//-----------------//
	vsTopSizer = new wxBoxSizer( wxVERTICAL );
	vsTopSizer->Add( hsSplitter,	1, wxEXPAND | wxALL, 2 );
	vsTopSizer->Add( hsSysButtons,	0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 2 );
	SetSizer( vsTopSizer );

	//--------------//
	//--- Layout ---//
	//--------------//
	
	Centre();
	Layout();
	HidePanels();
	//--------------------//
	//---  Load Prefs  ---//
	//--- Set Defaults ---//
	//--------------------//
	LoadPrefs();
	tcPreferencesTree->SelectItem( nInterfaceID );
}

//--------------//
//--- Events ---//
//--------------//
void MusikPrefsDialog::OnTreeChange		( wxTreeEvent& WXUNUSED(event) )	{	UpdatePrefsPanel();		}
void MusikPrefsDialog::OnOutputChanged	( wxCommandEvent& WXUNUSED(event) )	{	FindDevices();			}
void MusikPrefsDialog::OnClickOK			( wxCommandEvent& WXUNUSED(event) )	{	Close( false );			}
void MusikPrefsDialog::OnClickApply		( wxCommandEvent& WXUNUSED(event) )	{	SavePrefs();			}
void MusikPrefsDialog::OnClickCancel		( wxCommandEvent& WXUNUSED(event) )	{	Close( true );			}
void MusikPrefsDialog::OnClose			( wxCloseEvent& WXUNUSED(event) )	{	Close( true );			}

void MusikPrefsDialog::OnClickColour		( wxCommandEvent &event )
{
	//--- show the standard color dialog to change the background color of the button ---//
	wxButton_NoFlicker* Button = (wxButton_NoFlicker*)event.GetEventObject();
	Button->SetBackgroundColour( wxGetColourFromUser( this, Button->GetBackgroundColour() ) );
}

void MusikPrefsDialog::Close( bool bCancel )
{
	if ( !bCancel )
		if(!SavePrefs())
			return;

	g_MusikFrame->Enable( TRUE );
	this->Destroy();
	
}

void MusikPrefsDialog::LoadPrefs()
{
	TransferDataToWindow();
	//--------------------------//
	//--- options -> general ---//
	//--------------------------//
	chkSortArtistWithoutPrefix->SetValue( wxGetApp().Prefs.bSortArtistWithoutPrefix );
	chkPlaylistStripes->SetValue	( wxGetApp().Prefs.bPLStripes );
	chkActivityBoxStripes->SetValue	( wxGetApp().Prefs.bActStripes );
	chkSourcesBoxStripes->SetValue	( wxGetApp().Prefs.bSourcesStripes );
	chkPlaylistBorder->SetValue	( wxGetApp().Prefs.bPlaylistBorder );

	btnPlaylistStripeColour->SetBackgroundColour( StringToColour( wxGetApp().Prefs.sPLStripeColour ) );
	btnActivityStripeColour->SetBackgroundColour( StringToColour( wxGetApp().Prefs.sActStripeColour ) );
	btnSourcesStripeColour->SetBackgroundColour( StringToColour( wxGetApp().Prefs.sSourcesStripeColour ) );
	btnPlaylistBorderColour->SetBackgroundColour( StringToColour( wxGetApp().Prefs.sPlaylistBorderColour ) );

	//-----------------------------//
	//--- options -> selections ---//
	//-----------------------------//
	cmbSelStyle->SetSelection		( wxGetApp().Prefs.eSelStyle.val );
	for(size_t i = 0; i < WXSIZEOF(cmbActivityBoxes);i++)
		cmbActivityBoxes[i]->SetSelection	( wxGetApp().Prefs.nActBoxType[i] );

	//---------------------------//
	//--- options -> playlist ---//
	//---------------------------//
	for(size_t i = 0 ;i < NPLAYLISTCOLUMNS; i ++)
	{
		chkPLColumnEnable[i]->SetValue			( wxGetApp().Prefs.bPlaylistColumnEnable[i]);
		cmbPLColumnStatic[i]->SetSelection		( wxGetApp().Prefs.bPlaylistColumnDynamic[PLAYLISTCOLUMN_RATING]			);
	}

	//---------------------------//
	//--- sound -> crossfader ---//
	//---------------------------//

	float		fDuration;
	wxString	sDuration;

	fDuration =							(float)wxGetApp().Prefs.nFadeDuration / 1000;
	sDuration.sprintf					( wxT("%.1f"), fDuration );
	tcDuration->SetValue				( sDuration );

	fDuration =							(float)wxGetApp().Prefs.nFadeSeekDuration / 1000;
	sDuration.sprintf					( wxT("%.1f"), fDuration );
	tcSeekDuration->SetValue			( sDuration );

	fDuration =							(float)wxGetApp().Prefs.nFadePauseResumeDuration / 1000;
	sDuration.sprintf					( wxT("%.1f"), fDuration );
	tcPauseResumeDuration->SetValue		( sDuration );

	fDuration =							(float)wxGetApp().Prefs.nFadeStopDuration / 1000;
	sDuration.sprintf					( wxT("%.1f"), fDuration );
	tcStopDuration->SetValue			( sDuration );

	fDuration =							(float)wxGetApp().Prefs.nFadeExitDuration / 1000;
	sDuration.sprintf					( wxT("%.1f"), fDuration );
	tcExitDuration->SetValue			( sDuration );

	//-----------------------//
	//--- sound -> driver ---//
	//-----------------------//
	FindDevices();
	wxString sSndRate, sLength;
	float fLength;
	cmbOutputDrv->SetSelection		( wxGetApp().Prefs.nSndOutput );
    cmbSndDevice->SetSelection		( wxGetApp().Prefs.nSndDevice );
	sSndRate.sprintf				( wxT("%d"), wxGetApp().Prefs.nSndRate.val );
	fLength =						(float)wxGetApp().Prefs.nSndBuffer / 1000;
	sLength.sprintf					( wxT("%.1f"), fLength );
	tcBufferLength->SetValue		( sLength );
	cmbPlayRate->SetSelection		( cmbPlayRate->FindString ( sSndRate ) );

	//---------------------------------//
	//--- streaming -> proxy server ---//
	//---------------------------------//
	chkUseProxyServer->SetValue( wxGetApp().Prefs.bUseProxyServer );
	tcProxyServer->SetValue( wxGetApp().Prefs.sProxyServer );		
	tcProxyServerPort->SetValue( wxGetApp().Prefs.sProxyServerPort );		
	tcProxyServerUser->SetValue( wxGetApp().Prefs.sProxyServerUser );	
	tcProxyServerPassword->SetValue( wxGetApp().Prefs.sProxyServerPassword );	
}

void MusikPrefsDialog::FindDevices()
{
	cmbSndDevice->Clear();
	for ( int i = 0; i < FSOUND_GetNumDrivers(); i++ )
		cmbSndDevice->Append( ConvA2W( FSOUND_GetDriverName( i ) ) );
	if ( cmbSndDevice->GetCount() < 1 )
		cmbSndDevice->Append( _("[No Devices]") );
	cmbSndDevice->SetSelection( 0 );
}

void MusikPrefsDialog::HidePanels()
{
	hsSplitter->Show( vsOptions_Selections,	false );
	hsSplitter->Show( vsOptions_Interface,	false );
	hsSplitter->Show( vsOptions_Playlist,	false );
	hsSplitter->Show( vsOptions_FileAssoc,	false );
	
	hsSplitter->Show( vsOptions_Tunage,		false );
	sbTunageFile->Show( false );
	sbTunageURL->Show( false );
	sbTunageApp->Show( false );
	sbTunageMisc->Show( false );
	hsSplitter->Show( vsOptions_AutoDj,		false );
	sbAutoDj->Show( false );
	sbShuffle->Show( false );
	hsSplitter->Show( vsSound_Crossfader,	false );
	hsSplitter->Show( vsSound_Driver,		false );
	hsSplitter->Show( vsTagging_General,	false );
	hsSplitter->Show( vsTagging_Auto,		false );
	hsSplitter->Show( vsStreaming_Buffer,		false );
 	hsSplitter->Show( vsStreaming_ProxyServer,		false );

}

void MusikPrefsDialog::UpdatePrefsPanel()
{
	if( tcPreferencesTree->GetSelection() == nPlaybackID )
	{
		HidePanels();
		hsSplitter->Show( vsSound_Crossfader, true );
	}
	else if( tcPreferencesTree->GetSelection() == nDriverID )
	{
		HidePanels();
		hsSplitter->Show( vsSound_Driver, true );
	}
	else if( tcPreferencesTree->GetSelection() == nSelectionsID )
	{
		HidePanels();
		hsSplitter->Show( vsOptions_Selections, true );
	}
	else if( tcPreferencesTree->GetSelection() == nInterfaceID )
	{
		HidePanels();
		hsSplitter->Show( vsOptions_Interface, true );
	}
	else if ( tcPreferencesTree->GetSelection() == nAutoTagID )
	{
		HidePanels();
		hsSplitter->Show( vsTagging_Auto, true );
	}
	else if ( tcPreferencesTree->GetSelection() == nGeneralTagID )
	{
		HidePanels();
		hsSplitter->Show( vsTagging_General, true );
	}
	else if ( tcPreferencesTree->GetSelection() == nPlaylistID )
	{
		HidePanels();
		hsSplitter->Show( vsOptions_Playlist, true );
	}
	else if ( tcPreferencesTree->GetSelection() == nFileAssocsID )
	{
		HidePanels();
		hsSplitter->Show( vsOptions_FileAssoc, true );
	}
	else if ( tcPreferencesTree->GetSelection() == nTunageID )
	{
		HidePanels();
		hsSplitter->Show( vsOptions_Tunage, true );
		sbTunageFile->Show( true );
		sbTunageURL->Show( true );
		sbTunageApp->Show( true );
		sbTunageMisc->Show( true );

	}
	else if ( tcPreferencesTree->GetSelection() == nAutoDjID )
	{
		HidePanels();
		hsSplitter->Show( vsOptions_AutoDj, true );
		sbAutoDj->Show( true );
		sbShuffle->Show( true );
	}
 	else if ( tcPreferencesTree->GetSelection() == nStreamingBufferID )
	{
		HidePanels();
		hsSplitter->Show( vsStreaming_Buffer, true );
	}
 	else if ( tcPreferencesTree->GetSelection() == nStreamingProxyServerID )
	{
		HidePanels();
		hsSplitter->Show( vsStreaming_ProxyServer, true );
	}

	this->Layout();
}

void MusikPrefsDialog::OnTranslateKeys( wxKeyEvent& event )
{
	if ( event.GetKeyCode() == WXK_ESCAPE )
		Close( true );
	else
		event.Skip();
}

bool MusikPrefsDialog::SavePrefs()
{
	if(!Validate() )
		return false;
	bool bRestartFMOD		= false;
	bool bShowUnselChange	= false;
	bool bActivityChange	= false;
	bool bPlaylistUpdate = false;
	bool bActivityUpdate = false;
	bool bSourcesUpdate = false;
	//--------------------------//
	//--- Options -> general ---//
	//--------------------------//


	if(wxGetApp().Prefs.bSortArtistWithoutPrefix != chkSortArtistWithoutPrefix->GetValue())
	{
		bPlaylistUpdate = true;
		bActivityUpdate = true;
		wxGetApp().Prefs.bSortArtistWithoutPrefix = chkSortArtistWithoutPrefix->GetValue();
	}
	if ( chkPlaylistStripes->GetValue() != wxGetApp().Prefs.bPLStripes )
	{
		wxGetApp().Prefs.bPLStripes = chkPlaylistStripes->GetValue();
		bPlaylistUpdate = true;
	}
	if ( ColourToString( btnPlaylistStripeColour->GetBackgroundColour() ) != wxGetApp().Prefs.sPLStripeColour )
	{
		wxGetApp().Prefs.sPLStripeColour = ColourToString( btnPlaylistStripeColour->GetBackgroundColour() );
		bPlaylistUpdate = true;
	}
	if ( chkPlaylistBorder->GetValue() != wxGetApp().Prefs.bPlaylistBorder )
	{
		wxGetApp().Prefs.bPlaylistBorder = chkPlaylistBorder->GetValue();
		bPlaylistUpdate = true;
		bSourcesUpdate = true;
	}
	if ( ColourToString( btnPlaylistBorderColour->GetBackgroundColour() ) != wxGetApp().Prefs.sPlaylistBorderColour )
	{
		wxGetApp().Prefs.sPlaylistBorderColour = ColourToString( btnPlaylistBorderColour->GetBackgroundColour() );
		bPlaylistUpdate = true;
		bSourcesUpdate = true;
	}

	if ( bPlaylistUpdate )
		g_PlaylistBox->Update();

	if ( chkActivityBoxStripes->GetValue() != wxGetApp().Prefs.bActStripes )
	{
		wxGetApp().Prefs.bActStripes = chkActivityBoxStripes->GetValue();
		bActivityUpdate = true;
	}
	if ( ColourToString( btnActivityStripeColour->GetBackgroundColour() ) != wxGetApp().Prefs.sActStripeColour )
	{
		wxGetApp().Prefs.sActStripeColour = ColourToString( btnActivityStripeColour->GetBackgroundColour() );
		bActivityUpdate = true;
	}
	if ( bActivityUpdate )
		g_ActivityAreaCtrl->ResetAllContents();

	
	if ( chkSourcesBoxStripes->GetValue() != wxGetApp().Prefs.bSourcesStripes )
	{
		wxGetApp().Prefs.bSourcesStripes = chkSourcesBoxStripes->GetValue();
		bSourcesUpdate = true;
	}
	if ( ColourToString( btnSourcesStripeColour->GetBackgroundColour() ) != wxGetApp().Prefs.sSourcesStripeColour )
	{
		wxGetApp().Prefs.sSourcesStripeColour = ColourToString( btnSourcesStripeColour->GetBackgroundColour() );
		bSourcesUpdate = true;
	}
	if ( bSourcesUpdate )
		g_SourcesCtrl->Update();

	//-----------------------------//
    //--- Options -> selections ---//
	//-----------------------------//
	if ( (EMUSIK_ACTIVITY_SELECTION_TYPE)cmbSelStyle->GetSelection() != wxGetApp().Prefs.eSelStyle )
	{
		wxGetApp().Prefs.eSelStyle = (EMUSIK_ACTIVITY_SELECTION_TYPE)cmbSelStyle->GetSelection();
		bShowUnselChange = true;
	}
	for(size_t i = 0; i < WXSIZEOF(cmbActivityBoxes);i++)
	{
		if ( cmbActivityBoxes[i]->GetSelection() != wxGetApp().Prefs.nActBoxType[i] )
		{
			wxGetApp().Prefs.nActBoxType[i] = (EMUSIK_ACTIVITY_TYPE)cmbActivityBoxes[i]->GetSelection();
			bActivityChange = true;
		}
	}


	//---------------------------//
	//--- options -> playlist ---//
	//---------------------------//
	bool bResetColumns = false;
	for(size_t i = 0 ;i < NPLAYLISTCOLUMNS; i ++)
	{
		//--- enable / disable ---//
		if ( wxGetApp().Prefs.bPlaylistColumnEnable[i] != chkPLColumnEnable[i]->GetValue() )
		{
			wxGetApp().Prefs.bPlaylistColumnEnable[i] = chkPLColumnEnable[i]->GetValue();
			bResetColumns = true;
		}
		//--- standard / dynamic ---//
		if ( wxGetApp().Prefs.bPlaylistColumnDynamic[i] != (cmbPLColumnStatic[i]->GetSelection() ? true : false) )
		{
			wxGetApp().Prefs.bPlaylistColumnDynamic[i] = cmbPLColumnStatic[i]->GetSelection()? true : false;
			bResetColumns = true;
		}
	}
	//-------------------------//
	//--- options -> Auto DJ ---//
	//-------------------------//
	if ( tc2AutoDjFilter->GetValue()  != wxGetApp().Prefs.sAutoDjFilter )
	{
		wxGetApp().Prefs.sAutoDjFilter = tc2AutoDjFilter->GetValue(); 
		if(!wxGetApp().Library.SetAutoDjFilter(wxGetApp().Prefs.sAutoDjFilter))
		{
			wxMessageBox( _( "An error occured when setting the Auto DJ Filter" ), MUSIKAPPNAME_VERSION, wxOK | wxICON_ERROR );
		}
	}
	
	double fDuration;
	int nDuration;
	
	fDuration = StringToDouble( tcDuration->GetValue() );
	nDuration = ( int )( fDuration * 1000 );
	wxGetApp().Prefs.nFadeDuration = nDuration;
	
    fDuration = StringToDouble( tcSeekDuration->GetValue() );
	nDuration = ( int )( fDuration * 1000 );
	wxGetApp().Prefs.nFadeSeekDuration = nDuration;

	fDuration = StringToDouble( tcPauseResumeDuration->GetValue() );
	nDuration = ( int )( fDuration * 1000 );
	wxGetApp().Prefs.nFadePauseResumeDuration = nDuration;
	
	fDuration = StringToDouble( tcStopDuration->GetValue() );
	nDuration = ( int )( fDuration * 1000 );
	wxGetApp().Prefs.nFadeStopDuration = nDuration;
	
	fDuration = StringToDouble( tcExitDuration->GetValue() );
	nDuration = ( int )( fDuration * 1000 );
	wxGetApp().Prefs.nFadeExitDuration = nDuration;
	
	//-----------------------//
	//--- sound -> driver ---//
	//-----------------------//
	if ( cmbOutputDrv->GetSelection() != wxGetApp().Prefs.nSndOutput )
	{
		wxGetApp().Prefs.nSndOutput = cmbOutputDrv->GetSelection();
		bRestartFMOD = true;
	}
	if ( cmbSndDevice->GetSelection() != wxGetApp().Prefs.nSndDevice )
	{
		wxGetApp().Prefs.nSndDevice = cmbSndDevice->GetSelection();
		bRestartFMOD = true;
	}
	if ( wxStringToInt( cmbPlayRate->GetString( cmbPlayRate->GetSelection() ) ) != wxGetApp().Prefs.nSndRate )
	{
		int nRate = wxStringToInt( cmbPlayRate->GetString( cmbPlayRate->GetSelection() ) );
		wxGetApp().Prefs.nSndRate = nRate;
		bRestartFMOD = true;
	}
	if ( sc2SndMaxChan->GetValue() != wxGetApp().Prefs.nSndMaxChan )
	{
		bRestartFMOD = true;
	}
	double fLength = StringToDouble( tcBufferLength->GetValue() );
	int nLength = ( int )( fLength * 1000 );
	wxGetApp().Prefs.nSndBuffer = nLength;


	//---------------------------//
	//--- streaming -> buffer ---//
	//---------------------------//
	bool bNetBufferSettingChanged = false;
 	if (  sc2StreamingBufferSize->GetValue( )  != wxGetApp().Prefs.nStreamingBufferSize )
	{
		bNetBufferSettingChanged = true;
	}
 	if ( sc2StreamingPreBufferPercent->GetValue( ) != wxGetApp().Prefs.nStreamingPreBufferPercent )
	{
		bNetBufferSettingChanged = true;
	}
 	if ( sc2StreamingReBufferPercent->GetValue( ) != wxGetApp().Prefs.nStreamingReBufferPercent )
	{
		bNetBufferSettingChanged = true;
	}
	//---------------------------------//
	//--- streaming -> proxy server ---//
	//---------------------------------//
	wxGetApp().Prefs.bUseProxyServer = chkUseProxyServer->GetValue();
	wxGetApp().Prefs.sProxyServer = tcProxyServer->GetValue(); 
	wxGetApp().Prefs.sProxyServer = tcProxyServer->GetValue(  );		
	wxGetApp().Prefs.sProxyServerPort = tcProxyServerPort->GetValue(  );		
	wxGetApp().Prefs.sProxyServerUser = tcProxyServerUser->GetValue(  );	
	wxGetApp().Prefs.sProxyServerPassword = tcProxyServerPassword->GetValue(  );	


	//--- if we need to restart fmod ---//
	if ( bRestartFMOD )
		wxGetApp().Player.InitializeFMOD( FMOD_INIT_RESTART );
	else if(bNetBufferSettingChanged)
	{
		wxGetApp().Player.InitFMOD_NetBuffer();
	}

	wxGetApp().Player.InitFMOD_ProxyServer();

	if ( bActivityChange )
	{
		g_ActivityAreaCtrl->Delete();
		g_ActivityAreaCtrl->Create();
		g_MusikFrame->Layout();
		g_ActivityAreaCtrl->ResetAllContents();
	}
	else if ( bShowUnselChange )
		g_ActivityAreaCtrl->ResetAllContents();

	if ( bResetColumns )
		g_PlaylistBox->PlaylistCtrl().ResetColumns( true, true );
	
	if(bSourcesUpdate || bActivityUpdate || bPlaylistUpdate)
	{
		// force update of evrything
		g_MusikFrame->Show(false);
		g_MusikFrame->Show();	
	}
	DoFileAssociations();
	TransferDataFromWindow();
	return true;
}

void MusikPrefsDialog::DoFileAssociations()
{

	bool bNotifyExplorer = false;
	for(int i = 0; i < chklbFileAssocs->GetCount() ; i++)
	{
		wxString sEntry =	chklbFileAssocs->GetString(i);
		wxArrayString arrElements;
		DelimitStr(sEntry,wxT(" "),arrElements);
		if(chklbFileAssocs->IsChecked(i))
		{
			if(!FileTypeIsAssociated(arrElements[0]))
			{
				const tSongClass *pSongClass = CMetaDataHandler::GetSongClass(arrElements[0]);
				if(pSongClass)
				{
					wxString sDesc = wxGetTranslation(pSongClass->szDescription);
					AssociateWithFileType(arrElements[0],sDesc);
					bNotifyExplorer = true;
				}
			}

		}
		else
		{
			if(FileTypeIsAssociated(arrElements[0]))
			{
				UnassociateWithFileType(arrElements[0]);
				bNotifyExplorer = true;
			}
		}
	}
#ifdef __WXMSW__
	if(bNotifyExplorer)
		SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);
#endif
}
