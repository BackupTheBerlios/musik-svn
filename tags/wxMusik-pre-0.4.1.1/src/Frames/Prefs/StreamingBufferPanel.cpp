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
#include "wx/wxprec.h"
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


    PREF_CREATE_SPINCTRL2(StreamingBufferSize,64000,256000,64000);
    PREF_CREATE_SPINCTRL2(StreamingPreBufferPercent,10,99,10);
    PREF_CREATE_SPINCTRL2(StreamingReBufferPercent,10,99,10);

    wxFlexGridSizer *vsStreaming_Buffer = new wxFlexGridSizer( 3,2,2,2 );

    vsStreaming_Buffer->Add( PREF_STATICTEXT( _("Buffer Size (bytes):")) );
    vsStreaming_Buffer->Add( scStreamingBufferSize );
    vsStreaming_Buffer->Add( PREF_STATICTEXT( _("Prebuffering (%):")) );
    vsStreaming_Buffer->Add( scStreamingPreBufferPercent );
    vsStreaming_Buffer->Add( PREF_STATICTEXT(_("Rebuffering (%):")) );
    vsStreaming_Buffer->Add( scStreamingReBufferPercent );
    return vsStreaming_Buffer;
}
bool StreamingBufferPanel::DoSavePrefs()
{
    //---------------------------//
    //--- streaming -> buffer ---//
    //---------------------------//
    bool bNetBufferSettingChanged = false;
    if (  scStreamingBufferSize->GetValue( )  != wxGetApp().Prefs.nStreamingBufferSize )
    {
        bNetBufferSettingChanged = true;
    }
    if ( scStreamingPreBufferPercent->GetValue( ) != wxGetApp().Prefs.nStreamingPreBufferPercent )
    {
        bNetBufferSettingChanged = true;
    }
    if ( scStreamingReBufferPercent->GetValue( ) != wxGetApp().Prefs.nStreamingReBufferPercent )
    {
        bNetBufferSettingChanged = true;
    }
    if(bNetBufferSettingChanged)
    {
        wxGetApp().Player.InitFMOD_NetBuffer();
    }
    return true;
}
