/*
 *  MusikFX.cpp
 *
 *  Musik's special FX engine.
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


#include "wx/wxprec.h"	   // For compilers that support precompilation, includes "wx/wx.h".

#include "MusikFX.h"
#include "../MusikGlobals.h"

//-----------------------------//
//--- Stuff for Shibatch EQ ---//
//-----------------------------//
#include "../shibatch/paramlist.hpp"

extern void equ_init(int wb);
extern void equ_makeTable(float *lbc,float *rbc,paramlist *,float fs);
extern void equ_quit(void);
extern int equ_modifySamples(char *buf,int nsamples,int nch,int bps);
extern void equ_clearbuf(int,int);
static paramlist paramroot;

void CMusikFX::SetLeftBands( float data[18] )
{
	for ( int n = 0; n < 18; n++ )
		m_LeftBands[n] = data[n];
}

void CMusikFX::SetRightBands( float data[18] )
{
	for ( int n = 0; n < 18; n++ )
		m_RightBands[n] = data[n];
}

void CMusikFX::GetLeftBands( float *data )
{
	for ( int n = 0; n < 18; n++ )
		data[n] = m_LeftBands[n];
}

void CMusikFX::GetRightBands( float *data )
{
	for ( int n = 0; n < 18; n++ )
		data[n] = m_RightBands[n];
}

void CMusikFX::ResetBands()
{
	for ( int n = 0; n < 18; n++ )
	{
		m_LeftBands[n] = 1.0f;
		m_RightBands[n] = 1.0f;
	}
	MakeTable( 44100 );
}

void CMusikFX::InitEQ()
{
	equ_init( 14 );		//--- no one knows why, 14 is the magic number ---//
	MakeTable( 44100 );
}

void CMusikFX::EndEQ()
{
	equ_quit();
}

void CMusikFX::MakeTable( float samplerate )
{
	equ_makeTable( m_LeftBands, m_RightBands, &paramroot, samplerate );
}

void CMusikFX::ProcessSamples( void *pBuffer, int length, int channels, int bitspersample )
{
	equ_modifySamples( (char*)pBuffer, length, channels, bitspersample );
}

///////////////////////////////////////////////////////////////////////////////

void CMusikFX::LoadBands( wxFileConfig *pConfig )
{
	double bands[18];
	pConfig->Read( wxT( "EQL0" ),		&bands[0],				1.0					);
	pConfig->Read( wxT( "EQL1" ),		&bands[1],				1.0					);
	pConfig->Read( wxT( "EQL2" ),		&bands[2],				1.0					);
	pConfig->Read( wxT( "EQL3" ),		&bands[3],				1.0					);
	pConfig->Read( wxT( "EQL4" ),		&bands[4],				1.0					);
	pConfig->Read( wxT( "EQL5" ),		&bands[5],				1.0					);
	pConfig->Read( wxT( "EQL6" ),		&bands[6],				1.0					);
	pConfig->Read( wxT( "EQL7" ),		&bands[7],				1.0					);
	pConfig->Read( wxT( "EQL8" ),		&bands[8],				1.0					);
	pConfig->Read( wxT( "EQL9" ),		&bands[9],				1.0					);
	pConfig->Read( wxT( "EQL10" ),		&bands[10],				1.0					);
	pConfig->Read( wxT( "EQL11" ),		&bands[11],				1.0					);
	pConfig->Read( wxT( "EQL12" ),		&bands[12],				1.0					);
	pConfig->Read( wxT( "EQL13" ),		&bands[13],				1.0					);
	pConfig->Read( wxT( "EQL14" ),		&bands[14],				1.0					);
	pConfig->Read( wxT( "EQL15" ),		&bands[15],				1.0					);
	pConfig->Read( wxT( "EQL16" ),		&bands[16],				1.0					);
	pConfig->Read( wxT( "EQL17" ),		&bands[17],				1.0					);
	for ( int n = 0; n < 18; n++ )
		m_LeftBands[n] = (float)bands[n];
	
	pConfig->Read( wxT( "EQR0" ),		&bands[0],				1.0					);
	pConfig->Read( wxT( "EQR1" ),		&bands[1],				1.0					);
	pConfig->Read( wxT( "EQR2" ),		&bands[2],				1.0					);
	pConfig->Read( wxT( "EQR3" ),		&bands[3],				1.0					);
	pConfig->Read( wxT( "EQR4" ),		&bands[4],				1.0					);
	pConfig->Read( wxT( "EQR5" ),		&bands[5],				1.0					);
	pConfig->Read( wxT( "EQR6" ),		&bands[6],				1.0					);
	pConfig->Read( wxT( "EQR7" ),		&bands[7],				1.0					);
	pConfig->Read( wxT( "EQR8" ),		&bands[8],				1.0					);
	pConfig->Read( wxT( "EQR9" ),		&bands[9],				1.0					);
	pConfig->Read( wxT( "EQR10" ),		&bands[10],				1.0					);
	pConfig->Read( wxT( "EQR11" ),		&bands[11],				1.0					);
	pConfig->Read( wxT( "EQR12" ),		&bands[12],				1.0					);
	pConfig->Read( wxT( "EQR13" ),		&bands[13],				1.0					);
	pConfig->Read( wxT( "EQR14" ),		&bands[14],				1.0					);
	pConfig->Read( wxT( "EQR15" ),		&bands[15],				1.0					);
	pConfig->Read( wxT( "EQR16" ),		&bands[16],				1.0					);
	pConfig->Read( wxT( "EQR17" ),		&bands[17],				1.0					);
	for ( int n = 0; n < 18; n++ )
		m_RightBands[n] = (float)bands[n];
}

void CMusikFX::SaveBands( wxFileConfig *pConfig )
{
	pConfig->Write( wxT( "EQL0" ),		(double)m_LeftBands[0]	);
	pConfig->Write( wxT( "EQL1" ),		(double)m_LeftBands[1]	);
	pConfig->Write( wxT( "EQL2" ),		(double)m_LeftBands[2]	);
	pConfig->Write( wxT( "EQL3" ),		(double)m_LeftBands[3]	);
	pConfig->Write( wxT( "EQL4" ),		(double)m_LeftBands[4]	);
	pConfig->Write( wxT( "EQL5" ),		(double)m_LeftBands[5]	);
	pConfig->Write( wxT( "EQL6" ),		(double)m_LeftBands[6]	);
	pConfig->Write( wxT( "EQL7" ),		(double)m_LeftBands[7]	);
	pConfig->Write( wxT( "EQL8" ),		(double)m_LeftBands[8]	);
	pConfig->Write( wxT( "EQL9" ),		(double)m_LeftBands[9]	);
	pConfig->Write( wxT( "EQL10" ),		(double)m_LeftBands[10]	);
	pConfig->Write( wxT( "EQL11" ),		(double)m_LeftBands[11]	);
	pConfig->Write( wxT( "EQL12" ),		(double)m_LeftBands[12]	);
	pConfig->Write( wxT( "EQL13" ),		(double)m_LeftBands[13]	);
	pConfig->Write( wxT( "EQL14" ),		(double)m_LeftBands[14]	);
	pConfig->Write( wxT( "EQL15" ),		(double)m_LeftBands[15]	);
	pConfig->Write( wxT( "EQL16" ),		(double)m_LeftBands[16]	);
	pConfig->Write( wxT( "EQL17" ),		(double)m_LeftBands[17]	);
	
	pConfig->Write( wxT( "EQR0" ),		(double)m_RightBands[0]	);
	pConfig->Write( wxT( "EQR1" ),		(double)m_RightBands[1]	);
	pConfig->Write( wxT( "EQR2" ),		(double)m_RightBands[2]	);
	pConfig->Write( wxT( "EQR3" ),		(double)m_RightBands[3]	);
	pConfig->Write( wxT( "EQR4" ),		(double)m_RightBands[4]	);
	pConfig->Write( wxT( "EQR5" ),		(double)m_RightBands[5]	);
	pConfig->Write( wxT( "EQR6" ),		(double)m_RightBands[6]	);
	pConfig->Write( wxT( "EQR7" ),		(double)m_RightBands[7]	);
	pConfig->Write( wxT( "EQR8" ),		(double)m_RightBands[8]	);
	pConfig->Write( wxT( "EQR9" ),		(double)m_RightBands[9]	);
	pConfig->Write( wxT( "EQR10" ),		(double)m_RightBands[10] );
	pConfig->Write( wxT( "EQR11" ),		(double)m_RightBands[11] );
	pConfig->Write( wxT( "EQR12" ),		(double)m_RightBands[12] );
	pConfig->Write( wxT( "EQR13" ),		(double)m_RightBands[13] );
	pConfig->Write( wxT( "EQR14" ),		(double)m_RightBands[14] );
	pConfig->Write( wxT( "EQR15" ),		(double)m_RightBands[15] );
	pConfig->Write( wxT( "EQR16" ),		(double)m_RightBands[16] );
	pConfig->Write( wxT( "EQR17" ),		(double)m_RightBands[17] );	
}

void CMusikFX::SetFrequency( float newfreq )
{
	m_Frequency = newfreq;
	wxGetApp().Player.SetFrequency();
}
