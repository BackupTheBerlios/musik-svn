/*
 *  MusikLibraryThreads.h
 *
 *  Threads related to MusikLibraryFrame.h / cpp
 *  
 *  Copyright (c) 2003 Casey Langen (casey@bak.rr.com)
 *	Contributors: Simon Windmill, Dustin Carter, Gunnar Roth, Wade Brainerd
 *
 *  See the file "license.txt" for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL WARRANTIES.
*/

#ifndef MUSIK_LIBRARY_THREADS_H
#define MUSIK_LIBRARY_THREADS_H

//--- wx ---//
#include "wx/wxprec.h"
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif 

#include "wx/thread.h"
#include <wx/dir.h>
#include <wx/filename.h>

enum MUSIK_LIBRARY_THREAD_COMMAND
{
	SET_CURRENT = 1,
	SET_TOTAL,
	SET_NEW
};
class wxMusicTraverser;

class MusikScanNewThread : public wxThread
{
public:
	MusikScanNewThread(wxEvtHandler *pParent,wxArrayString & m_refFiles);

	virtual void *Entry();
	virtual void OnExit();
	wxEvtHandler *Parent() {return m_pParent;}
protected:

	void GetMusicDirs (  const wxArrayString & aDirs, wxArrayString & aFiles );
	void GetMusicDir  ( const wxString & sDir, wxArrayString & aFiles );


	wxArrayString & m_refFiles;
	wxEvtHandler *m_pParent;
	friend class wxMusicTraverser;
};

class MusikUpdateLibThread : public MusikScanNewThread
{
public:
	MusikUpdateLibThread(wxEvtHandler *pParent, wxArrayString* del , wxArrayString & m_refFiles, bool bCompleteRebuild );

	virtual void *Entry();
	virtual void OnExit();

private:
	wxArrayString* m_pPathesToAdd;
	wxArrayString* m_pPathesDel;
	bool m_bCompleteRebuild;
};



class MusikPurgeLibThread : public wxThread
{
public:
	MusikPurgeLibThread(wxEvtHandler *pParent);

	virtual void *Entry();
	virtual void OnExit();
	wxEvtHandler *Parent() {return m_pParent;}
protected:
   wxEvtHandler *m_pParent;
};

#endif

