/*
 *  MusikTagThreads.h
 *
 *  Threads related to MusikTagFrame.h / cpp
 *  
 *  Copyright (c) 2003 Casey Langen (casey@bak.rr.com)
 *	Contributors: Simon Windmill, Dustin Carter, Gunnar Roth, Wade Brainerd
 *
 *  See the file "license.txt" for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL WARRANTIES.
*/

#ifndef MUSIK_TAG_THREADS_H
#define MUSIK_TAG_THREADS_H

//--- wx ---//
#include "wx/wxprec.h"
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif 

#include "wx/thread.h"

#include "../Classes/MusikLibrary.h"

class MusikTagApplyThread : public wxThread
{
public:
	MusikTagApplyThread(wxEvtHandler *dest ,const CMusikSongArray & songs);
	virtual void *Entry();
	virtual void OnExit();

private:
	const CMusikSongArray & m_Songs;
	wxEvtHandler * m_pPostDest;
};

#endif

