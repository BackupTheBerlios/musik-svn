/*
 *  MusikVolumeFrame.h
 *
 *  Frame that popups to adjust volume
 *  
 *  Copyright (c) 2003 Casey Langen (casey@bak.rr.com)
 *	Contributors: Simon Windmill, Dustin Carter, Gunnar Roth, Wade Brainerd
 *
 *  See the file "license.txt" for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL WARRANTIES.
*/

#ifndef MUSIK_VOLUME_FRAME_H
#define MUSIK_VOLUME_FRAME_H

#include "wx/wxprec.h"
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif 

//--- event handler for gauge ---//
#include "../Classes/GaugeSeek.h"

enum EMUSIK_VOL_OBJECTS
{
	MUSIK_SL_VOLUME = 0,
	MUSIK_CHK_CROSSFADE
};

class MusikVolumeFrame : public wxFrame
{
public:
	MusikVolumeFrame( wxFrame* pParent, wxPoint pos );
	~MusikVolumeFrame() { gSeek->PopEventHandler( true ); }

	wxGauge*		gSeek;		
	CGaugeSeekEvt*	pSeekEvt;


	void Close();

	DECLARE_EVENT_TABLE()
private:
	void OnActivate				( wxActivateEvent&	WXUNUSED(event)	);
	void OnClose				( wxCloseEvent&	WXUNUSED(event)	)	{ Close(); }
	void OnChar					( wxKeyEvent&		event	);

	wxFrame *parent;
};

#endif
