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

#include <wx/snglinst.h>
#include <wx/ipc.h>
#include <wx/intl.h>
#include <wx/app.h>


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
#if wxUSE_DEBUGREPORT
	//! fatal exeption handling
	void OnFatalException();
#endif

	void OnPlayFiles(const wxArrayString &aFilelist);
	enum eCopyFileOptions { CopyDefault = 0,CopyPreserveDirectories = 1};
	void CopyFiles(const MusikSongIdArray &songs, eCopyFileOptions options = CopyDefault);
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
