/*
 *  PlaylistInfoCtrl.cpp
 *
 *  The "Playlist Info" information
 *	This is the control visible on the main frame between
 *	the activity area and the playlist.
 *
 *  Copyright (c) 2003 Casey Langen (casey@bak.rr.com)
 *	Contributors: Simon Windmill, Dustin Carter, Gunnar Roth, Wade Brainerd
 *
 *  See the file "license.txt" for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL WARRANTIES.
*/

//--- For compilers that support precompilation, includes "wx/wx.h". ---//
#include "wx/wxprec.h"

#include "PlaylistInfoCtrl.h"
#include "PlaylistCtrl.h"
//--- globals ---//
#include "../MusikGlobals.h"
#include "../MusikUtils.h"

BEGIN_EVENT_TABLE(CPlaylistInfoCtrl,wxPanel)
#ifndef __WXMAC__
EVT_ERASE_BACKGROUND(CPlaylistInfoCtrl::OnEraseBackground)
#endif
END_EVENT_TABLE()

CPlaylistInfoCtrl::CPlaylistInfoCtrl( wxWindow *parent ,IPlaylistInfo *pIPlaylistInfo )
	: wxPanel( parent, -1, wxPoint( -1, -1 ), wxSize( -1, -1 ),wxNO_FULL_REPAINT_ON_RESIZE| wxCLIP_CHILDREN|wxTRANSPARENT_WINDOW )
	,m_pIPlaylistInfo( pIPlaylistInfo )
{

 	//--- static text objects ---/
	stTotal			= new wxStaticText_NoFlicker( this, -1, _( "Total Songs: " ),			wxPoint( -1, -1 ), wxSize( -1, -1 ), wxALIGN_LEFT | wxTRANSPARENT_WINDOW );
	stTotalVal		= new wxStaticText_NoFlicker( this, -1, wxT( "0" ),					wxPoint( -1, -1 ), wxSize( -1, -1 ), wxALIGN_LEFT | wxTRANSPARENT_WINDOW );
	stPlaylist		= new wxStaticText_NoFlicker( this, -1, _( "Playlist Songs: " ),		wxPoint( -1, -1 ), wxSize( -1, -1 ), wxALIGN_LEFT | wxTRANSPARENT_WINDOW );
	stPlaylistVal	= new wxStaticText_NoFlicker( this, -1, wxT( "0" ),					wxPoint( -1, -1 ), wxSize( -1, -1 ), wxALIGN_LEFT | wxTRANSPARENT_WINDOW );
	stRuntime		= new wxStaticText_NoFlicker( this, -1, _( "Playlist Runtime: " ),	wxPoint( -1, -1 ), wxSize( -1, -1 ), wxALIGN_LEFT | wxTRANSPARENT_WINDOW );
	stRuntimeVal	= new wxStaticText_NoFlicker( this, -1, wxT( "0:00" ),				wxPoint( -1, -1 ), wxSize( -1, -1 ), wxALIGN_LEFT | wxTRANSPARENT_WINDOW );
	stFilesize		= new wxStaticText_NoFlicker( this, -1, _( "Playlist size: " ),		wxPoint( -1, -1 ), wxSize( -1, -1 ), wxALIGN_LEFT | wxTRANSPARENT_WINDOW );
	stFilesizeVal	= new wxStaticText_NoFlicker( this, -1, wxT( "Empty" ),				wxPoint( -1, -1 ), wxSize( -1, -1 ), wxALIGN_LEFT | wxTRANSPARENT_WINDOW );
	

	//--- font stuff ---//
	stTotal->SetFont		( g_fntRegular );
	stTotalVal->SetFont		( g_fntBold );
	stPlaylist->SetFont		( g_fntRegular );
	stPlaylistVal->SetFont	( g_fntBold );
	stRuntime->SetFont		( g_fntRegular );
	stRuntimeVal->SetFont	( g_fntBold );
	stFilesize->SetFont		( g_fntRegular );
	stFilesizeVal->SetFont	( g_fntBold );

	//--- playlist info sizer ---//
	pSizer = new wxBoxSizer	( wxHORIZONTAL	);
	pSizer->Add( stTotal,		0, wxLEFT | wxALIGN_CENTER   , 0 );
	pSizer->Add( stTotalVal,	0, wxLEFT | wxALIGN_CENTER | wxADJUST_MINSIZE  , 2 );
	pSizer->Add( stPlaylist,	0, wxLEFT | wxALIGN_CENTER   , 4 );
	pSizer->Add( stPlaylistVal,	0, wxLEFT | wxALIGN_CENTER | wxADJUST_MINSIZE  , 2 );
	pSizer->Add( stRuntime,		0, wxLEFT | wxALIGN_CENTER   , 4 );
	pSizer->Add( stRuntimeVal,	0, wxLEFT | wxALIGN_CENTER | wxADJUST_MINSIZE  , 2 );   
	pSizer->Add( stFilesize,	0, wxLEFT | wxALIGN_CENTER   , 4 );
	pSizer->Add( stFilesizeVal,	0, wxLEFT | wxALIGN_CENTER | wxADJUST_MINSIZE  , 2 );

	//--- top sizer, vertical ---//
	pVert = new wxBoxSizer( wxVERTICAL );
	pVert->Add( pSizer, 0, wxALIGN_CENTER , 0 );

	SetSizerAndFit( pVert );
	Layout();
}

void CPlaylistInfoCtrl::OnEraseBackground( wxEraseEvent& (event) )
{	
	// empty => no background erasing to avoid flicker

	wxDC * TheDC = event.m_dc;
	wxColour BGColor =  GetBackgroundColour();
	wxBrush MyBrush(BGColor ,wxSOLID);
	TheDC->SetBackground(MyBrush);

	wxCoord width,height;
	TheDC->GetSize(&width,&height);
	wxCoord x,y,w,h;
	TheDC->GetClippingBox(&x,&y,&w,&h); 

	// Now  declare the Clipping Region which is
	// what needs to be repainted
	wxRegion MyRegion(x,y,w,h); 

	//Get all the windows(controls)  rect's on the dialog
	wxWindowList & children = GetChildren();
	for ( wxWindowList::Node *node = children.GetFirst(); node; node = node->GetNext() )
	{
		wxWindow *current = (wxWindow *)node->GetData();

		// now subtract out the controls rect from the
		//clipping region
		MyRegion.Subtract(current->GetRect());
	}

	// now destroy the old clipping region
	TheDC->DestroyClippingRegion();

	//and set the new one
	TheDC->SetClippingRegion(MyRegion);
	TheDC->Clear();
}
void CPlaylistInfoCtrl::Update()
{
	//--- playlist info ---//
	if ( wxGetApp().Prefs.bShowPLInfo == 1 )
	{
		stTotalVal->SetLabel( wxString::Format( wxT( "%d" ), wxGetApp().Library.GetSongCount() ) );
		stPlaylistVal->SetLabel( wxString::Format( wxT( "%d" ), m_pIPlaylistInfo->GetCount() ) );
		int nPlaylistTime = m_pIPlaylistInfo->GetTotalPlayingTimeInSeconds();
		wxTimeSpan PlayTime( 0, 0, nPlaylistTime );
		stRuntimeVal->SetLabel ( PlayTime.Format() );
		wxString strsize( wxT("0.0 mb") );
		double kbsize = 1024.0;
		double totsize = m_pIPlaylistInfo->GetTotalFilesize().GetValue();
		if ( totsize < kbsize )
			strsize = wxString::Format( wxT("%.2f b"),totsize );
		else if ( totsize < ( kbsize * kbsize ) )
			strsize = wxString::Format( wxT("%.2f kb"), totsize / kbsize );
		else if ( totsize < ( kbsize * kbsize * kbsize ) )
			strsize = wxString::Format( wxT("%.2f mb"), totsize / kbsize / kbsize);
		else if ( totsize < ( kbsize * kbsize * kbsize * kbsize ) )
			strsize = wxString::Format( wxT("%.2f gb"), totsize / kbsize / kbsize / kbsize );
		stFilesizeVal->SetLabel( strsize );
	}
	wxColour bg = GetParent()->GetBackgroundColour();
	if(GetBackgroundColour() !=  bg)
	{
		SetBackgroundColour( bg );
		wxWindowList & children = GetChildren();
		for ( wxWindowList::Node *node = children.GetFirst(); node; node = node->GetNext() )
		{
			wxWindow *current = (wxWindow *)node->GetData();
			current->SetBackgroundColour(bg);
		}

	}
	Layout();
}
