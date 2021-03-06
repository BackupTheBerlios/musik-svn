
/*
*  StreamingProxyServerPanel.cpp
*
*  StreamingProxyServerPanel 
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

#include "StreamingProxyServerPanel.h"

IMPLEMENT_PREFPANEL_CONSTRUCTOR(StreamingProxyServerPanel)

wxSizer * StreamingProxyServerPanel::CreateControls()
{
    //--------------------------------//
    //--- Streaming -> ProxyServer ---//
    //--------------------------------//
    chkUseProxyServer		= new wxCheckBox_NoFlicker	( this, -1,	_("Use Proxy server") );

    PREF_CREATE_TEXTCTRL2(ProxyServer,wxFILTER_NONE);
    PREF_CREATE_TEXTCTRL2(ProxyServerPort,wxFILTER_NUMERIC);
    PREF_CREATE_TEXTCTRL2(ProxyServerUser,wxFILTER_NONE);
    PREF_CREATE_TEXTCTRL2(ProxyServerPassword,wxFILTER_NONE);

 
    wxFlexGridSizer * fsProxySizer		= new wxFlexGridSizer( 2,2,2 );
    fsProxySizer->AddGrowableCol(1);
    fsProxySizer->Add(PREF_STATICTEXT(_("Proxy server address:")),0,wxALIGN_CENTER_VERTICAL);
    fsProxySizer->Add(tcProxyServer,0,wxEXPAND);
    fsProxySizer->Add(PREF_STATICTEXT(_("Proxy server port:")),0,wxALIGN_CENTER_VERTICAL);
    fsProxySizer->Add(tcProxyServerPort,0,wxEXPAND);
    fsProxySizer->Add(PREF_STATICTEXT(_("Proxy server user:")),0,wxALIGN_CENTER_VERTICAL);
    fsProxySizer->Add(tcProxyServerUser,0,wxEXPAND);
    fsProxySizer->Add(PREF_STATICTEXT(_("Proxy server password:")),0,wxALIGN_CENTER_VERTICAL);
    fsProxySizer->Add(tcProxyServerPassword,0,wxEXPAND);

    wxBoxSizer *vsStreaming_ProxyServer	= new wxBoxSizer( wxVERTICAL );

    vsStreaming_ProxyServer->Add( chkUseProxyServer,		0, wxALL, 2 );
    vsStreaming_ProxyServer->Add( fsProxySizer,		0, wxALL | wxEXPAND, 4 );
    return vsStreaming_ProxyServer;

}
void StreamingProxyServerPanel::AfterDataTransferredFromWindow()
{

    wxGetApp().Player.InitFMOD_ProxyServer();
}
