
/*
*  OptionAutoDJPanel.cpp
*
*  OptionAutoDJPanel 
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

#include "OptionAutoDJPanel.h"

IMPLEMENT_PREFPANEL_CONSTRUCTOR(OptionAutoDJPanel)

wxSizer * OptionAutoDJPanel::CreateControls()
{
    //-------------------------//
    //--- options -> Auto DJ ---//
    //-------------------------//
    wxStaticBox *sbAutoDj = new wxStaticBox( this, -1, _("Auto DJ") );// IMPORTANT! Create wxStaticBox BEFORE creation of
    // controls which should be placed inside
    // or else they wont be displayed on wxGTK						

    tcAutoDjFilter = new wxTextCtrl_NoFlicker( this, -1,wxEmptyString,wxDefaultPosition,wxSize(-1,80),wxTE_MULTILINE);

    PREF_CREATE_SPINCTRL(AutoDjDoNotPlaySongPlayedTheLastNHours,1,2000000,1);
    PREF_CREATE_SPINCTRL(AutoDJChooseSongsToPlayInAdvance,1,1000,1);
    PREF_CREATE_SPINCTRL(AutoDJChooseAlbumsToPlayInAdvance,1,20,1);
    wxBoxSizer *hsAutoDjFilter = new wxBoxSizer( wxVERTICAL );
    hsAutoDjFilter->Add( PREF_STATICTEXT( _("Use this filter to select songs:")), 0, wxRIGHT | wxALIGN_CENTER_VERTICAL, 2 );
    hsAutoDjFilter->Add( tcAutoDjFilter, 1, wxEXPAND, 0 );
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


    wxStaticBox *sbShuffle =	 new wxStaticBox( this, -1, _("Shuffle") );
    PREF_CREATE_SPINCTRL(MaxShuffleHistory,0,10000,0);
    wxBoxSizer *hsMaxShuffleHistory = new wxBoxSizer( wxHORIZONTAL );
    hsMaxShuffleHistory->Add( PREF_STATICTEXT(_("Shuffle history size:")), 0, wxRIGHT | wxALIGN_CENTER_VERTICAL, 2 );
    hsMaxShuffleHistory->Add( scMaxShuffleHistory, 1, 0, 0 );


    wxStaticBoxSizer *vsShuffle = new wxStaticBoxSizer( sbShuffle, wxVERTICAL );
    vsShuffle->Add( hsMaxShuffleHistory,								0, wxALL | wxEXPAND, 2 );

    wxBoxSizer *vsOptions_AutoDj = new wxBoxSizer( wxVERTICAL );

    vsOptions_AutoDj->Add( vsShuffle,		0, wxALL | wxEXPAND, 2 );
    vsOptions_AutoDj->Add( vsAutoDj,		0, wxALL | wxEXPAND, 2 );
    return vsOptions_AutoDj;
}

void OptionAutoDJPanel::DoLoadPrefs()
{
    tcAutoDjFilter->SetValue(wxGetApp().Prefs.sAutoDjFilter);    
}
bool OptionAutoDJPanel::DoSavePrefs()
{
    //-------------------------//
    //--- options -> Auto DJ ---//
    //-------------------------//
    if ( tcAutoDjFilter->GetValue()  != wxGetApp().Prefs.sAutoDjFilter )
    {
        wxGetApp().Prefs.sAutoDjFilter = tcAutoDjFilter->GetValue(); 
        if(!wxGetApp().Library.SetAutoDjFilter(wxGetApp().Prefs.sAutoDjFilter))
        {
            wxMessageBox( _( "An error occured when setting the Auto DJ Filter" ), MUSIKAPPNAME_VERSION, wxOK | wxICON_ERROR );
        }
    }
    return true;
}
