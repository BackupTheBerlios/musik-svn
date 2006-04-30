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
#include "myprec.h"

#include "MusikEQCtrl.h"

//--- globals ---//
#include "../MusikGlobals.h"
#include "MUSIKEngine/MUSIKEngine/inc/equalizer.h"

BEGIN_EVENT_TABLE(CMusikEQCtrl, wxPanel)
	EVT_BUTTON			( BTN_RESET,		CMusikEQCtrl::OnClickReset		)
	EVT_CHECKBOX		( CHK_EQENABLE,		CMusikEQCtrl::OnToggleEQEnable	)
//	EVT_ERASE_BACKGROUND		( CMusikEQCtrl::OnEraseBackground )
END_EVENT_TABLE()

void CMusikEQCtrl::OnEraseBackground( wxEraseEvent& WXUNUSED(event) )
{	
	// empty => no background erasing to avoid flicker
}

CMusikEQCtrl::CMusikEQCtrl( wxWindow* pParent,MUSIKEqualizer *pEQ )
	: wxPanel( pParent, -1, wxDefaultPosition, wxDefaultSize, wxNO_BORDER |wxCLIP_CHILDREN )
{
    m_pEQ = pEQ;
	//-------------------------------------//
	//--- initialize the sizers			---//
	//-------------------------------------//
	pBandsSizer		= new wxFlexGridSizer	( 2, 18, 2, 2 );
	pOptionsSizer	= new wxBoxSizer		( wxHORIZONTAL );
	pMainSizer		= new wxBoxSizer		( wxVERTICAL );
 

    
	//-------------------------------------//
	//--- create the bands				---//
	//-------------------------------------//

    for ( int i = 0; i < m_pEQ->ChannelBands(MUSIKEqualizer::Bands::Left).Count() ; i++ )
	{
		slLeft.push_back(new CMusikEQGauge( this, MUSIK_FX_LEFT, i ));
		slLeft[i]->SetToolTip( wxString::Format( _( "Left Channel: %d hz" ), 
                                    m_pEQ->ChannelBands(MUSIKEqualizer::Bands::Left)[i].Frequency()) );
		pBandsSizer->Add( slLeft[i],1,wxEXPAND );
	}
	for ( int i = 0; i < m_pEQ->ChannelBands(MUSIKEqualizer::Bands::Right).Count(); i++ )
	{
		slRight.push_back(new CMusikEQGauge( this, MUSIK_FX_RIGHT, i ));
		slRight[i]->SetToolTip( wxString::Format( _( "Right Channel: %d hz" ),
                                    m_pEQ->ChannelBands(MUSIKEqualizer::Bands::Left)[i].Frequency()) );
		pBandsSizer->Add( slRight[i],1,wxEXPAND );
	}
	pBandsSizer->AddGrowableRow(0);
	pBandsSizer->AddGrowableRow(1);

	//-------------------------------------//
	//--- enableequalizer				---//
	//-------------------------------------//
	chkEQEnable = new wxCheckBox_NoFlicker( this, CHK_EQENABLE, _("Enable EQ") );
	chkEQEnable->SetValue( m_pEQ->Enabled());
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
	m_pEQ->Reset();
	SlidersFromBands();
    m_pEQ->ApplyChanges();
}

void CMusikEQCtrl::OnToggleEQEnable( wxCommandEvent& WXUNUSED(event) )
{
	wxGetApp().Prefs.bUseEQ = chkEQEnable->IsChecked();
    m_pEQ->Enable(wxGetApp().Prefs.bUseEQ);
}

void CMusikEQCtrl::SlidersFromBands()
{
    MUSIKEqualizer::Bands &lband = m_pEQ->ChannelBands(MUSIKEqualizer::Bands::Left);
	for ( size_t n = 0; n < lband.Count(); n++ )
	{
		slLeft[n]->SetValue( (int)( lband[n] * 50.0 ) );
		slLeft[n]->Colourize();
	}
    MUSIKEqualizer::Bands &rband = m_pEQ->ChannelBands(MUSIKEqualizer::Bands::Right);
    for ( size_t n = 0; n < rband.Count(); n++ )
    {
        slRight[n]->SetValue( (int)( rband[n] * 50.0 ) );
        slRight[n]->Colourize();
    }
}

void CMusikEQCtrl::BandsFromSliders()
{
    MUSIKEqualizer::Bands &lband = m_pEQ->ChannelBands(MUSIKEqualizer::Bands::Left);
    for ( size_t n = 0; n < lband.Count(); n++ )
	{
		lband[n] = (double)slLeft[n]->GetValue() / 50.0;
	}
    MUSIKEqualizer::Bands &rband = m_pEQ->ChannelBands(MUSIKEqualizer::Bands::Right);
    for ( size_t n = 0; n < rband.Count(); n++ )
    {
        rband[n] = (double)slRight[n]->GetValue() / 50.0;
    }
	m_pEQ->ApplyChanges();
}

