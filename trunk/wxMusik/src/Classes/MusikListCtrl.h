/*
 *  MusikListCtrl.h
 *
 *  The "MusikListCtrl" control
 *	Base class for our ListCtrls
 *
 *  Copyright (c) 2003 Casey Langen (casey@bak.rr.com)
 *	Contributors: Simon Windmill, Dustin Carter, Gunnar Roth, Wade Brainerd
 *
 *  See the file "license.txt" for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL WARRANTIES.
*/
#ifndef MUSIKLISTCTRL_H
#define MUSIKLISTCTRL_H

#include "wx/wxprec.h"


#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif 
#include "wx/listctrl.h"


class CMusikListCtrl : public wxListCtrl
{
public:
	//--------------------------------//
	//--- constructor / destructor ---//
	//--------------------------------//
	CMusikListCtrl( wxWindow *parent, const wxWindowID id, const wxPoint& pos, const wxSize& size ,long style);
	~CMusikListCtrl(){};


	//--------------//
	//--- events ---//
	//--------------//

#ifdef __WXMSW__
	void OnEraseBackground(wxEraseEvent& event);
	void OnPaint(wxPaintEvent& event);
	void Freeze();
	void Thaw();
#endif

	DECLARE_EVENT_TABLE()
protected:
	virtual void DoSetSize(int x, int y,int width, int height,int sizeFlags);
	virtual bool OnRescaleColumns();
private:
#ifdef __WXMSW__
	int m_freezeCount;
#endif
};
#endif
