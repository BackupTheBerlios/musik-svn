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
#include "myprec.h"
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif 

#include "OptionPlaylistPanel.h"
#include "MusikGlobals.h"
#include "Classes/PlaylistCtrl.h"

IMPLEMENT_PREFPANEL_CONSTRUCTOR(OptionPlaylistPanel)

wxSizer * OptionPlaylistPanel::CreateControls()
{
    //---------------------------------//
    //--- Options -> Playlist Sizer ---//
    //---------------------------------//
    wxBoxSizer *vsOptions_Playlist = new wxBoxSizer( wxVERTICAL );
    wxFlexGridSizer *vsOptions_Playlist_Columns = new wxFlexGridSizer( 4, 0, 10 );
    vsOptions_Playlist->Add(vsOptions_Playlist_Columns);
    //---------------------------//
    //--- Options -> Playlist ---//
    //---------------------------//
    const wxString choicesCMBStatic[] = {  _("Static"),	 _("Dynamic") };

    for(size_t i = 0; i < PlaylistColumn::NCOLUMNS; i++)
    {
        chkPLColumnEnable[i] = new  wxCheckBox_NoFlicker(	this, -1, wxString(wxGetTranslation(g_PlaylistColumn[i].Label))+wxT(":"));
        vsOptions_Playlist_Columns->Add(chkPLColumnEnable[i],		0, wxALIGN_BOTTOM);
        rdbPLColumnStatic[i] = new wxRadioBox( this, -1, wxT(""), wxDefaultPosition, wxDefaultSize, WXSIZEOF(choicesCMBStatic), choicesCMBStatic, 2, wxRA_SPECIFY_COLS | wxBORDER_NONE);
        vsOptions_Playlist_Columns->Add( rdbPLColumnStatic[i],		0 ,wxALIGN_TOP);
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
    for(size_t i = 0 ;i < PlaylistColumn::NCOLUMNS; i ++)
    {
        chkPLColumnEnable[i]->SetValue			( wxGetApp().Prefs.bPlaylistColumnEnable[i]);
        rdbPLColumnStatic[i]->SetSelection		( wxGetApp().Prefs.bPlaylistColumnDynamic[i]);
    }

}

bool OptionPlaylistPanel::DoSavePrefs()
{
    //---------------------------//
    //--- options -> playlist ---//
    //---------------------------//
    bool bResetColumns = false;
    for(size_t i = 0 ;i < PlaylistColumn::NCOLUMNS; i ++)
    {
        //--- enable / disable ---//
        if ( wxGetApp().Prefs.bPlaylistColumnEnable[i] != chkPLColumnEnable[i]->GetValue() )
        {
            wxGetApp().Prefs.bPlaylistColumnEnable[i] = chkPLColumnEnable[i]->GetValue();
            bResetColumns = true;
        }
        //--- standard / dynamic ---//
        if ( wxGetApp().Prefs.bPlaylistColumnDynamic[i] != (rdbPLColumnStatic[i]->GetSelection() ? true : false) )
        {
            wxGetApp().Prefs.bPlaylistColumnDynamic[i] = rdbPLColumnStatic[i]->GetSelection()? true : false;
            bResetColumns = true;
        }
    }
    if ( bResetColumns )
        g_PlaylistBox->PlaylistCtrl().ResetColumns( true, true );
    return true;
}
