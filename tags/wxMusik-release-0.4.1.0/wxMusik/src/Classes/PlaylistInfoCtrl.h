/*
 *  PlaylistInfoCtrl.h
 *
 *  The "Playlist Info" information
 *	This is the control visible on the main frame between
 *	the activity area and the playlist.
 *
 *  Copyright (c) 2003 Casey Langen (casey@bak.rr.com)
 *	Contributors: Simon Windmill, Dustin Carter, Gunnar Roth, Wade Brainerd
 *
 *  See the file "license.txt" for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL WARRANTIES.
*/

#ifndef MUSIK_PLAYLISTINFO_CTRL
#define MUSIK_PLAYLISTINFO_CTRL

#include "wx/wxprec.h"
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif 
#include "../MusikUtils.h"
class IPlaylistInfo;

class CPlaylistInfoCtrl : public wxPanel
{
public:
	CPlaylistInfoCtrl( wxWindow *pParent ,IPlaylistInfo *pIPlaylistInfo);

	wxStaticText_NoFlicker* stTotal;		
	wxStaticText_NoFlicker* stTotalVal;
	wxStaticText_NoFlicker* stPlaylist;		
	wxStaticText_NoFlicker* stPlaylistVal;
	wxStaticText_NoFlicker* stRuntime;	
	wxStaticText_NoFlicker* stRuntimeVal;	
	wxStaticText_NoFlicker* stFilesize;
	wxStaticText_NoFlicker* stFilesizeVal;

	wxBoxSizer *pSizer;
	wxBoxSizer *pVert;

	void Update();
	void OnEraseBackground(wxEraseEvent& event);
	DECLARE_EVENT_TABLE()

private:
	IPlaylistInfo* m_pIPlaylistInfo;
};

#endif
