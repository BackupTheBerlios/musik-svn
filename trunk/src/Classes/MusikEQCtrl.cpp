/*
 *  MusikEQCtrl.cpp
 *
 *  Equalizer control. Used by MusikFXDialog.
 *
 *  Copyright (c) 2003 Casey Langen (casey@bak.rr.com)
 *	Contributors: Simon Windmill, Dustin Carter, Gunnar Roth, Wade Brainerd
 *
 *  See the file "license.txt" for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL WARRANTIES.
*/

//--- For compilers that support precompilation, includes "wx/wx.h". ---//
#include "wx/wxprec.h"

#include "MusikEQCtrl.h"

//--- globals ---//
#include "../MusikGlobals.h"

int freqs[] = {55,77,110,156,220,311,440,622,880,1244,1760,2489,3520,4978,7040,9956,14080,19912};

BEGIN_EVENT_TABLE(CMusikEQCtrl, wxPanel)
	EVT_BUTTON			( BTN_RESET,		CMusikEQCtrl::OnClickReset		)
	EVT_CHECKBOX		( CHK_EQENABLE,		CMusikEQCtrl::OnToggleEQEnable	)
//	EVT_ERASE_BACKGROUND		( CMusikEQCtrl::OnEraseBackground )
END_EVENT_TABLE()

void CMusikEQCtrl::OnEraseBackground( wxEraseEvent& WXUNUSED(event) )
{	
	// empty => no background erasing to avoid flicker
}

CMusikEQCtrl::CMusikEQCtrl( wxWindow* pParent )
	: wxPanel( pParent, -1, wxDefaultPosition, wxDefaultSize, wxNO_BORDER |wxCLIP_CHILDREN| wxTRANSPARENT_WINDOW )
{
	//-------------------------------------//
	//--- initialize the sizers			---//
	//-------------------------------------//
	pBandsSizer		= new wxFlexGridSizer	( 2, 18, 2, 2 );
	pOptionsSizer	= new wxBoxSizer		( wxHORIZONTAL );
	pMainSizer		= new wxBoxSizer		( wxVERTICAL );

	//-------------------------------------//
	//--- create the bands				---//
	//-------------------------------------//
	for ( int i = 0; i < 18; i++ )
	{
		slLeft[i] = new CMusikEQGauge( this, MUSIK_FX_LEFT, i );
		slLeft[i]->SetToolTip( wxString::Format( _( "Left Channel: %d hz" ), freqs[i]) );
		pBandsSizer->Add( slLeft[i],1,wxEXPAND );
	}
	for ( int i = 0; i < 18; i++ )
	{
		slRight[i] = new CMusikEQGauge( this, MUSIK_FX_RIGHT, i );
		slRight[i]->SetToolTip( wxString::Format( _( "Right Channel: %d hz" ), freqs[i]) );
		pBandsSizer->Add( slRight[i],1,wxEXPAND );
	}
	pBandsSizer->AddGrowableRow(0);
	pBandsSizer->AddGrowableRow(1);

	//-------------------------------------//
	//--- enableequalizer				---//
	//-------------------------------------//
	chkEQEnable = new wxCheckBox_NoFlicker( this, CHK_EQENABLE, _("Enable EQ") );
	chkEQEnable->SetValue( wxGetApp().Prefs.bUseEQ );
	pOptionsSizer->Add( chkEQEnable, 1, wxALIGN_CENTER | wxALIGN_CENTER_VERTICAL );

	//-------------------------------------//
	//--- lock channels					---//
	//-------------------------------------//
	chkLock = new wxCheckBox_NoFlicker( this, -1, _("Lock channels") );
	chkLock->SetValue( true );
	pOptionsSizer->Add( chkLock, 1, wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL );

	//-------------------------------------//
	//--- reset bands					---//
	//-------------------------------------//
	btnReset = new wxButton_NoFlicker( this, BTN_RESET, _("Reset bands") );
	pOptionsSizer->Add( btnReset, 1, wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL );

	pMainSizer->Add( pOptionsSizer, 0, wxEXPAND | wxBOTTOM, 4 );
	pMainSizer->Add( pBandsSizer, 1 );

	SetSizerAndFit( pMainSizer );
	
	//-------------------------------------//
	//--- grab the band information.	---//
	//-------------------------------------//
	SlidersFromBands();
}

CMusikEQCtrl::~CMusikEQCtrl()
{
}

void CMusikEQCtrl::OnClickReset( wxCommandEvent& WXUNUSED(event) )
{
	g_FX.ResetBands();
	SlidersFromBands();
}

void CMusikEQCtrl::OnToggleEQEnable( wxCommandEvent& WXUNUSED(event) )
{
	wxGetApp().Prefs.bUseEQ = chkEQEnable->IsChecked();
	wxGetApp().Player.ActivateDSP();
}

void CMusikEQCtrl::SlidersFromBands()
{
	g_FX.GetLeftBands( &ldata[0] );
	g_FX.GetRightBands( &rdata[0] );

	for ( int n = 0; n < 18; n++ )
	{
		slLeft[n]->SetValue( (int)( ldata[n] * 50.0f ) );
		slLeft[n]->Colourize();
		slRight[n]->SetValue( (int)( rdata[n] * 50.0f ) );
		slRight[n]->Colourize();
	}
}

void CMusikEQCtrl::BandsFromSliders()
{
	for ( int n = 0; n < 18; n++ )
	{
		ldata[n] = (float)slLeft[n]->GetValue() / 50.0f;
		rdata[n] = (float)slRight[n]->GetValue() / 50.0f;
	}

	g_FX.SetLeftBands( ldata );
	g_FX.SetRightBands( rdata );
	g_FX.MakeTable( 44100 );
}
