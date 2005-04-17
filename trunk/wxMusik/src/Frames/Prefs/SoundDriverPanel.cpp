
/*
*  SoundDriverPanel.cpp
*
*  SoundDriverPanel 
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

#include "SoundDriverPanel.h"
#include "Classes/MusikPlayer.h"

#define MUSIK_PREFERENCES_OUTPUT_DRV 1

BEGIN_EVENT_TABLE(SoundDriverPanel, PrefPanel)
EVT_COMBOBOX			(MUSIK_PREFERENCES_OUTPUT_DRV,				SoundDriverPanel::OnOutputChanged	)
END_EVENT_TABLE()

IMPLEMENT_PREFPANEL_CONSTRUCTOR(SoundDriverPanel)

wxSizer * SoundDriverPanel::CreateControls()
{
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

    cmbSndDevice	= new wxComboBox ( this,-1 , wxT(""), wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_READONLY );	
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
    fsSound_Driver->Add( scSndMaxChan, 1, 0, 0 );

    fsSound_Driver->AddGrowableCol(1);
    wxBoxSizer *vsSound_Driver = new wxBoxSizer( wxVERTICAL );
    vsSound_Driver->Add( fsSound_Driver,	0, wxALL, 4  );
    vsSound_Driver->Add( chkUse_MPEGACCURATE_ForMP3VBRFiles );
    return vsSound_Driver;

}
void SoundDriverPanel::DoLoadPrefs()
{
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


}
void SoundDriverPanel::FindDevices()
{
    cmbSndDevice->Clear();
    for ( int i = 0; i < FSOUND_GetNumDrivers(); i++ )
        cmbSndDevice->Append( ConvA2W( FSOUND_GetDriverName( i ) ) );
    if ( cmbSndDevice->GetCount() < 1 )
        cmbSndDevice->Append( _("[No Devices]") );
    cmbSndDevice->SetSelection( 0 );
}

bool SoundDriverPanel::DoSavePrefs()
{

    bool bRestartFMOD		= false;
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
    if ( scSndMaxChan->GetValue() != wxGetApp().Prefs.nSndMaxChan )
    {
        bRestartFMOD = true;
    }
    double fLength = StringToDouble( tcBufferLength->GetValue() );
    int nLength = ( int )( fLength * 1000 );
    wxGetApp().Prefs.nSndBuffer = nLength;

    if ( bRestartFMOD )
        wxGetApp().Player.InitializeFMOD( FMOD_INIT_RESTART );
    return true;
}
