/*
 *  MusikFX.h
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

#ifndef MUSIKFX_H
#define MUSIKFX_H

#include "wx/wxprec.h"
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif 

#include <wx/config.h> 
#include <wx/confbase.h>
#include <wx/fileconf.h> 

class CMusikFX
{
public:
	void	SetLeftBands	( float data[18] );
	void	SetRightBands	( float data[18] );
	void	GetLeftBands	( float *data );
	void	GetRightBands	( float *data );
	void	MakeTable		( float samplerate );
	void	ResetBands		();
	void	InitEQ			();
	void	EndEQ			();
	void	ProcessSamples	( void *pBuffer, int length, int channels, int bitspersample );

	void	SaveBands		( wxFileConfig *pConfig );
	void	LoadBands		( wxFileConfig *pConfig );

	void	SetFrequency	( float newfreq );
	float	GetFrequency	( ) { return m_Frequency; }

private:
	float m_LeftBands[18];
	float m_RightBands[18];

	float	m_Frequency;
};

#endif
