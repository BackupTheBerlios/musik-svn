/*
 *  GaugeSeek.h
 *
 *  Custom event to allow mouse drag seeking for
 *  If a wxGA_HORIZONTAL is specified, it will be assumed to be the seek bar. 0-100
 *	If a wxGA_VERTICAL is specified, it will be assumed to be the volume control. 0-255
 *
 *  Copyright (c) 2003 Casey Langen (casey@bak.rr.com)
 *	Contributors: Simon Windmill, Dustin Carter, Gunnar Roth, Wade Brainerd
 *
 *  See the file "license.txt" for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL WARRANTIES.
*/

#ifndef MUSIK_GAUGESEEK_EVT
#define MUSIK_GAUGESEEK_EVT

#include "wx/wxprec.h"
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif 

class CGaugeSeekEvt : public wxEvtHandler
{
public:
	CGaugeSeekEvt( wxGauge *parent, long style = wxGA_HORIZONTAL );
	~CGaugeSeekEvt(){}
	
	void OnLeftDown			( wxMouseEvent& event );
	void OnLeftUp			( wxMouseEvent& event );
	void OnMouseMove		( wxMouseEvent& event );
	void SetFromMousePos	( wxMouseEvent& event );
	void OnEraseBackground  ( wxEraseEvent& event );
	void OnRightDown		(wxMouseEvent& event);
#ifdef __WXMSW__
	void OnPaint(wxPaintEvent& event);
#endif
	DECLARE_EVENT_TABLE()
private:
	bool m_Dragging;
	int		m_LastPos;
	long	lType;
	wxGauge	*pParent;
	float fPos; //--- we'll end up using the last known fPos value on cleanup ---//
};

#endif
