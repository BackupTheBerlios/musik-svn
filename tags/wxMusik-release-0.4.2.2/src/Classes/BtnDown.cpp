/*
 *  BtnDown.cpp
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
// For compilers that support precompilation, includes "wx/wx.h".
#include "myprec.h"
#include "BtnDown.h"

//-------------------//
//--- CBtnDownEvt ---//
//-------------------//
BEGIN_EVENT_TABLE(CBtnDownEvt, wxEvtHandler)
	EVT_LEFT_DOWN	(CBtnDownEvt::OnLeftDown) 
	EVT_LEFT_UP		(CBtnDownEvt::OnLeftUp)
	EVT_MOTION		(CBtnDownEvt::OnMouseMove)
//	EVT_ERASE_BACKGROUND(CBtnDownEvt::OnEraseBackground)
END_EVENT_TABLE()

void CBtnDownEvt::OnLeftDown( wxMouseEvent& event )
{
	pParent->SetBitmapLabel( *pBtnDown );
	pParent->Refresh();
	event.Skip();
}

void CBtnDownEvt::OnLeftUp( wxMouseEvent& event )
{
	pParent->SetBitmapLabel( *pBtnUp );
	pParent->Refresh();
	event.Skip();
}

void CBtnDownEvt::OnMouseMove( wxMouseEvent& event )
{
	if ( event.LeftIsDown() )
		pParent->SetBitmapLabel( *pBtnUp );
	event.Skip();
}
