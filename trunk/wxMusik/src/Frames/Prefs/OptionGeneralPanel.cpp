/*
*  OptionGeneralPanel.cpp
*
*  OptionGeneralPanel 
*  
*  Copyright (c) 2005 Gunnar Roth (gunnar.roth@gmx.de)
*	Contributors: Casey Langen, Simon Windmill, Dustin Carter, Wade Brainerd
*
*  See the file "license.txt" for information on usage and redistribution
*  of this file, and for a DISCLAIMER OF ALL WARRANTIES.
*/

//--- For compilers that support precompilation, includes "wx/wx.h". ---//
//--- wx ---//
#include "myprec.h"
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif 

#include "OptionGeneralPanel.h"

#include <wx/colordlg.h>
#include "Frames/MusikFrame.h"
#include "Classes/ActivityAreaCtrl.h"
#include "Classes/PlaylistCtrl.h"
#include "Classes/SourcesBox.h"
#include "MusikGlobals.h"


BEGIN_EVENT_TABLE(OptionGeneralPanel, PrefPanel)
END_EVENT_TABLE()


IMPLEMENT_PREFPANEL_CONSTRUCTOR(OptionGeneralPanel)

wxSizer * OptionGeneralPanel::CreateControls()
{
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
    PREF_CREATE_CHECKBOX(CopyPreserveDirectories,_("Preserve directories when copying"));


    PREF_CREATE_SPINCTRL_EX(DBCacheSize,1000,10000000,&snkDBCacheSize);

    //--------------------------------//
    //--- Options -> General Sizer ---//
    //--------------------------------//
    wxBoxSizer *vsOptions_Interface = new wxBoxSizer( wxVERTICAL );
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
	vsOptions_Interface->Add( chkCopyPreserveDirectories,0, wxALL, 4 );
	
    wxBoxSizer *hsDBCacheSize = new wxBoxSizer( wxHORIZONTAL );
    hsDBCacheSize->Add( PREF_STATICTEXT(_("Database Cache Size:")), 0, wxRIGHT | wxALIGN_CENTER_VERTICAL, 2 );
    hsDBCacheSize->Add( scDBCacheSize, 0, 0, 0 );

    vsOptions_Interface->Add( hsDBCacheSize,0, wxALL, 2 );

    
    return vsOptions_Interface;
}

void OptionGeneralPanel::DoLoadPrefs()
{
}
bool OptionGeneralPanel::DoSavePrefs()
{
    return true;
}

void OptionGeneralPanel::AfterDataTransferredFromWindow()
{

}
void OptionGeneralPanel::DbChacheSizeSink::OnValueChange(wxGenericValidator *,wxWindowBase *)
{
    wxGetApp().Library.SetCacheSize(wxGetApp().Prefs.nDBCacheSize);
}


