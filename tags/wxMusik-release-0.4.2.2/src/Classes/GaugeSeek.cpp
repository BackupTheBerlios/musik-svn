/*
 *  GaugeSeek.cpp
 *
 *  Custom event to allow mouse drag seeking for
 *  (currently only) horizontal wxGauge objects.
 *
 *  Copyright (c) 2003 Casey Langen (casey@bak.rr.com)
 *	Contributors: Simon Windmill, Dustin Carter, Gunnar Roth, Wade Brainerd
 *
 *  See the file "license.txt" for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL WARRANTIES.
*/
// For compilers that support precompilation, includes "wx/wx.h".
#include "myprec.h"
#include "GaugeSeek.h"
#include "MusikGlobals.h"
#include "MusikUtils.h" 
#include "Classes/NowPlayingCtrl.h"
//--- crossfader, other threads ---//
#include "Threads/MusikThreads.h"

BEGIN_EVENT_TABLE(CGaugeSeekEvt, wxEvtHandler)
	EVT_LEFT_DOWN			(CGaugeSeekEvt::OnLeftDown		) 
	EVT_LEFT_UP				(CGaugeSeekEvt::OnLeftUp		) 
	EVT_MOTION				(CGaugeSeekEvt::OnMouseMove		) 
	EVT_ERASE_BACKGROUND	( CGaugeSeekEvt::OnEraseBackground )
#ifdef __WXMSW__
	EVT_PAINT ( CGaugeSeekEvt::OnPaint )
#endif
END_EVENT_TABLE()

CGaugeSeekEvt::CGaugeSeekEvt( wxGauge *parent, long style )
{
	pParent		= parent; 
	lType		= style; 
	m_LastPos	= -1;
	m_Dragging	= false;
}
#ifdef __WXMSW__
#include "wx/dcbuffer.h"
void CGaugeSeekEvt::OnPaint(wxPaintEvent& WXUNUSED(event))
{
	wxControl * pControl = (wxControl*)pParent;
	wxBufferedPaintDC dc(pControl);
	wxEraseEvent erase_event(pControl->GetId(), &dc);
	pControl->OnEraseBackground(erase_event);
	pControl->MSWDefWindowProc(WM_PAINT, (WPARAM) (HDC) dc.GetHDC(), 0);
}
#endif
void CGaugeSeekEvt::OnEraseBackground( wxEraseEvent& WXUNUSED(event) )
{
	// empty => no background erasing to avoid flicker
}

void CGaugeSeekEvt::OnLeftDown( wxMouseEvent& event )
{
	//--- we're using the seek bar AND music is playing ---//
	//--- OR we're adjusting volume                     ---//
	//-----------------------------------------------------//
	if ( !m_Dragging )
	{
		m_Dragging = true;
		pParent->CaptureMouse();
		if ( lType == wxGA_HORIZONTAL )
			g_TimeSeeking = true;

		if ( ( lType == wxGA_HORIZONTAL && wxGetApp().Player.IsSeekable() ) || lType == wxGA_VERTICAL )
			SetFromMousePos( event );
	}
	else
		event.Skip();

}

void CGaugeSeekEvt::OnMouseMove( wxMouseEvent& event )
{
	if ( m_Dragging )
	{
		if ( event.LeftIsDown() )
		{
			if ( ( lType == wxGA_HORIZONTAL && wxGetApp().Player.IsSeekable() ) || lType == wxGA_VERTICAL )
				SetFromMousePos( event );		
		}
	}
	else
		event.Skip();
}

void CGaugeSeekEvt::OnLeftUp( wxMouseEvent& (event) )
{
	if ( m_Dragging )
	{
		pParent->ReleaseMouse();
		m_Dragging = false;
		//-----------------------------------------------//
		//--- if we have left up and modifying time	  ---//
		//--- assume user is done. update pos		  ---//
		//-----------------------------------------------//   		
		if ( lType == wxGA_HORIZONTAL )
		{
			if(wxGetApp().Player.IsSeekable())
			{
				//--- set player pos ---//
				int nTime = ( (int)fPos * (int)wxGetApp().Player.GetDuration( UNIT_SEC ) ) / (int)100;
				wxGetApp().Player.SetTime( nTime );	
			}
			g_TimeSeeking = false;
		}
	}
	else
		event.Skip();

}

void CGaugeSeekEvt::SetFromMousePos( wxMouseEvent& event )
{
	//--- stuff we'll need for calculation ---//
	wxPoint mousepos	= event.GetPosition();
	wxSize	wndsize		= pParent->GetSize();

	//---------------------------------------------//
	//--- if horizontal, we're seeking the song ---//
	//---------------------------------------------//
	if ( lType == wxGA_HORIZONTAL )
	{
		//--- get percent ---//
		int nSizeX	= wndsize.GetWidth();	
		int nMouseX = mousepos.x;

		//--- set value ---//
		fPos = (float)100* ( (float)nMouseX / (float)nSizeX );
		if ( fPos < 0.0f )
			fPos = 0.0f;
		else if ( fPos > 100.0f )
			fPos = 100.0f;

		int nThisPos = (int)fPos;

		if ( nThisPos != m_LastPos )
		{
			pParent->SetValue( nThisPos );
			m_LastPos = nThisPos;
		}

		//--- set string ---//
		int nCurrTime = (int)( (float)wxGetApp().Player.GetDuration( UNIT_MILLISEC ) * (float)( fPos / (float)100 ) );
		g_MusikFrame->m_pBottomPanel->SetTime(nCurrTime);
	}

	//-----------------------------------------------//
	//--- if vertical, we're adjusting the volume ---//
	//-----------------------------------------------//
	if ( lType == wxGA_VERTICAL )
	{
		//--- get percent ---//
		int nSizeY	= wndsize.GetHeight();	
		int nMouseY = mousepos.y;

		//--- set value ---//
		fPos = (float)255* ( (float)nMouseY / (float)nSizeY );
		if ( fPos < 0.0f )
			fPos = 0.0f;
		else if ( fPos > 255.0f )
			fPos = 255.0f;

		int nThisPos = (int)fPos;
		if ( nThisPos != m_LastPos )
		{
			pParent->SetValue( 255 - (int)fPos );
			m_LastPos = nThisPos;
		}

		//--- set volume ---//

		wxGetApp().Player.SetVolume((int)pParent->GetValue());
	}
}
