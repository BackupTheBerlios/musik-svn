/*
 *  MusikEQGauge.h
 *
 *  Subclassed wxGauge used by equalizer. Has special nifty functions.
 *
 *  Copyright (c) 2003 Casey Langen (casey@bak.rr.com)
 *	Contributors: Simon Windmill, Dustin Carter, Gunnar Roth, Wade Brainerd
 *
 *  See the file "license.txt" for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL WARRANTIES.
*/

#ifndef MUSIK_FX_GAUGE_H
#define MUSIK_FX_GAUGE_H

//--- wx ---//
#include "wx/wxprec.h"
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif 

enum EMUSIK_FX_CHANNEL
{
	MUSIK_FX_LEFT = 0,
	MUSIK_FX_RIGHT
};

class CMusikEQCtrl;

class CMusikEQGauge : public wxGauge
{
public:
	CMusikEQGauge( CMusikEQCtrl* parent, size_t nChannel, size_t nBandID );
	~CMusikEQGauge();
#ifdef __WXMSW__
	void OnPaint(wxPaintEvent& event);
#endif
	void OnEraseBackground ( wxEraseEvent& event );
	void OnLeftDown			( wxMouseEvent& event );
	void OnLeftUp			( wxMouseEvent& event );
	void OnMouseMove		( wxMouseEvent& event );
	void SetFromMousePos	( wxMouseEvent& event );

	void SetEQ();
	void SetPos( size_t m_Pos );
	void Colourize( );

	DECLARE_EVENT_TABLE()

private:
	bool m_Dragging;

	CMusikEQCtrl *m_Parent;
	size_t m_Channel;
	size_t m_BandID;

	//--- for the mouse events ---//
	wxPoint m_MousePos;
	wxSize m_WndSize;
	float m_Pos;
	float m_Temp;
	float m_Value;
};

#endif
