/*
*  OptionPlaylistPanel.cpp
*
*  OptionPlaylistPanel 
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

#include "OptionPlaylistPanel.h"
#include "MusikGlobals.h"

IMPLEMENT_PREFPANEL_CONSTRUCTOR(OptionPlaylistPanel)

wxSizer * OptionPlaylistPanel::CreateControls()
{
    //---------------------------------//
    //--- Options -> Playlist Sizer ---//
    //---------------------------------//
    wxBoxSizer *vsOptions_Playlist = new wxBoxSizer( wxVERTICAL );

    wxFlexGridSizer *vsOptions_Playlist_Columns = new wxFlexGridSizer( NPLAYLISTCOLUMNS, 2, 2, 2 );
    vsOptions_Playlist->Add(vsOptions_Playlist_Columns);
    //---------------------------//
    //--- Options -> Playlist ---//
    //---------------------------//
    const wxString choicesCMBStatic[] = {  _("Static"),	 _("Dynamic") };

    for(size_t i = 0; i < NPLAYLISTCOLUMNS; i++)
    {
        chkPLColumnEnable[i] = new  wxCheckBox_NoFlicker(	this, -1, wxString(wxGetTranslation(g_PlaylistColumnLabels[i]))+wxT(":"));
        vsOptions_Playlist_Columns->Add(chkPLColumnEnable[i],		0, wxALIGN_CENTER_VERTICAL | wxADJUST_MINSIZE );
        cmbPLColumnStatic[i] = new wxComboBox( this, -1, wxT(""), wxDefaultPosition, wxDefaultSize, WXSIZEOF(choicesCMBStatic), choicesCMBStatic, wxCB_READONLY );
        vsOptions_Playlist_Columns->Add( cmbPLColumnStatic[i],		0 );
    }
    PREF_CREATE_CHECKBOX(DisplayEmptyPlaylistColumnAsUnkown,_("Display <unknown> in empty colums"));
    vsOptions_Playlist->Add( chkDisplayEmptyPlaylistColumnAsUnkown,		0, wxTOP, 5 );
    return vsOptions_Playlist;
}

void OptionPlaylistPanel::DoLoadPrefs()
{
    //---------------------------//
    //--- options -> playlist ---//
    //---------------------------//
    for(size_t i = 0 ;i < NPLAYLISTCOLUMNS; i ++)
    {
        chkPLColumnEnable[i]->SetValue			( wxGetApp().Prefs.bPlaylistColumnEnable[i]);
        cmbPLColumnStatic[i]->SetSelection		( wxGetApp().Prefs.bPlaylistColumnDynamic[i]);
    }

}

bool OptionPlaylistPanel::DoSavePrefs()
{
    //---------------------------//
    //--- options -> playlist ---//
    //---------------------------//
    bool bResetColumns = false;
    for(size_t i = 0 ;i < NPLAYLISTCOLUMNS; i ++)
    {
        //--- enable / disable ---//
        if ( wxGetApp().Prefs.bPlaylistColumnEnable[i] != chkPLColumnEnable[i]->GetValue() )
        {
            wxGetApp().Prefs.bPlaylistColumnEnable[i] = chkPLColumnEnable[i]->GetValue();
            bResetColumns = true;
        }
        //--- standard / dynamic ---//
        if ( wxGetApp().Prefs.bPlaylistColumnDynamic[i] != (cmbPLColumnStatic[i]->GetSelection() ? true : false) )
        {
            wxGetApp().Prefs.bPlaylistColumnDynamic[i] = cmbPLColumnStatic[i]->GetSelection()? true : false;
            bResetColumns = true;
        }
    }
    if ( bResetColumns )
        g_PlaylistBox->PlaylistCtrl().ResetColumns( true, true );
    return true;
}
