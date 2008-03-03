/*
*  OptionVisualAttributesPanel.cpp
*
*  OptionVisualAttributesPanel 
*  
*  Copyright (c) 2008 Gunnar Roth (gunnar.roth@gmx.de)
*	
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

#include "OptionVisualAttributesPanel.h"

#include <wx/colordlg.h>
#include "Frames/MusikFrame.h"
#include "Classes/ActivityAreaCtrl.h"
#include "Classes/PlaylistCtrl.h"
#include "Classes/SourcesBox.h"
#include "MusikGlobals.h"

enum OBJECT_ID
{
    MUSIK_PREFERENCES_PLAYLIST_STRIPE_COLOUR = 1,
    MUSIK_PREFERENCES_ACTIVITY_STRIPE_COLOUR,
    MUSIK_PREFERENCES_SOURCES_STRIPE_COLOUR,
    MUSIK_PREFERENCES_PLAYLIST_BORDER_COLOUR,
	MUSIK_PREFERENCES_TRANSPARENTWINDOWALPHA
};
BEGIN_EVENT_TABLE(OptionVisualAttributesPanel, PrefPanel)
EVT_COLOURPICKER_CHANGED(MUSIK_PREFERENCES_PLAYLIST_STRIPE_COLOUR,  OptionVisualAttributesPanel::OnColourChange_PLStripeColour)
EVT_COLOURPICKER_CHANGED(MUSIK_PREFERENCES_ACTIVITY_STRIPE_COLOUR,  OptionVisualAttributesPanel::OnColourChange_ActivityStripeColour)
EVT_COLOURPICKER_CHANGED(MUSIK_PREFERENCES_SOURCES_STRIPE_COLOUR,	OptionVisualAttributesPanel::OnColourChange_SourcesStripeColour		)
EVT_COLOURPICKER_CHANGED(MUSIK_PREFERENCES_PLAYLIST_BORDER_COLOUR,	OptionVisualAttributesPanel::OnColourChange_PLBorderStripeColour		)

EVT_COMMAND_SCROLL(MUSIK_PREFERENCES_TRANSPARENTWINDOWALPHA, OptionVisualAttributesPanel::OnTransparentWindowAlpha)
END_EVENT_TABLE()

void OptionVisualAttributesPanel::OnColourChange_PLStripeColour(wxColourPickerEvent& event)
{
	if ( ColourToString( event.GetColour() ) != wxGetApp().Prefs.sPLStripeColour )
	{
		wxGetApp().Prefs.sPLStripeColour = ColourToString(event.GetColour());
		OptionVisualAttributesPanel::NeedUpdateSink::m_bPlaylistUpdate = true;
	}
}
void OptionVisualAttributesPanel::OnColourChange_ActivityStripeColour(wxColourPickerEvent& event)
{
	if ( ColourToString(event.GetColour()) != wxGetApp().Prefs.sActStripeColour )
	{
		wxGetApp().Prefs.sActStripeColour = ColourToString(event.GetColour());
		OptionVisualAttributesPanel::NeedUpdateSink::m_bActivityUpdate = true;
	}
}
void OptionVisualAttributesPanel::OnColourChange_SourcesStripeColour(wxColourPickerEvent& event)
{
	if ( ColourToString(event.GetColour()) != wxGetApp().Prefs.sActStripeColour )
	{
		wxGetApp().Prefs.sActStripeColour = ColourToString(event.GetColour());
		OptionVisualAttributesPanel::NeedUpdateSink::m_bSourcesUpdate = true;
	}
}
void OptionVisualAttributesPanel::OnColourChange_PLBorderStripeColour(wxColourPickerEvent& event)
{
	if ( ColourToString(event.GetColour()) != wxGetApp().Prefs.sPlaylistBorderColour )
	{
		wxGetApp().Prefs.sPlaylistBorderColour = ColourToString(event.GetColour());
		OptionVisualAttributesPanel::NeedUpdateSink::m_bPlaylistUpdate = true;
		OptionVisualAttributesPanel::NeedUpdateSink::m_bSourcesUpdate = true;
	}
}

void OptionVisualAttributesPanel::OnTransparentWindowAlpha(wxScrollEvent& event)
{
	wxGetApp().Prefs.nTransparentWindowAlpha = event.GetInt();
	wxTopLevelWindow *parent = GetTopLevelParent();
	parent && parent->SetTransparent(wxMulDivInt32(wxGetApp().Prefs.nTransparentWindowAlpha , 255 , 100));
}
wxTopLevelWindow * OptionVisualAttributesPanel::GetTopLevelParent()
{
	return  wxDynamicCast(GetParent(),wxTopLevelWindow);
}

IMPLEMENT_PREFPANEL_CONSTRUCTOR(OptionVisualAttributesPanel)

wxSizer * OptionVisualAttributesPanel::CreateControls()
{    //--------------------------//
    //--- Options -> Visual Attributes ---//
    //--------------------------//
	PREF_CREATE_CHECKBOX(BlankSwears,_("Censor common swearwords"));
    PREF_CREATE_CHECKBOX_EX(SortArtistWithoutPrefix,_("Sort Artist without prefix"),&snkPlaylistActivityNeedUpdate);
    PREF_CREATE_CHECKBOX_EX(PLStripes,_("Show \"stripes\" in playlist"),&snkPlaylistNeedUpdate);
    PREF_CREATE_CHECKBOX_EX(ActStripes,_("Show \"stripes\" in selection boxes"),&snkActivityNeedUpdate);
    PREF_CREATE_CHECKBOX_EX(SourcesStripes,_("Show \"stripes\" in sources box"),&snkSourcesNeedUpdate);
    PREF_CREATE_CHECKBOX_EX(PlaylistBorder,_("Use selected border colour"),&snkPlaylistSourcesNeedUpdate);

    wxColourPickerCtrl *cpcPlaylistStripeColour	=	new wxColourPickerCtrl( this, MUSIK_PREFERENCES_PLAYLIST_STRIPE_COLOUR,StringToColour( wxGetApp().Prefs.sPLStripeColour ) );
    wxColourPickerCtrl *cpcActivityStripeColour	=	new wxColourPickerCtrl( this, MUSIK_PREFERENCES_ACTIVITY_STRIPE_COLOUR,StringToColour( wxGetApp().Prefs.sActStripeColour ) );
    wxColourPickerCtrl *cpcSourcesStripeColour	=	new wxColourPickerCtrl( this, MUSIK_PREFERENCES_SOURCES_STRIPE_COLOUR,StringToColour( wxGetApp().Prefs.sSourcesStripeColour ) );
    wxColourPickerCtrl *cpcPlaylistBorderColour =   new wxColourPickerCtrl( this, MUSIK_PREFERENCES_PLAYLIST_BORDER_COLOUR,StringToColour( wxGetApp().Prefs.sPlaylistBorderColour ) );

	wxTopLevelWindow *parent = GetTopLevelParent();
	wxSlider * sldTransparentWindowAlpha = NULL;
	if(parent && parent->CanSetTransparent())
	{
		sldTransparentWindowAlpha = new wxSlider(this,MUSIK_PREFERENCES_TRANSPARENTWINDOWALPHA,
			wxMin(100,wxMax(50,wxGetApp().Prefs.nTransparentWindowAlpha)), 50, 100,
			wxDefaultPosition, wxDefaultSize,
			wxSL_LABELS|wxSL_TOP);
	}
	
    wxBoxSizer *vsOptions_Interface = new wxBoxSizer( wxVERTICAL );
    vsOptions_Interface->Add( chkBlankSwears,			0, wxALL, 4 );
    vsOptions_Interface->Add( chkSortArtistWithoutPrefix,0, wxALL, 4 );
	
    vsOptions_Interface->Add( chkPLStripes,		0, wxALL, 4 );
    vsOptions_Interface->Add( cpcPlaylistStripeColour,	0, wxALL, 4 );
    vsOptions_Interface->Add( chkActStripes,	0, wxALL, 4 );
    vsOptions_Interface->Add( cpcActivityStripeColour,	0, wxALL, 4 );
    vsOptions_Interface->Add( chkSourcesStripes,		0, wxALL, 4 );
    vsOptions_Interface->Add( cpcSourcesStripeColour,	0, wxALL, 4 );
    vsOptions_Interface->Add( chkPlaylistBorder,		0, wxALL, 4 );
    vsOptions_Interface->Add( cpcPlaylistBorderColour,	0, wxALL, 4 );
	if(sldTransparentWindowAlpha)
		vsOptions_Interface->Add( sldTransparentWindowAlpha,0, wxALL, 4 );
   
    return vsOptions_Interface;
}

void OptionVisualAttributesPanel::DoLoadPrefs()
{
}
bool OptionVisualAttributesPanel::DoSavePrefs()
{
    return true;
}

void OptionVisualAttributesPanel::AfterDataTransferredFromWindow()
{
    if ( OptionVisualAttributesPanel::NeedUpdateSink::m_bPlaylistUpdate )
        g_PlaylistBox->Update();
    if ( OptionVisualAttributesPanel::NeedUpdateSink::m_bSourcesUpdate )
        g_SourcesCtrl->Update();
    if ( OptionVisualAttributesPanel::NeedUpdateSink::m_bActivityUpdate )
        g_ActivityAreaCtrl->ReloadAllContents();

    if(OptionVisualAttributesPanel::NeedUpdateSink::m_bSourcesUpdate 
        || OptionVisualAttributesPanel::NeedUpdateSink::m_bActivityUpdate 
        || OptionVisualAttributesPanel::NeedUpdateSink::m_bPlaylistUpdate)
    {
        // force update of everything
        g_MusikFrame->SendSizeEvent();
    }

}



bool OptionVisualAttributesPanel::NeedUpdateSink::m_bPlaylistUpdate;
bool OptionVisualAttributesPanel::NeedUpdateSink::m_bActivityUpdate;
bool OptionVisualAttributesPanel::NeedUpdateSink::m_bSourcesUpdate;
