/*
 *  MusikEQCtrl.h
 *
 *  Equalizer control. Used by MusikFXDialog.
 *
 *  Copyright (c) 2003 Casey Langen (casey@bak.rr.com)
 *	Contributors: Simon Windmill, Dustin Carter, Gunnar Roth, Wade Brainerd
 *
 *  See the file "license.txt" for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL WARRANTIES.
*/

#ifndef MUSIK_EQ_CTRL_H
#define MUSIK_EQ_CTRL_H

#include "myprec.h"
#ifndef WX_PRECOMP
	#include "wx/wx.h"
	#include <vector>
#endif 
#include "../MusikUtils.h"
#include "MusikEQGauge.h"

enum EMUSIK_EQ_OBJECT_ID
{
	BTN_RESET = wxID_HIGHEST,
	CHK_EQENABLE
};
class MUSIKEqualizer;
class CMusikEQCtrl : public wxPanel
{
public:

	CMusikEQCtrl( wxWindow* pParent,MUSIKEqualizer *pEQ );
	~CMusikEQCtrl();

	//-----------------//
	//--- functions ---//
	//-----------------//
	void OnToggleEQEnable	( wxCommandEvent& WXUNUSED(event) );
	void OnClickReset		( wxCommandEvent& WXUNUSED(event) );
	void SlidersFromBands	();
	void BandsFromSliders	();
	void OnEraseBackground ( wxEraseEvent& event );


    //------------------//
	//--- eq sliders ---//
	//------------------//
    std::vector<CMusikEQGauge*>	 slLeft;
	std::vector<CMusikEQGauge*>  slRight;

	//------------------//
	//--- eq options ---//
	//------------------//
	wxCheckBox_NoFlicker				*chkLock;
	wxCheckBox_NoFlicker				*chkEQEnable;
	wxButton_NoFlicker				*btnReset;

	//--------------//
	//--- sizers ---//
	//--------------//
	wxFlexGridSizer			*pBandsSizer;
	wxBoxSizer				*pOptionsSizer;
	wxBoxSizer				*pMainSizer;

    MUSIKEqualizer  *m_pEQ;    
	DECLARE_EVENT_TABLE()
};

#endif
