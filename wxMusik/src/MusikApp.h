/*
 *  MusikApp.h
 *
 *  Application start point
 *  
 *  Copyright (c) 2003 Casey Langen (casey@bak.rr.com)
 *	Contributors: Simon Windmill, Dustin Carter, Gunnar Roth, Wade Brainerd
 *
 *  See the file "license.txt" for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL WARRANTIES.
*/

#ifndef MUSIK_APP_H
#define MUSIK_APP_H

#include "wx/wxprec.h"
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif 
#include <wx/snglinst.h>
#include <wx/ipc.h>
#include "wx/intl.h"

#include "Classes/MusikLibrary.h"
#include "Classes/MusikPlayer.h"
#include "Classes/MusikPrefs.h"
#include "Classes/WebServer.h"

class MusikAppServer: public wxServer
{
public:
	virtual wxConnectionBase * OnAcceptConnection(const wxString& topic);
};
class MusikApp : public wxApp
{
public:
	MusikApp()
		:WebServer(&Player)
	{
	}
	~MusikApp() 
	{
		delete m_pSingleInstanceChecker;
	}
	virtual bool OnInit();
	virtual int OnExit();
	void OnPlayFiles(const wxArrayString &aFilelist);
	void CopyFiles(const CMusikSongArray &songs);
public:
	CMusikPrefs			Prefs;
	CMusikLibrary		Library;
	CMusikPlayer		Player;
	CMusikWebServer		WebServer;
private:
	wxString ReadOldVersionFile();
	void CheckOldVersion();

	wxSingleInstanceChecker *m_pSingleInstanceChecker;
	MusikAppServer *m_pServer;
	wxLocale m_locale; // locale we'll be using
};

DECLARE_APP(MusikApp)

#endif
