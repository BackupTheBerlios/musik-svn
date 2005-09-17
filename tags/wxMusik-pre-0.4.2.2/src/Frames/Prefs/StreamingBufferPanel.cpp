/*
*  StreamingBufferPanel.cpp
*
*  StreamingBufferPanel 
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

#include "StreamingBufferPanel.h"

IMPLEMENT_PREFPANEL_CONSTRUCTOR(StreamingBufferPanel)

wxSizer * StreamingBufferPanel::CreateControls()
{
    //---------------------------//
    //--- Streaming -> Buffer ---//
    //---------------------------//


    PREF_CREATE_SPINCTRL_EX(StreamingBufferSize,64000,256000,&snkNBSC);
    PREF_CREATE_SPINCTRL_EX(StreamingPreBufferPercent,10,99,&snkNBSC);
    PREF_CREATE_SPINCTRL_EX(StreamingReBufferPercent,10,99,&snkNBSC);

    wxFlexGridSizer *vsStreaming_Buffer = new wxFlexGridSizer( 2,2,2 );
    vsStreaming_Buffer->AddGrowableCol(1);
    vsStreaming_Buffer->Add( PREF_STATICTEXT( _("Buffer Size (bytes):")), 0, wxRIGHT | wxALIGN_CENTER_VERTICAL, 2 );
    vsStreaming_Buffer->Add( scStreamingBufferSize,0,wxEXPAND);
    vsStreaming_Buffer->Add( PREF_STATICTEXT( _("Prebuffering (%):")) , 0, wxRIGHT | wxALIGN_CENTER_VERTICAL, 2 );
    vsStreaming_Buffer->Add( scStreamingPreBufferPercent,0,wxEXPAND);
    vsStreaming_Buffer->Add( PREF_STATICTEXT(_("Rebuffering (%):")), 0, wxRIGHT | wxALIGN_CENTER_VERTICAL, 2  );
    vsStreaming_Buffer->Add( scStreamingReBufferPercent,0,wxEXPAND);
    return vsStreaming_Buffer;
}
void StreamingBufferPanel::AfterDataTransferredFromWindow()
{
   
    if(snkNBSC.m_bNetBufferSettingChanged)
    {
        wxGetApp().Player.Init_NetBuffer();
    }
}
