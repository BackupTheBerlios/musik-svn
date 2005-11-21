
/*
*  WebServerPanel.cpp
*
*  WebServerPanel 
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

#include "WebServerPanel.h"

IMPLEMENT_PREFPANEL_CONSTRUCTOR(WebServerPanel)

wxSizer * WebServerPanel::CreateControls()
{
    //--------------------------------//
    //--- Streaming -> ProxyServer ---//
    //--------------------------------//
    PREF_CREATE_CHECKBOX(WebServerEnable,_("Enable Webserver") );
    PREF_CREATE_SPINCTRL(WebServerPort,80,0xffff,1024);
    PREF_CREATE_SPINCTRL(WebServerRefresh,1,600,5);

 
    wxFlexGridSizer * fsSizer		= new wxFlexGridSizer( 2,2,2 );
    fsSizer->AddGrowableCol(1);
    fsSizer->Add(PREF_STATICTEXT(_("Webserver port:")),0,wxALIGN_CENTER_VERTICAL);
    fsSizer->Add(scWebServerPort,0,wxEXPAND);
    fsSizer->Add(PREF_STATICTEXT(_("Refresh Rate(s):")),0,wxALIGN_CENTER_VERTICAL);
    fsSizer->Add(scWebServerRefresh,0,wxEXPAND);

    wxBoxSizer *vsBoxVert	= new wxBoxSizer( wxVERTICAL );

    vsBoxVert->Add( chkWebServerEnable,		0, wxALL, 2 );
    vsBoxVert->Add( fsSizer,		0, wxALL | wxEXPAND, 4 );
    return vsBoxVert;

}
void WebServerPanel::AfterDataTransferredFromWindow()
{
    if(wxGetApp().Prefs.bWebServerEnable)
        wxGetApp().WebServer.Start(wxGetApp().Prefs.nWebServerPort);
    else
        wxGetApp().WebServer.Stop();
}
