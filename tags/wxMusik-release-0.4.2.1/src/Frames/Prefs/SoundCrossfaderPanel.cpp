
/*
*  SoundCrossfaderPanel.cpp
*
*  SoundCrossfaderPanel 
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

#include "SoundCrossfaderPanel.h"

IMPLEMENT_PREFPANEL_CONSTRUCTOR(SoundCrossfaderPanel)

wxSizer * SoundCrossfaderPanel::CreateControls()
{
    //--------------------------//
    //--- Sound -> Crossfader ---//
    //---------------------------//
    PREF_CREATE_CHECKBOX(FadeEnable, _("Crossfade on new song (seconds)"));
    PREF_CREATE_CHECKBOX(FadeSeekEnable, _("Crossfade on track seek (seconds)"));
    PREF_CREATE_CHECKBOX(FadePauseResumeEnable, _("Crossfade on pause or resume (seconds)"));
    PREF_CREATE_CHECKBOX(FadeStopEnable, _("Crossfade on stop (seconds)"));
    PREF_CREATE_CHECKBOX(FadeExitEnable, _("Crossfade on program exit (seconds)"));


    tcDuration 				= new wxTextCtrl_NoFlicker( this, -1);
    tcSeekDuration 			= new wxTextCtrl_NoFlicker( this, -1);
    tcPauseResumeDuration	= new wxTextCtrl_NoFlicker( this, -1);
    tcStopDuration			= new wxTextCtrl_NoFlicker( this, -1);
    tcExitDuration			= new wxTextCtrl_NoFlicker( this, -1);
    //--- crossfader sizer ---//
    wxFlexGridSizer	*fsCrossfader = new wxFlexGridSizer( 5, 2, 2, 2 );
    fsCrossfader->Add( chkFadeEnable				);
    fsCrossfader->Add( tcDuration 				);
    fsCrossfader->Add( chkFadeSeekEnable		);
    fsCrossfader->Add( tcSeekDuration		 	);
    fsCrossfader->Add( chkFadePauseResumeEnable	);
    fsCrossfader->Add( tcPauseResumeDuration	);
    fsCrossfader->Add( chkFadeStopEnable		);
    fsCrossfader->Add( tcStopDuration			);
    fsCrossfader->Add( chkFadeExitEnable		);
    fsCrossfader->Add( tcExitDuration			);
    return fsCrossfader;

}
void SoundCrossfaderPanel::DoLoadPrefs()
{


    float		fDuration;
    wxString	sDuration;

    fDuration =							(float)wxGetApp().Prefs.nFadeDuration / 1000;
    sDuration.sprintf					( wxT("%.1f"), fDuration );
    tcDuration->SetValue				( sDuration );

    fDuration =							(float)wxGetApp().Prefs.nFadeSeekDuration / 1000;
    sDuration.sprintf					( wxT("%.1f"), fDuration );
    tcSeekDuration->SetValue			( sDuration );

    fDuration =							(float)wxGetApp().Prefs.nFadePauseResumeDuration / 1000;
    sDuration.sprintf					( wxT("%.1f"), fDuration );
    tcPauseResumeDuration->SetValue		( sDuration );

    fDuration =							(float)wxGetApp().Prefs.nFadeStopDuration / 1000;
    sDuration.sprintf					( wxT("%.1f"), fDuration );
    tcStopDuration->SetValue			( sDuration );

    fDuration =							(float)wxGetApp().Prefs.nFadeExitDuration / 1000;
    sDuration.sprintf					( wxT("%.1f"), fDuration );
    tcExitDuration->SetValue			( sDuration );
}

bool SoundCrossfaderPanel::DoSavePrefs()
{
    double fDuration;
    int nDuration;

    fDuration = StringToDouble( tcDuration->GetValue() );
    nDuration = ( int )( fDuration * 1000 );
    wxGetApp().Prefs.nFadeDuration = nDuration;

    fDuration = StringToDouble( tcSeekDuration->GetValue() );
    nDuration = ( int )( fDuration * 1000 );
    wxGetApp().Prefs.nFadeSeekDuration = nDuration;

    fDuration = StringToDouble( tcPauseResumeDuration->GetValue() );
    nDuration = ( int )( fDuration * 1000 );
    wxGetApp().Prefs.nFadePauseResumeDuration = nDuration;

    fDuration = StringToDouble( tcStopDuration->GetValue() );
    nDuration = ( int )( fDuration * 1000 );
    wxGetApp().Prefs.nFadeStopDuration = nDuration;

    fDuration = StringToDouble( tcExitDuration->GetValue() );
    nDuration = ( int )( fDuration * 1000 );
    wxGetApp().Prefs.nFadeExitDuration = nDuration;
    return true;
}
