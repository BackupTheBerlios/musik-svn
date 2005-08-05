/*
*  OptionTunagePanel.cpp
*
*  OptionTunagePanel 
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

#include "OptionTunagePanel.h"

IMPLEMENT_PREFPANEL_CONSTRUCTOR(OptionTunagePanel)

wxSizer * OptionTunagePanel::CreateControls()
{
    //-------------------------//
    //--- options -> tunage ---//
    //-------------------------//
    // IMPORTANT! Create wxStaticBox BEFORE creation of
    // controls which should be placed inside
    // or else they wont be displayed on wxGTK		
    wxStaticBox *sbTunageFile = new wxStaticBox( this, -1, _("Write to file") );
    wxStaticBox *sbTunageURL = new wxStaticBox( this, -1, _("Post to a website") );
    wxStaticBox *sbTunageApp = new wxStaticBox( this, -1, _("Run a program") );
    wxStaticBox *sbTunageMisc = new wxStaticBox( this, -1, _("Misc.") );

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

    wxBoxSizer *vsOptions_Tunage = new wxBoxSizer( wxVERTICAL );
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
    return vsOptions_Tunage;

}
