/*
 *  MusikPrefsDialog.cpp
 *
 *  Preferences frame
 *  
 *  Copyright (c) 2003 Casey Langen (casey@bak.rr.com)
 *	Contributors: Simon Windmill, Dustin Carter, Gunnar Roth, Wade Brainerd
 *
 *  See the file "license.txt" for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL WARRANTIES.
*/

//--- wx ---//
#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif 

#include "MusikPrefsFrame.h"

//--- globals ---//
#include "MusikGlobals.h"
#include "MusikUtils.h"
//--- classes ---//


//--- related frames ---//
#include "Frames/MusikFrame.h"

#include "OptionGeneralPanel.h"
#include "OptionPlaylistPanel.h"
#include "OptionSelectionsPanel.h"
#include "OptionTunagePanel.h"
#include "OptionFileAssocPanel.h"
#include "OptionAutoDJPanel.h"
#include "SoundCrossfaderPanel.h"
#include "SoundDriverPanel.h"
#include "StreamingBufferPanel.h"
#include "StreamingProxyServerPanel.h"
#include "TaggingAutoRenamePanel.h"
#include "TaggingGeneralPanel.h"




enum EMUSIK_PREFERENCES_OBJECT_ID
{
    MUSIK_PREFERENCES_TREE = 1
};

BEGIN_EVENT_TABLE(PrefPanel, wxPanel)
END_EVENT_TABLE()







class PrefTreeItemData : public wxTreeItemData
{
public:
    PrefTreeItemData(PrefPanel * panel) : m_pPanel(panel) { }
    PrefPanel * Panel() { return m_pPanel;}
private:
    PrefPanel * m_pPanel;
};


BEGIN_EVENT_TABLE(MusikPrefsDialog, wxDialog)
#ifdef __WXMSW__
	EVT_CHAR_HOOK			(								MusikPrefsDialog::OnTranslateKeys	)
#else
	EVT_CHAR	(											MusikPrefsDialog::OnTranslateKeys	)
#endif
	EVT_TREE_SEL_CHANGED	(MUSIK_PREFERENCES_TREE,					MusikPrefsDialog::OnTreeChange		)
	EVT_BUTTON				(wxID_OK,						MusikPrefsDialog::OnClickOK			)
	EVT_BUTTON				(wxID_APPLY,					MusikPrefsDialog::OnClickApply		)
	EVT_BUTTON				(wxID_CANCEL,					MusikPrefsDialog::OnClickCancel		)
	EVT_CLOSE				(											MusikPrefsDialog::OnClose			)
END_EVENT_TABLE()


MusikPrefsDialog::MusikPrefsDialog( wxWindow *pParent, const wxString &sTitle )
    :wxDialog( pParent, -1, sTitle, wxDefaultPosition, wxSize(750,700), wxDEFAULT_FRAME_STYLE|wxRESIZE_BORDER|wxCAPTION | wxTAB_TRAVERSAL | wxFRAME_FLOAT_ON_PARENT | wxFRAME_NO_TASKBAR )
    ,m_pCurrentPanel(NULL)
{
	//---------------//
	//--- colours ---//
	//---------------//
//	static wxColour cBtnFace = wxSystemSettings::GetColour( wxSYS_COLOUR_3DFACE );
//	this->SetBackgroundColour ( cBtnFace );

	//------------//
	//--- icon ---//
	//------------//
	#if defined (__WXMSW__)
		SetIcon( wxICON( musicbox ) );
	#endif


	//------------------------------//
	//--- Selection Tree Control ---//
	//------------------------------//
	tcPreferencesTree	= new wxTreeCtrl( this, MUSIK_PREFERENCES_TREE, wxDefaultPosition, wxSize(-1,-1), wxTR_HAS_BUTTONS | wxSIMPLE_BORDER );
	//--- root nodes ---//
	wxTreeItemId nRootID			=	tcPreferencesTree->AddRoot		( _( "Preferences" )		);
	wxTreeItemId nOptionsRootID	=	tcPreferencesTree->AppendItem	( nRootID, _( "Options" )	);
	wxTreeItemId nTagRootID		=	tcPreferencesTree->AppendItem	( nRootID, _( "Tagging" )	);
	wxTreeItemId nSoundRootID	=	tcPreferencesTree->AppendItem	( nRootID, _( "Sound" )		);	
	wxTreeItemId nStreamingRootID	=	tcPreferencesTree->AppendItem	( nRootID, _( "Streaming" )	);
	//--- child nodes ---//



	//----------------------//
	//--- System Buttons ---//
	//----------------------//
	wxButton* btnCancel =	new wxButton_NoFlicker( this, wxID_CANCEL,	_("Cancel"),	wxDefaultPosition, wxDefaultSize );
	wxButton* btnApply =	new wxButton_NoFlicker( this, wxID_APPLY,	_("Apply"),		wxDefaultPosition, wxDefaultSize );
	wxButton* btnOK =		new wxButton_NoFlicker( this, wxID_OK,		_("OK"),		wxDefaultPosition, wxDefaultSize );

	//----------------------------//
	//--- System Buttons Sizer ---//
	//----------------------------//
	wxBoxSizer *hsSysButtons = new wxBoxSizer( wxHORIZONTAL );
	hsSysButtons->Add( btnCancel,	0, wxALIGN_LEFT		);
	hsSysButtons->Add( -1,-1,	1, wxEXPAND			);
	hsSysButtons->Add( btnApply,	0, wxALIGN_RIGHT	);
	hsSysButtons->Add( btnOK,		0, wxALIGN_RIGHT | wxLEFT, 4);

	//-------------------------//
	//--- Hide / Show Sizer ---//
	//-------------------------//
	wxBoxSizer *hsSplitter = new wxBoxSizer( wxHORIZONTAL );
    wxBoxSizer *vsRight = new wxBoxSizer( wxVERTICAL );

    m_sizerPanels = new wxBoxSizer( wxVERTICAL );
	hsSplitter->Add( tcPreferencesTree,		1, wxEXPAND | wxRIGHT, 8 );
    hsSplitter->Add( vsRight,	3 ,wxEXPAND|wxALL,5);
    
    wxPanel *HeaderPanel = new wxPanel(this,-1,wxDefaultPosition,wxSize(-1,40),wxBORDER);
    HeaderPanel->SetBackgroundColour ( wxSystemSettings::GetColour( wxSYS_COLOUR_HIGHLIGHT ) );
    m_stPanelHeader = new wxStaticText(HeaderPanel,-1,wxEmptyString);
    wxBoxSizer *hsHeaderPanel = new wxBoxSizer( wxHORIZONTAL );
    hsHeaderPanel->Add( m_stPanelHeader,1,wxALIGN_CENTER_VERTICAL|wxLEFT,10);   
    HeaderPanel->SetSizer(hsHeaderPanel);
    
    wxFont f = m_stPanelHeader->GetFont();
    f.SetPointSize(f.GetPointSize() * 2);
    f.SetWeight(wxBOLD);
    m_stPanelHeader->SetFont(f);
    
    vsRight->Add(HeaderPanel,0,wxEXPAND|wxALL,5);
    vsRight->Add(m_sizerPanels,1,wxEXPAND|wxALL,10);
 
    AddPanel(nOptionsRootID,new OptionGeneralPanel(this));
    AddPanel(nOptionsRootID,new OptionSelectionsPanel(this));
    AddPanel(nOptionsRootID,new OptionPlaylistPanel(this));
    AddPanel(nOptionsRootID,new OptionFileAssocPanel(this));
    AddPanel(nOptionsRootID,new OptionTunagePanel(this));
    AddPanel(nOptionsRootID,new OptionAutoDJPanel(this));
    AddPanel(nTagRootID,new TaggingGeneralPanel(this));
    AddPanel(nTagRootID,new TaggingAutoRenamePanel(this));
    AddPanel(nSoundRootID,new SoundDriverPanel(this));
    AddPanel(nSoundRootID,new SoundCrossfaderPanel(this));
    AddPanel(nStreamingRootID,new StreamingBufferPanel(this));
    AddPanel(nStreamingRootID,new StreamingProxyServerPanel(this));


    //--- expand all the root nodes ---//
    tcPreferencesTree->Expand( nRootID );
    tcPreferencesTree->Expand( nOptionsRootID );
    tcPreferencesTree->Expand( nTagRootID );
    tcPreferencesTree->Expand( nSoundRootID );
    tcPreferencesTree->Expand( nSoundRootID );
    tcPreferencesTree->Expand( nStreamingRootID );
    
    //-----------------//
	//--- Top Sizer ---//
	//-----------------//
	wxBoxSizer *vsTopSizer = new wxBoxSizer( wxVERTICAL );
	vsTopSizer->Add( hsSplitter,	1, wxEXPAND | wxALL, 2 );
	vsTopSizer->Add( hsSysButtons,	0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 2 );
	SetSizer( vsTopSizer );

	//--------------//
	//--- Layout ---//
	//--------------//
	
	Centre();
	//--------------------//
	//--- Set Defaults ---//
	//--------------------//
    SelectFirstPanel(); 
    Layout();

}

void MusikPrefsDialog::SelectFirstPanel()
{
    wxTreeItemId node = tcPreferencesTree->GetRootItem();
    while(tcPreferencesTree->ItemHasChildren(node))
    {
        wxTreeItemIdValue cookie;
        node = tcPreferencesTree->GetFirstChild(node,cookie);
    }
    tcPreferencesTree->SelectItem( node );
}
void MusikPrefsDialog::AddPanel( const wxTreeItemId &tidParent,PrefPanel * panel)
{
    tcPreferencesTree->AppendItem	( tidParent, panel->Name(),-1,-1,new PrefTreeItemData(panel));
    m_sizerPanels->Add( panel,	1 ,wxEXPAND);
    m_sizerPanels->Show( panel,false );
}

//--------------//
//--- Events ---//
//--------------//

void MusikPrefsDialog::OnClickOK			( wxCommandEvent& WXUNUSED(event) )	{	Close( false );			}
void MusikPrefsDialog::OnClickApply		( wxCommandEvent& WXUNUSED(event) )	{	SavePrefs();			}
void MusikPrefsDialog::OnClickCancel		( wxCommandEvent& WXUNUSED(event) )	{	Close( true );			}
void MusikPrefsDialog::OnClose			( wxCloseEvent& WXUNUSED(event) )	{	Close( true );			}


void MusikPrefsDialog::Close( bool bCancel )
{
	if ( !bCancel )
		if(!SavePrefs())
			return;

	g_MusikFrame->Enable( TRUE );
	this->Destroy();
	
}


void MusikPrefsDialog::OnTreeChange( wxTreeEvent& event )
{
    PrefPanel *newpanel = NULL;
    if(event.GetItem().IsOk())
    {
        PrefTreeItemData * newitemdata = (PrefTreeItemData *)tcPreferencesTree->GetItemData(event.GetItem());
        if(newitemdata)
            newpanel = newitemdata->Panel();
    }
    if( m_pCurrentPanel && newpanel && (m_pCurrentPanel != newpanel)) // we have a valid new item, if not we must not hide the current panel
    { // hide old panel
        m_sizerPanels->Show( m_pCurrentPanel, false );
    }
    if(newpanel && (m_pCurrentPanel != newpanel))
    {   // show new panel
        m_stPanelHeader->SetLabel(newpanel->Name());
        m_sizerPanels->Show( m_pCurrentPanel = newpanel, true );
        m_pCurrentPanel->Layout();
    }
  	Layout();
}

void MusikPrefsDialog::OnTranslateKeys( wxKeyEvent& event )
{
	if ( event.GetKeyCode() == WXK_ESCAPE )
		Close( true );
	else
		event.Skip();
}
bool MusikPrefsDialog::SavePrefs()
{
    wxTreeItemId root = tcPreferencesTree->GetRootItem();
    if(root.IsOk())
        return SavePrefs(root);
    return true;
}
bool MusikPrefsDialog::SavePrefs(const wxTreeItemId & root)
{
    if(tcPreferencesTree->ItemHasChildren(root))
    {
        wxTreeItemIdValue cookie;
        wxTreeItemId child = tcPreferencesTree->GetFirstChild(root,cookie);
        while(child.IsOk())
        {
            if(SavePrefs(child) == false)
                return false;
            child = tcPreferencesTree->GetNextChild(root,cookie);
        }
    }
    else
    { // root is in fact a leaf
        PrefTreeItemData *itemdata = (PrefTreeItemData *)tcPreferencesTree->GetItemData(root);
        if(itemdata)
            return itemdata->Panel()->SavePrefs();

    }
	return true;
}

