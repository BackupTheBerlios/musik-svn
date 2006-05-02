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
#include "myprec.h"
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif 

#include "OptionSelectionsPanel.h"
#include "MusikGlobals.h"
#include "PlaylistColumn.h"
#include "Classes/ActivityAreaCtrl.h" 

IMPLEMENT_PREFPANEL_CONSTRUCTOR(OptionSelectionsPanel)

struct ActivityBoxCBData: public wxClientData
{
    ActivityBoxCBData(PlaylistColumn::eId id)
        :m_id(id)
    {
    }

    PlaylistColumn::eId m_id;
};
wxSizer * OptionSelectionsPanel::CreateControls()
{
    //-----------------------------------//
    //--- Options -> Selections Sizer ---//
    //-----------------------------------//
    wxFlexGridSizer *vsOptions_Selections = new wxFlexGridSizer ( 4, 2, 2, 2 );

    //----------------------------//
    //--- Options -> Selection ---//
    //----------------------------//
 
    //--- activity boxes ---//
    for(size_t i = 0; i < WXSIZEOF(cmbActivityBoxes);i++)
    {
        vsOptions_Selections->Add(  PREF_STATICTEXT( wxString::Format(_("Selection Box %d:"),i + 1)),	0, wxCENTER | wxRIGHT | wxALIGN_CENTER_VERTICAL, 0 );
        cmbActivityBoxes[i] = new wxComboBox( this, -1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_READONLY );
        cmbActivityBoxes[i]->Append( _("None"),new ActivityBoxCBData(PlaylistColumn::INVALID));
        for(int n = 0; n < PlaylistColumn::NCOLUMNS;n++)
        {
            if(g_PlaylistColumn[n].bEnableActivityBox)
            {
                cmbActivityBoxes[i]->Append( wxGetTranslation( g_PlaylistColumn[n].Label ),new ActivityBoxCBData(g_PlaylistColumn[n].Id));
            }
        }
        vsOptions_Selections->Add( cmbActivityBoxes[i],	1, wxCENTER, 0 );
    }
    //--- selection style ---//
    cmbSelStyle = new wxComboBox( this, -1, wxT(""), wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_READONLY );
    cmbSelStyle->Append( _( "Standard" ) );
    cmbSelStyle->Append( _( "Sloppy" ) );
    cmbSelStyle->Append( _( "Highlight") );
    cmbSelStyle->Append( _( "Normal") );

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
    {
        for(size_t n = 0;n < cmbActivityBoxes[i]->GetCount();n++)
        {
            ActivityBoxCBData *pData = (ActivityBoxCBData*)cmbActivityBoxes[i]->GetClientObject(n);
            if(pData->m_id == wxGetApp().Prefs.nActBoxType[i])
            {
                cmbActivityBoxes[i]->SetSelection	( n );
                break;
            }
        }
    }
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
        ActivityBoxCBData *pData = (ActivityBoxCBData*)cmbActivityBoxes[i]->GetClientObject(cmbActivityBoxes[i]->GetSelection());
        if (  pData->m_id  != wxGetApp().Prefs.nActBoxType[i] )
        {
            wxGetApp().Prefs.nActBoxType[i] = pData->m_id;
            g_ActivityAreaCtrl->ReCreate();
            g_MusikFrame->Layout();
        }
    }
    return true;
}
