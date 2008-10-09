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

#include "wx/socket.h"

class CMusikSong;

class MusikScrobbler;

class CTunage
{
public:
	CTunage();
	~CTunage();
                         
	void Started(const CMusikSong& song );
	void Paused();
	void Resumed();
	void Stopped();

private:

	void PostURL(const CMusikSong& song );
	void WriteFile(const CMusikSong& song );
	void RunApp(const CMusikSong& song );

	void ParseTags( wxString& str ,const CMusikSong& song );

	MusikScrobbler * m_pScrobbler;
};
#endif
