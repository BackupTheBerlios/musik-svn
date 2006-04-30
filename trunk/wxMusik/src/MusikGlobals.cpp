/*
 *  MusikGlobals.cpp
 *
 *  Global objects that are common to (most) all classes and
 *  derived / inherited objects.
 *  
 *  Copyright (c) 2003 Casey Langen (casey@bak.rr.com)
 *	Contributors: Simon Windmill, Dustin Carter, Gunnar Roth, Wade Brainerd
 *
 *  See the file "license.txt" for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL WARRANTIES.
*/
// For compilers that support precompilation, includes "wx/wx.h".
#include "myprec.h"

//----------------//
//--- includes ---//
//----------------//
#include "MusikGlobals.h"


//----------------------------//
//--- pointer to MusikFrame ---//
//----------------------------//
MusikFrame *g_MusikFrame;



//------------------------//
//--- internal objects ---//
//------------------------//

CMusikPaths			g_Paths;
MusikSongIdArray		g_thePlaylist;	


MusikFaderThread*	g_FaderThread;
wxImageList*		g_SourcesImages; 
wxImageList*		g_NullImageList;
wxImageList*		g_RatingImages;


//----------------//
//--- controls ---//
//----------------//
CActivityAreaCtrl	*g_ActivityAreaCtrl;
CPlaylistBox		*g_PlaylistBox;
CSourcesBox			*g_SourcesCtrl;


//-------------//
//--- fonts ---//
//-------------//
wxFont g_fntArtist;
wxFont g_fntSong;
wxFont g_fntTime;
wxFont g_fntRegular;
wxFont g_fntBold;
wxFont g_fntListBold;

//------------//
//--- misc ---//
//------------//
bool g_SelInProg;
bool g_TimeSeeking;
bool g_DisablePlacement;

