/*
*  TaggingGeneralPanel.cpp
*
*  TaggingGeneralPanel 
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

#include "TaggingGeneralPanel.h"

IMPLEMENT_PREFPANEL_CONSTRUCTOR(TaggingGeneralPanel)

wxSizer * TaggingGeneralPanel::CreateControls()
{
    //--------------------------//
    //--- Tagging -> General ---//
    //--------------------------//
    PREF_CREATE_CHECKBOX(ActBoxWrite,_("Write tag to file"));
    PREF_CREATE_CHECKBOX(ActBoxClear,_("Clear old tag"));
    PREF_CREATE_CHECKBOX(ActBoxRename,_("Automatically rename file"));
    PREF_CREATE_CHECKBOX(TagDlgWrite,_("Write tag to file"));
    PREF_CREATE_CHECKBOX(TagDlgClear,_("Clear old tag"));
    PREF_CREATE_CHECKBOX(TagDlgRename,_("Automatically rename file"));
    //--------------------------------//
    //--- Tagging -> General Sizer ---//
    //--------------------------------//
    wxBoxSizer *vsTagging_General = new wxBoxSizer( wxVERTICAL );
    vsTagging_General->Add	( PREF_STATICTEXT( _("Selection Boxes (artist/album/etc):")),		0, wxALL | wxEXPAND, 4 );
    vsTagging_General->Add	( chkActBoxWrite,		0, wxALL | wxEXPAND, 4 );
    vsTagging_General->Add	( chkActBoxClear,		0, wxALL | wxEXPAND, 4 );
    vsTagging_General->Add	( chkActBoxRename,	0, wxALL | wxEXPAND, 4 );
    vsTagging_General->Add	( PREF_STATICTEXT(_("\nTag Dialog Box:")),			0, wxALL | wxEXPAND, 4 );
    vsTagging_General->Add	( chkTagDlgWrite,		0, wxALL | wxEXPAND, 4 );
    vsTagging_General->Add	( chkTagDlgClear,		0, wxALL | wxEXPAND, 4 );
    vsTagging_General->Add	( chkTagDlgRename,		0, wxALL | wxEXPAND, 4 );
    return vsTagging_General;

}  
