/*
 *  Tunage.h
 *
 *  Musik's system for updating websites, files, etc.
 *  with currently-playing info
 *  
 *  Copyright (c) 2003 Casey Langen (casey@bak.rr.com)
 *	Contributors: Simon Windmill, Dustin Carter, Gunnar Roth, Wade Brainerd
 *
 *  See the file "license.txt" for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL WARRANTIES.
*/

#ifndef TUNAGE_H
#define TUNAGE_H

#include "wx/wxprec.h"
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include "wx/socket.h"

#include "MusikLibrary.h"

class CTunage
{
public:
	CTunage();
	~CTunage();

	void Execute(const CMusikSong& song );
	void Stopped();

private:
	CMusikSong m_Song;

	void PostURL();
	void WriteFile();
	void RunApp();

	void ParseTags( wxString& str );
};
#endif
