/*
*  OptionFileAssocPanel.cpp
*
*  OptionFileAssocPanel 
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

#ifdef __WXMSW__
#include <shlobj.h>
#endif
#include <wx/mimetype.h>

#include "OptionFileAssocPanel.h"
#include "Classes/Library/MetaDataHandler.h"

IMPLEMENT_PREFPANEL_CONSTRUCTOR(OptionFileAssocPanel)
wxSizer * OptionFileAssocPanel::CreateControls()
{
    //------------------------------------//
    //--- options -> file associations ---//
    //------------------------------------//
    wxBoxSizer *vsOptions_FileAssoc = new wxBoxSizer( wxVERTICAL);

    chklbFileAssocs = new wxCheckListBox(this,-1);
    vsOptions_FileAssoc->Add(chklbFileAssocs,1,wxEXPAND|wxALL,5);

    for(int i = 0; i < COUNT_MUSIK_FORMAT ; i++)
    {
        const tSongClass *pSongClass = CMetaDataHandler::GetSongClass((EMUSIK_FORMAT_TYPE)i);
        if(pSongClass)
        {
            bool bRegisterdForwxMusik = false;
            wxString sDesc = wxGetTranslation(pSongClass->szDescription);
            GetFileTypeAssociationInfo(pSongClass->szExtension,NULL,&bRegisterdForwxMusik);
            chklbFileAssocs->Append(wxString::Format(wxT("%s %s"),pSongClass->szExtension,sDesc.c_str()));
            if(bRegisterdForwxMusik)
                chklbFileAssocs->Check(chklbFileAssocs->GetCount()-1);
        }
    }
    return vsOptions_FileAssoc;
}
bool OptionFileAssocPanel::DoSavePrefs()
{
    DoFileAssociations();
    return true;
}

void OptionFileAssocPanel::DoFileAssociations()
{

    bool bNotifyExplorer = false;
    for(int i = 0; i < chklbFileAssocs->GetCount() ; i++)
    {
        wxString sEntry =	chklbFileAssocs->GetString(i);
        wxArrayString arrElements;
        DelimitStr(sEntry,wxT(" "),arrElements);
        if(chklbFileAssocs->IsChecked(i))
        {
            if(!FileTypeIsAssociated(arrElements[0]))
            {
                const tSongClass *pSongClass = CMetaDataHandler::GetSongClass(arrElements[0]);
                if(pSongClass)
                {
                    wxString sDesc = wxGetTranslation(pSongClass->szDescription);
                    AssociateWithFileType(arrElements[0],sDesc);
                    bNotifyExplorer = true;
                }
            }

        }
        else
        {
            if(FileTypeIsAssociated(arrElements[0]))
            {
                UnassociateWithFileType(arrElements[0]);
                bNotifyExplorer = true;
            }
        }
    }
#ifdef __WXMSW__
    if(bNotifyExplorer)
        SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);
#endif
}
