/*
 *  ActivityBoxThreads.h
 *
 *  Threads related to ActivityBox.h / cpp
 *  
 *  Copyright (c) 2003 Casey Langen (casey@bak.rr.com)
 *	Contributors: Simon Windmill, Dustin Carter, Gunnar Roth, Wade Brainerd
 *
 *  See the file "license.txt" for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL WARRANTIES.
*/

#ifndef MUSIK_ACTIVITYBOX_THREADS_H
#define MUSIK_ACTIVITYBOX_THREADS_H

//--- wx ---//
#include "wx/wxprec.h"
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif 

#include "wx/thread.h"

#include "../Classes/MusikLibrary.h"
#include "../Classes/ActivityBox.h"

enum EACTIVITY_RENAME_MODE
{
	ACTIVITY_RENAME_ACTIVITY = 0,  //--- rename activities, e.g. artist -> An Artist ---//
	ACTIVITY_RENAME_SONGS          //--- apply activity to songs ---//
};

//-------------------------------------------------//
//--- NOTE: post all events to the m_ParentBox.	---//
//--- It will relay events to g_MusikFrame		---//
//-------------------------------------------------//
class MusikActivityRenameThread : public wxThread
{
public:
	MusikActivityRenameThread( CActivityBox* parent_box, int mode, wxString newvalue );

	virtual void *Entry();
	virtual void OnExit();

private:
	CActivityBox*	m_ParentBox;
	int				m_Mode;
	int				m_Type;
	wxString		m_TypeStr;
	CSongMetaData::StringData m_Replace;

	CMusikSongArray m_Songs;
};


#endif
