/*
*  OptionSelectionsPanel.cpp
*
*  OptionSelectionsPanel 
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

#include "OptionSelectionsPanel.h"
#include "MusikGlobals.h"


IMPLEMENT_PREFPANEL_CONSTRUCTOR(OptionSelectionsPanel)

wxSizer * OptionSelectionsPanel::CreateControls()
{
    //-----------------------------------//
    //--- Options -> Selections Sizer ---//
    //-----------------------------------//
    wxFlexGridSizer *vsOptions_Selections = new wxFlexGridSizer ( 4, 2, 2, 2 );

    //----------------------------//
    //--- Options -> Selection ---//
    //----------------------------//
    const wxString arrSelectionBoxes[] = 
    {
        _("None")  , 
            _("Artist"),
            _("Album") ,
            _("Genre") ,
            _("Year")  
    };

    //--- activity boxes ---//
    for(size_t i = 0; i < WXSIZEOF(cmbActivityBoxes);i++)
    {
        vsOptions_Selections->Add(  PREF_STATICTEXT( wxString::Format(_("Selection Box %d:"),i + 1)),	0, wxCENTER | wxRIGHT | wxALIGN_CENTER_VERTICAL, 0 );
        cmbActivityBoxes[i] = new wxComboBox( this, -1, wxEmptyString, wxDefaultPosition, wxDefaultSize, WXSIZEOF(arrSelectionBoxes), arrSelectionBoxes, wxCB_READONLY );
        vsOptions_Selections->Add( cmbActivityBoxes[i],	1, wxCENTER, 0 );
    }
    //--- selection style ---//
    cmbSelStyle = new wxComboBox( this, -1, wxT(""), wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_READONLY );
    cmbSelStyle->Append( _( "Standard" ) );
    cmbSelStyle->Append( _( "Sloppy" ) );
    cmbSelStyle->Append( _( "Highlight") );

    vsOptions_Selections->Add( PREF_STATICTEXT(_("Selection Style:" )),		0, wxCENTER | wxRIGHT | wxALIGN_CENTER_VERTICAL, 0 );
    vsOptions_Selections->Add( cmbSelStyle,		1, wxCENTER, 0 );
    return vsOptions_Selections;

}

void OptionSelectionsPanel::DoLoadPrefs()
{
    //-----------------------------//
    //--- options -> selections ---//
    //-----------------------------//
    cmbSelStyle->SetSelection		( wxGetApp().Prefs.eSelStyle.val );
    for(size_t i = 0; i < WXSIZEOF(cmbActivityBoxes);i++)
        cmbActivityBoxes[i]->SetSelection	( wxGetApp().Prefs.nActBoxType[i] );

}

bool OptionSelectionsPanel::DoSavePrefs()
{
    //-----------------------------//
    //--- Options -> selections ---//
    //-----------------------------//
    if ( (EMUSIK_ACTIVITY_SELECTION_TYPE)cmbSelStyle->GetSelection() != wxGetApp().Prefs.eSelStyle )
    {
        wxGetApp().Prefs.eSelStyle = (EMUSIK_ACTIVITY_SELECTION_TYPE)cmbSelStyle->GetSelection();
        g_ActivityAreaCtrl->ResetAllContents();
    }
    for(size_t i = 0; i < WXSIZEOF(cmbActivityBoxes);i++)
    {
        if ( cmbActivityBoxes[i]->GetSelection() != wxGetApp().Prefs.nActBoxType[i] )
        {
            wxGetApp().Prefs.nActBoxType[i] = (EMUSIK_ACTIVITY_TYPE)cmbActivityBoxes[i]->GetSelection();

            g_ActivityAreaCtrl->Delete();
            g_ActivityAreaCtrl->Create();
            g_MusikFrame->Layout();
        }
    }
    return true;
}
