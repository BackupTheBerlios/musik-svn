/*
 *  BtnDown.h
 *
 *  Custom event to push to buttons to display a different
 *  image on a mouse down event.
 *
 *  Copyright (c) 2003 Casey Langen (casey@bak.rr.com)
 *	Contributors: Simon Windmill, Dustin Carter, Gunnar Roth, Wade Brainerd
 *
 *  See the file "license.txt" for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL WARRANTIES.
*/

#ifndef MUSIK_BTNDOWN_EVT
#define MUSIK_BTNDOWN_EVT

#include "wx/wxprec.h"
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif 

class CBtnDownEvt : public wxEvtHandler
{
public:
	CBtnDownEvt( wxBitmapButton *parent, wxBitmap *down, wxBitmap *up ){ pParent = parent; pBtnDown = down; pBtnUp = up; };
	~CBtnDownEvt(){};
	
	void OnLeftDown	( wxMouseEvent& event );
	void OnLeftUp	( wxMouseEvent& event );
	void OnMouseMove( wxMouseEvent& event );
	void OnEraseBackground(wxEraseEvent& ) {}
	DECLARE_EVENT_TABLE()
private:
	wxBitmapButton	*pParent;
	wxBitmap		*pBtnDown;
	wxBitmap		*pBtnUp;
};

#endif
