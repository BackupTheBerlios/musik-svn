/*
 *  MusikFXDialog.h
 *
 *  Front end to Musik's special FX engine.
 *
 *  Uses FMOD sound API and Shibatch EQ.
 *  Information about FMOD is available at http://www.fmod.org
 *	Information about Shibatch is available at http://shibatch.sourceforge.net
 *  
 *  Copyright (c) 2003 Casey Langen (casey@bak.rr.com)
 *	Contributors: Simon Windmill, Dustin Carter, Gunnar Roth, Wade Brainerd
 *
 *  See the file "license.txt" for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL WARRANTIES.
*/

#ifndef MusikFXDialog_H
#define MusikFXDialog_H

//--- wx ---//
#include "wx/wxprec.h"
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif 

#include "../Classes/MusikEQCtrl.h"

enum EMUSIK_FX_OBJECT_ID
{
	CHK_PITCHENABLE = wxID_HIGHEST
};

class MusikFXDialog : public wxDialog
{
public:
	MusikFXDialog( wxWindow* pParent, const wxString &sTitle, const wxPoint &pos, const wxSize &size );

	//--------------//
	//--- events ---//
	//--------------//
	void OnClose			( wxCloseEvent& WXUNUSED(event) );
	void OnSlidePitch		( wxScrollEvent& event );
	void OnRightClick		( wxContextMenuEvent& event );
	void OnTogglePitchEnable	( wxCommandEvent& WXUNUSED(event) );

	void OnEraseBackground ( wxEraseEvent& event );

	//------------------//
	//--- eq sliders ---//
	//-------------------------//
	//--- utility functions ---//
	//-------------------------//
	void Close();
	
	//----------------//
	//--- controls ---//
	//----------------//
	CMusikEQCtrl			*pEQ;
	wxSlider				*slPitch;
	wxCheckBox				*chkPitchEnable;
	
	//--------------//
	//--- sizers ---//
	//--------------//
	wxBoxSizer				*vsMain;
	wxBoxSizer				*hsPitch;
	
	DECLARE_EVENT_TABLE()

private:


};

#endif
