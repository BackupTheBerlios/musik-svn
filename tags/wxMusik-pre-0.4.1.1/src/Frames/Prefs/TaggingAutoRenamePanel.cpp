/*
*  TaggingAutoRenamePanel.cpp
*
*  TaggingAutoRenamePanel 
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

#include "TaggingAutoRenamePanel.h"

IMPLEMENT_PREFPANEL_CONSTRUCTOR(TaggingAutoRenamePanel)


wxSizer * TaggingAutoRenamePanel::CreateControls()
{
    //---------------------------//
    //--- Tagging -> Auto Tag ---//
    //---------------------------//
    //--- rename options and sizer ---//
    PREF_CREATE_TEXTCTRL(  AutoRename,wxFILTER_NONE);
    wxBoxSizer *hsRename	= new wxBoxSizer	( wxHORIZONTAL );
    hsRename->Add ( PREF_STATICTEXT( _("Rename:")), 0, wxRIGHT | wxALIGN_CENTER_VERTICAL, 2 );
    hsRename->Add ( tcAutoRename, 1, wxEXPAND, 0 );
    //--- information ---//
    wxString sRenameInfo = 
        wxString( _("\nAuto Rename syntax:\n\n") ) + 
        wxString(wxFileName::GetPathSeparator()) + _( " - Directory Separator\n" )  +
        wxString( _("%1 - Song Title\n")				) + 
        wxString( _("%2 - Artist Name\n")				) +
        wxString( _("%3 - Album Name\n")				) +
        wxString( _("%4 - Genre\n")						) +
        wxString( _("%5 - Year\n")						) +
        wxString( _("%6 - Track Number\n\n")			) +
        wxString(MUSIKAPPNAME)							  +	
        wxString( _(" will not delete empty directories!" ) );

    //---------------------------------//
    //--- Tagging -> Auto Tag Sizer ---//
    //---------------------------------//
    wxBoxSizer *vsTagging_Auto = new wxBoxSizer( wxVERTICAL );
    vsTagging_Auto->Add	( hsRename, 0, wxALL | wxEXPAND, 4 );
    vsTagging_Auto->Add ( PREF_STATICTEXT(sRenameInfo), 0, wxADJUST_MINSIZE | wxALL | wxEXPAND, 4 );
    return vsTagging_Auto;
}
