/*
 *  MusikVolumeFrame.cpp
 *
 *  Frame that popups to adjust volume
 *  
 *  Copyright (c) 2003 Casey Langen (casey@bak.rr.com)
 *	Contributors: Simon Windmill, Dustin Carter, Gunnar Roth, Wade Brainerd
 *
 *  See the file "license.txt" for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL WARRANTIES.
*/

//--- For compilers that support precompilation, includes "wx/wx.h". ---//
#include "wx/wxprec.h"

//--- main header ---//
#include "MusikVolumeFrame.h"

//--- globals ---//
#include "../MusikGlobals.h"
#include "../MusikUtils.h"

BEGIN_EVENT_TABLE(MusikVolumeFrame, wxFrame)
	EVT_ACTIVATE			(MusikVolumeFrame::OnActivate		)
	EVT_CLOSE				(MusikVolumeFrame::OnClose			)
	EVT_CHAR				(MusikVolumeFrame::OnChar			)
END_EVENT_TABLE()

//--- default constructor ---//
MusikVolumeFrame::MusikVolumeFrame( wxFrame* pParent, wxPoint pos )
	: wxFrame( pParent, -1, _("Playback"), pos, wxSize( -1, -1 ),  wxFRAME_FLOAT_ON_PARENT | wxFRAME_NO_TASKBAR | wxCLIP_CHILDREN )
{
	//------------------------------//
	//--- initialize needed vars ---//
	//------------------------------//
	parent			= pParent;
	

	//----------------------------//
	//--- colours and controls ---//
	//----------------------------//
	SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_3DFACE ) );
	gSeek = new wxGauge	( this, -1, 255, wxPoint( -1, -1 ), wxSize( wxSystemSettings::GetMetric(wxSYS_HSCROLL_Y),wxSystemSettings::GetMetric(wxSYS_HSCROLL_Y)*15 ), wxGA_SMOOTH | wxGA_VERTICAL | wxCLIP_CHILDREN );
	pSeekEvt = new CGaugeSeekEvt( gSeek, wxGA_VERTICAL );
	gSeek->PushEventHandler( pSeekEvt );

	//-----------------//
	//--- top sizer ---//
	//-----------------//
	wxBoxSizer *pTopSizer = new wxBoxSizer( wxHORIZONTAL );
	pTopSizer->Add( gSeek, 0,  wxALL, 2 );
	


	//-------------------------//
	//--- initialize values ---//
	//-------------------------//
	gSeek->SetValue( wxGetApp().Prefs.nSndVolume );

	SetSizerAndFit( pTopSizer );

	Layout();
}

//--- when window gets activated ---//
void MusikVolumeFrame::OnActivate( wxActivateEvent& event )
{
	if ( !event.GetActive() )
		Close();

}

//--- translate keypress ---//
void MusikVolumeFrame::OnChar( wxKeyEvent& event )
{
	if ( event.GetKeyCode() == WXK_ESCAPE )
		Close();
	else
		event.Skip();
}

//--- close func ---//
void MusikVolumeFrame::Close()
{
	Destroy();
}
