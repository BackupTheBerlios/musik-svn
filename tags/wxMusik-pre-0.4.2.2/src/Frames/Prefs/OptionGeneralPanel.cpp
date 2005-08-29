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

enum OBJECT_ID
{
    MUSIK_PREFERENCES_PLAYLIST_STRIPE_COLOUR = 1,
    MUSIK_PREFERENCES_ACTIVITY_STRIPE_COLOUR,
    MUSIK_PREFERENCES_SOURCES_STRIPE_COLOUR,
    MUSIK_PREFERENCES_PLAYLIST_BORDER_COLOUR,
};
BEGIN_EVENT_TABLE(OptionGeneralPanel, PrefPanel)
EVT_BUTTON				(MUSIK_PREFERENCES_PLAYLIST_STRIPE_COLOUR,	OptionGeneralPanel::OnClickColour		)
EVT_BUTTON				(MUSIK_PREFERENCES_ACTIVITY_STRIPE_COLOUR,	OptionGeneralPanel::OnClickColour		)
EVT_BUTTON				(MUSIK_PREFERENCES_SOURCES_STRIPE_COLOUR,	OptionGeneralPanel::OnClickColour		)
EVT_BUTTON				(MUSIK_PREFERENCES_PLAYLIST_BORDER_COLOUR,	OptionGeneralPanel::OnClickColour		)
END_EVENT_TABLE()

void OptionGeneralPanel::OnClickColour		( wxCommandEvent &event )
{
    //--- show the standard color dialog to change the background color of the button ---//
    wxButton_NoFlicker* Button = (wxButton_NoFlicker*)event.GetEventObject();
    Button->SetBackgroundColour( wxGetColourFromUser( this, Button->GetBackgroundColour() ) );
}

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
    PREF_CREATE_CHECKBOX(BlankSwears,_("Censor common swearwords"));

    PREF_CREATE_CHECKBOX_EX(SortArtistWithoutPrefix,_("Sort Artist without prefix"),&snkPlaylistActivityNeedUpdate);
    PREF_CREATE_CHECKBOX_EX(PLStripes,_("Show \"stripes\" in playlist"),&snkPlaylistNeedUpdate);
    PREF_CREATE_CHECKBOX_EX(ActStripes,_("Show \"stripes\" in selection boxes"),&snkActivityNeedUpdate);
    PREF_CREATE_CHECKBOX_EX(SourcesStripes,_("Show \"stripes\" in sources box"),&snkSourcesNeedUpdate);
    PREF_CREATE_CHECKBOX_EX(PlaylistBorder,_("Use selected border colour"),&snkPlaylistSourcesNeedUpdate);

    btnPlaylistStripeColour	=	new wxButton_NoFlicker( this, MUSIK_PREFERENCES_PLAYLIST_STRIPE_COLOUR,	_("Set Color") );
    btnActivityStripeColour	=	new wxButton_NoFlicker( this, MUSIK_PREFERENCES_ACTIVITY_STRIPE_COLOUR,	_("Set Color") );
    btnSourcesStripeColour	=	new wxButton_NoFlicker( this, MUSIK_PREFERENCES_SOURCES_STRIPE_COLOUR,	_("Set Color") );
    btnPlaylistBorderColour =   new wxButton_NoFlicker( this, MUSIK_PREFERENCES_PLAYLIST_BORDER_COLOUR,	_("Set Color") );

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
    vsOptions_Interface->Add( chkBlankSwears,			0, wxALL, 4 );
    vsOptions_Interface->Add( chkSortArtistWithoutPrefix,0, wxALL, 4 );
    vsOptions_Interface->Add( chkPLStripes,		0, wxALL, 4 );
    vsOptions_Interface->Add( btnPlaylistStripeColour,	0, wxALL, 4 );
    vsOptions_Interface->Add( chkActStripes,	0, wxALL, 4 );
    vsOptions_Interface->Add( btnActivityStripeColour,	0, wxALL, 4 );
    vsOptions_Interface->Add( chkSourcesStripes,		0, wxALL, 4 );
    vsOptions_Interface->Add( btnSourcesStripeColour,	0, wxALL, 4 );
    vsOptions_Interface->Add( chkPlaylistBorder,		0, wxALL, 4 );
    vsOptions_Interface->Add( btnPlaylistBorderColour,	0, wxALL, 4 );

    wxBoxSizer *hsDBCacheSize = new wxBoxSizer( wxHORIZONTAL );
    hsDBCacheSize->Add( PREF_STATICTEXT(_("Database Cache Size:")), 0, wxRIGHT | wxALIGN_CENTER_VERTICAL, 2 );
    hsDBCacheSize->Add( scDBCacheSize, 0, 0, 0 );

    vsOptions_Interface->Add( hsDBCacheSize,0, wxALL, 2 );

    
    return vsOptions_Interface;
}

void OptionGeneralPanel::DoLoadPrefs()
{
    btnPlaylistStripeColour->SetBackgroundColour( StringToColour( wxGetApp().Prefs.sPLStripeColour ) );
    btnActivityStripeColour->SetBackgroundColour( StringToColour( wxGetApp().Prefs.sActStripeColour ) );
    btnSourcesStripeColour->SetBackgroundColour( StringToColour( wxGetApp().Prefs.sSourcesStripeColour ) );
    btnPlaylistBorderColour->SetBackgroundColour( StringToColour( wxGetApp().Prefs.sPlaylistBorderColour ) );
}
bool OptionGeneralPanel::DoSavePrefs()
{
    if ( ColourToString( btnPlaylistStripeColour->GetBackgroundColour() ) != wxGetApp().Prefs.sPLStripeColour )
    {
        wxGetApp().Prefs.sPLStripeColour = ColourToString( btnPlaylistStripeColour->GetBackgroundColour() );
        OptionGeneralPanel::NeedUpdateSink::m_bPlaylistUpdate = true;
    }
    if ( ColourToString( btnPlaylistBorderColour->GetBackgroundColour() ) != wxGetApp().Prefs.sPlaylistBorderColour )
    {
        wxGetApp().Prefs.sPlaylistBorderColour = ColourToString( btnPlaylistBorderColour->GetBackgroundColour() );
        OptionGeneralPanel::NeedUpdateSink::m_bPlaylistUpdate = true;
        OptionGeneralPanel::NeedUpdateSink::m_bSourcesUpdate = true;
    }


    if ( ColourToString( btnActivityStripeColour->GetBackgroundColour() ) != wxGetApp().Prefs.sActStripeColour )
    {
        wxGetApp().Prefs.sActStripeColour = ColourToString( btnActivityStripeColour->GetBackgroundColour() );
        OptionGeneralPanel::NeedUpdateSink::m_bActivityUpdate = true;
    }


    if ( ColourToString( btnSourcesStripeColour->GetBackgroundColour() ) != wxGetApp().Prefs.sSourcesStripeColour )
    {
        wxGetApp().Prefs.sSourcesStripeColour = ColourToString( btnSourcesStripeColour->GetBackgroundColour() );
        OptionGeneralPanel::NeedUpdateSink::m_bSourcesUpdate = true;
    }
    return true;
}

void OptionGeneralPanel::AfterDataTransferredFromWindow()
{
    if ( OptionGeneralPanel::NeedUpdateSink::m_bPlaylistUpdate )
        g_PlaylistBox->Update();
    if ( OptionGeneralPanel::NeedUpdateSink::m_bSourcesUpdate )
        g_SourcesCtrl->Update();
    if ( OptionGeneralPanel::NeedUpdateSink::m_bActivityUpdate )
        g_ActivityAreaCtrl->ReloadAllContents();

    if(OptionGeneralPanel::NeedUpdateSink::m_bSourcesUpdate 
        || OptionGeneralPanel::NeedUpdateSink::m_bActivityUpdate 
        || OptionGeneralPanel::NeedUpdateSink::m_bPlaylistUpdate)
    {
        // force update of everything
        g_MusikFrame->SendSizeEvent();
    }

}
void OptionGeneralPanel::DbChacheSizeSink::OnValueChange(wxGenericValidator *,wxWindowBase *)
{
    wxGetApp().Library.SetCacheSize(wxGetApp().Prefs.nDBCacheSize);
}


bool OptionGeneralPanel::NeedUpdateSink::m_bPlaylistUpdate;
bool OptionGeneralPanel::NeedUpdateSink::m_bActivityUpdate;
bool OptionGeneralPanel::NeedUpdateSink::m_bSourcesUpdate;
