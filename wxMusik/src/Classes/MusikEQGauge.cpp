/*
 *  MusikEQGauge.cpp
 *
 *  Subclassed wxGauge used by equalizer. Has special nifty functions.
 *
 *  Copyright (c) 2003 Casey Langen (casey@bak.rr.com)
 *	Contributors: Simon Windmill, Dustin Carter, Gunnar Roth, Wade Brainerd
 *
 *  See the file "license.txt" for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL WARRANTIES.
*/

//--- For compilers that support precompilation, includes "wx/wx.h". ---//
#include "wx/wxprec.h"

//--- globals ---//
#include "../MusikGlobals.h"
#include "../MusikUtils.h"

#include "MusikEQGauge.h"
#include "MusikEQCtrl.h"

//-----------------------------------------//
//--- this is the gauge					---//
//-----------------------------------------//
BEGIN_EVENT_TABLE(CMusikEQGauge, wxGauge)
	EVT_LEFT_DOWN				( CMusikEQGauge::OnLeftDown		) 
	EVT_LEFT_UP					( CMusikEQGauge::OnLeftUp		)
	EVT_MOTION					( CMusikEQGauge::OnMouseMove	) 
#ifndef __WXMAC__
	EVT_ERASE_BACKGROUND		( CMusikEQGauge::OnEraseBackground )
#endif
#ifdef __WXMSW__
	EVT_PAINT ( CMusikEQGauge::OnPaint )
#endif
END_EVENT_TABLE()

CMusikEQGauge::CMusikEQGauge( CMusikEQCtrl* parent, size_t nChannel, size_t nBandID )
	: wxGauge( parent, -1, 100, wxPoint( 0, 0 ), wxSize( wxSystemSettings::GetMetric( wxSYS_HSCROLL_Y ), 200 ), wxGA_SMOOTH | wxGA_VERTICAL | wxCLIP_CHILDREN )
{
	m_Channel = nChannel;
	m_BandID = nBandID;
	m_Parent = parent;	
	m_Dragging = false;
}

CMusikEQGauge::~CMusikEQGauge()
{
}
#ifdef __WXMSW__
#include "wx/dcbuffer.h"
void CMusikEQGauge::OnPaint(wxPaintEvent& WXUNUSED(event))
{
	
	wxBufferedPaintDC dc(this);
	wxEraseEvent erase_event(GetId(), &dc);
	wxGauge::OnEraseBackground(erase_event);
	MSWDefWindowProc(WM_PAINT, (WPARAM) (HDC) dc.GetHDC(), 0);
}
#endif
void CMusikEQGauge::OnEraseBackground( wxEraseEvent& WXUNUSED(event) )
{	
	// empty => no background erasing to avoid flicker
}

void CMusikEQGauge::SetPos( size_t m_Pos )
{
	SetValue( (int)m_Pos );
	Colourize();
	if ( m_Parent->chkLock->IsChecked() )
	{
		if ( m_Channel == MUSIK_FX_LEFT )
		{
			m_Parent->slRight[m_BandID]->SetValue( m_Pos );
			m_Parent->slRight[m_BandID]->Colourize();
		}
		else
		{
			m_Parent->slLeft[m_BandID]->SetValue( m_Pos );
			m_Parent->slLeft[m_BandID]->Colourize();
		}
	}
}

void CMusikEQGauge::Colourize()
{
	/*
	if ( wxGetApp().Prefs.nEQColourize != 1 )
		return;
	*/
	int nGreen = (int)( 200.0f * (float)GetValue() / 100.0f );
	SetForegroundColour( wxColour( 200 - nGreen, nGreen, 0 ) );
}

void CMusikEQGauge::SetEQ()
{
	m_Parent->BandsFromSliders();
}

void CMusikEQGauge::OnLeftDown( wxMouseEvent& event )
{
	m_Dragging = true;
	CaptureMouse();
    SetFromMousePos( event );
}

void CMusikEQGauge::OnMouseMove( wxMouseEvent& event )
{
	if ( m_Dragging )
	{
		if ( event.LeftIsDown() )
			SetFromMousePos( event );
	}
}

void CMusikEQGauge::OnLeftUp( wxMouseEvent& WXUNUSED(event) )
{
	if ( m_Dragging )
	{
		SetEQ();
		ReleaseMouse();
		m_Dragging = false;
	}
}

void CMusikEQGauge::SetFromMousePos( wxMouseEvent& event )
{
	
	//--- stuff we'll need for calculation ---//
	m_MousePos	= event.GetPosition();
	m_WndSize	= GetSize();	

	//--- set value ---//
	m_Temp = (float)100* ( (float)m_MousePos.y / (float)m_WndSize.GetHeight() );
	if ( m_Temp < 0.0f )
		m_Temp = 0.0f;
	else if ( m_Temp > 100.0f )
		m_Temp = 100.0f;

	if ( m_Temp != m_Pos )
	{
		m_Pos = m_Temp;
		SetPos( (size_t)( 100 - (int)m_Pos ) );
	}	
}
