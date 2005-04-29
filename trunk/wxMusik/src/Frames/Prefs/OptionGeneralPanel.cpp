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
    vsOptions_Interface->Add( chkPlaylistStripes,		0, wxALL, 4 );
    vsOptions_Interface->Add( btnPlaylistStripeColour,	0, wxALL, 4 );
    vsOptions_Interface->Add( chkActivityBoxStripes,	0, wxALL, 4 );
    vsOptions_Interface->Add( btnActivityStripeColour,	0, wxALL, 4 );
    vsOptions_Interface->Add( chkSourcesBoxStripes,		0, wxALL, 4 );
    vsOptions_Interface->Add( btnSourcesStripeColour,	0, wxALL, 4 );
    vsOptions_Interface->Add( chkPlaylistBorder,		0, wxALL, 4 );
    vsOptions_Interface->Add( btnPlaylistBorderColour,	0, wxALL, 4 );
    return vsOptions_Interface;
}

void OptionGeneralPanel::DoLoadPrefs()
{
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
}

bool OptionGeneralPanel::DoSavePrefs()
{
    //--------------------------//
    //--- Options -> general ---//
    //--------------------------//

    bool bPlaylistUpdate = false;
    bool bActivityUpdate = false;
    bool bSourcesUpdate = false;

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

    if(bSourcesUpdate || bActivityUpdate || bPlaylistUpdate)
    {
        // force update of everything
        g_MusikFrame->SendSizeEvent();
    }
    return true;
}
