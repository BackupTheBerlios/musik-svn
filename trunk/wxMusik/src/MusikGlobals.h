/*
 *  MusikGlobals.h
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

#ifndef MUSIKGLOBALS_H
#define MUSIKGLOBALS_H

//--------------------------------------//
//--- ignore stupid warning messages ---//
//---  so we can see whats going on  ---//
//--------------------------------------//
/*
#ifdef _MSC_VER
	#pragma warning(disable : 4018)		//--- '<' signed/unsigned mismatch										---//
	#pragma warning(disable : 4288)		//--- nonstandard extension used : 'i'									---//
	#pragma warning(disable : 4238)		//--- nonstandard extension used : class rvalue as lvalue				---//
	#pragma warning(disable : 4245)		//--- conversion from ID3_TagType to flags_t signed/unsigned mismatch	---//
	#pragma warning(disable : 4267)		//--- '=' conversion from size_t to long								---//
	#pragma warning(disable : 4311)		//--- type cast from void* to long										---//
	#pragma warning(disable : 4700)		//--- local variable used without having been initialized				---//
	#pragma warning(disable : 4701)		//--- local variable used without being intialized						---//
	#pragma warning(disable : 4702)		//--- unreachable code													---//
	#pragma warning(disable : 4805)		//--- '!=' unsafe mix of type bool and type int							---//
	#pragma warning(disable : 4389)		//--- '==' signed/unsigned mismatch										---//
	#pragma warning(disable : 4390)		//--- empty controlled statement found; is this the intent?				---//
	#pragma warning(disable : 4996)		//--- declared deprecated												---//
#endif
*/
#ifdef _MSC_VER
//#pragma warning(disable : 4800)		//--- 'int' forcing bool to true or false								---//
#endif
#include "MusikDefines.h"
#include "Frames/MusikFrame.h"


//---------------------------//
//--- pointer to main dlg ---//
//---------------------------//
class	MusikFrame;
extern	MusikFrame *g_MusikFrame;

//------------------------//
//--- internal objects ---//
//------------------------//
#include <wx/image.h>
#include <wx/imaglist.h>

#include "MusikApp.h"
//forward declarations
class CMusikPrefs;
class CMusikPaths;
class MusikFaderThread;

extern CMusikPaths			g_Paths;			
extern MusikSongIdArray		g_thePlaylist;	
extern MusikFaderThread*	g_FaderThread;
extern wxImageList*			g_SourcesImages; 
extern wxImageList*			g_NullImageList;
extern wxImageList*			g_RatingImages;


//----------------//
//--- controls ---//
//----------------//

class CActivityAreaCtrl;
class CPlaylistBox;
class CSourcesBox;

extern CActivityAreaCtrl	*g_ActivityAreaCtrl;
extern CPlaylistBox		*g_PlaylistBox;	
extern CSourcesBox			*g_SourcesCtrl;	



//------------//
//--- misc ---//
//------------//
extern bool g_SelInProg;
extern bool g_TimeSeeking;
extern bool g_DisablePlacement;

//-------------//
//--- fonts ---//
//-------------//
extern wxFont g_fntArtist;
extern wxFont g_fntSong;
extern wxFont g_fntTime;
extern wxFont g_fntRegular;
extern wxFont g_fntBold;
extern wxFont g_fntListBold;



#endif	//MUSIKGLOBALS_H
