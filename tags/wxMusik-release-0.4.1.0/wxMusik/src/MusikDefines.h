/*
 *  MusikDefines.h
 *
 *  Global Defines and enums
 *  
 *  Copyright (c) 2003 Casey Langen (casey@bak.rr.com)
 *	Contributors: Simon Windmill, Dustin Carter, Gunnar Roth, Wade Brainerd
 *
 *  See the file "license.txt" for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL WARRANTIES.
*/

#ifndef MUSIKDEFINES_H
#define MUSIKDEFINES_H

#include "MusikVersion.h"

// wx 2.4.2 compatiblity macros
#if wxVERSION_NUMBER >= 2500  
#define WXSYSTEMCOLOUR(x)	wxTheColourDatabase->Find(x)
#else
#define WXSYSTEMCOLOUR(x)	*wxTheColourDatabase->FindColour(x)
#endif


#if defined( __WXGTK__ ) || defined( __WXMAC__)
#define WXLISTCTRL_SETITEMSTATE_IS_BUGGY 1
#endif
//#if defined(__WXGTK__) || defined( __WXMAC__)
#define WXMUSIK_BUGWORKAROUND_LISTCTRL_CONTEXTMENU 1
//#endif

#ifndef wxFULL_REPAINT_ON_RESIZE
#define wxFULL_REPAINT_ON_RESIZE 0
#endif


//---------------//
//--- defines ---//
//---------------//
#define MUSIK_DND_ABOVE			0
#define MUSIK_DND_BELOW			1
#define MUSIK_ACTIVITYBOX1		3000
#define MUSIK_ACTIVITYBOX2		3001
#define	MUSIK_ACTIVITYBOX3		3002
#define	MUSIK_ACTIVITYBOX4		3003
#define MUSIK_NOWPLAYING_TIMER	3011
#define MUSIK_ACTIVITYCTRL		3012
#define	MUSIK_PICTURE_BOX		3013
#define MUSIK_SOURCES_LISTCTRL	3015
#define MUSIK_SOURCES			3016
#define MUSIK_PLAYLIST			3017

#define MUSIKAPPNAME			wxT("wxMusik")
#define MUSIKAPPNAME_VERSION	MUSIKAPPNAME wxT(" ") MUSIK_TITLE_VERSION_STR
#define MUSIKSERV_VERSION		MUSIKAPPNAME wxT("Serv ") MUSIK_VERSION_STR
//----------------------------------//
//--- path stuff ~/.Musik/ vs ./ ---//
//----------------------------------//
#include <wx/filename.h>
#define MUSIK_HOME_DIR 			wxFileName::GetHomeDir() + wxFileName::GetPathSeparator() + wxT( ".Musik" ) + wxFileName::GetPathSeparator()
#define MUSIK_PLAYLIST_DIR		wxGetApp().Prefs.sDataPath + wxT( "playlists" ) + wxFileName::GetPathSeparator()
#define MUSIK_OLD_DB_FILENAME		wxGetApp().Prefs.sDataPath + wxT( "musiklib.db" )
#define MUSIK_DB_FILENAME		wxGetApp().Prefs.sDataPath + wxT( "wxmusiklib.db" )
#define MUSIK_SOURCES_FILENAME	wxGetApp().Prefs.sDataPath + wxT( "musiksources.dat" )
#define MUSIK_PATHS_FILENAME	wxGetApp().Prefs.sDataPath + wxT( "musikpaths.dat" )
#define MUSIK_VERSION_FILENAME	wxGetApp().Prefs.sDataPath + wxT( "musikversion.dat" )
#define MUSIK_PLAYERLIST_FILENAME	wxGetApp().Prefs.sDataPath + wxT( "musikplayerlist.dat" )

	
//-------------------------------------------------//
//--- for some reason some versions of wx do	---//
//--- not have this macro. pulled straight from	---//
//--- hashmap.h of the cvs head					---//
//-------------------------------------------------//
#include <wx/hashmap.h>
#ifndef WX_CLEAR_HASH_MAP
	#define WX_CLEAR_HASH_MAP(type, hashmap)                                     \
		{                                                                        \
			type::iterator it, en;                                               \
			for( it = (hashmap).begin(), en = (hashmap).end(); it != en; ++it )  \
				delete it->second;                                               \
			(hashmap).clear();                                                   \
		}
#endif 
//-----------------------------------------//
//--- needed for vc++/gcc compatibility	---//
//-----------------------------------------//

#ifdef _MSC_VER
#define strncasecmp strnicmp
#define strcasecmp	stricmp 
#endif

const int  MUSIK_MIN_RATING = -9;
const int  MUSIK_MAX_RATING = 9;

const size_t ActivityBoxesMaxCount = 4;

//-------------------------//
//--- enumerated values ---//
//-------------------------//
enum EPLAYLISTCOLUMNS
{
	PLAYLISTCOLUMN_RATING = 0,
	PLAYLISTCOLUMN_TRACK,
	PLAYLISTCOLUMN_TITLE,
	PLAYLISTCOLUMN_ARTIST,
	PLAYLISTCOLUMN_ALBUM,
	PLAYLISTCOLUMN_YEAR,
	PLAYLISTCOLUMN_GENRE,
	PLAYLISTCOLUMN_TIMES_PLAYED,
	PLAYLISTCOLUMN_LAST_PLAYED,
	PLAYLISTCOLUMN_TIME,
	PLAYLISTCOLUMN_BITRATE,
	PLAYLISTCOLUMN_FILENAME,
	PLAYLISTCOLUMN_NOTES,
	PLAYLISTCOLUMN_TIMEADDED,
	NPLAYLISTCOLUMNS			//--- this is ALWAYS the last entry ---//
};

enum EMUSIK_ACTIVITY_SELECTION_TYPE
{
	MUSIK_SELECTION_TYPE_STANDARD = 0,
	MUSIK_SELECTION_TYPE_SLOPPY,
	MUSIK_SELECTION_TYPE_HIGHLIGHT,
	NMUSIK_SELECTION_TYPE
};

enum EMUSIK_THREAD_EVENTS
{
	MUSIK_LIBRARY_THREAD_START = 1000,
	MUSIK_LIBRARY_THREAD_END,
	MUSIK_LIBRARY_THREAD_PROG,
	MUSIK_LIBRARY_THREAD_SCAN_PROG,

	MUSIK_TAG_THREAD_START,
	MUSIK_TAG_THREAD_END,
	MUSIK_TAG_THREAD_PROG,

	MUSIK_FRAME_THREAD_START,
	MUSIK_FRAME_THREAD_END,
	MUSIK_FRAME_THREAD_PROG,
	MUSIK_FRAME_EXIT_FADE_DONE,

	MUSIK_ACTIVITY_RENAME_THREAD_START,
	MUSIK_ACTIVITY_RENAME_THREAD_END,
	MUSIK_ACTIVITY_RENAME_THREAD_PROG,

	MUSIK_PLAYLIST_THREAD_START,
	MUSIK_PLAYLIST_THREAD_END,
	MUSIK_PLAYLIST_THREAD_PROG,

	MUSIK_PLAYER_NEXT_SONG,
	MUSIK_PLAYER_FADE_COMPLETE,
	MUSIK_PLAYER_NEW_METADATA,
	MUSIK_PLAYER_STOP,
	MUSIK_PLAYER_PLAY_RESTART,
	MUSIK_PLAYER_RESUME,
	MUSIK_PLAYER_EXIT,

	MUSIK_LIBRARY_FILE_SCANNED,
	MUSIK_LOGWINDOW_NEWMSG
};

enum EMUSIK_THREAD_TYPE
{
	MUSIK_ACTIVITY_RENAME_THREAD = 0,
	MUSIK_PLAYLIST_RENAME_THREAD,
	MUSIK_PLAYLIST_RETAG_THREAD,
	MUSIK_LIBRARY_SCANNEW_THREAD,
	MUSIK_LIBRARY_UPDATE_THREAD,
	MUSIK_LIBRARY_PURGE_THREAD,
	MUSIK_TAG_SINGLE_THREAD,
	MUSIK_TAG_BATCH_THREAD,
	MUSIK_WRITE_DIRTY_THREAD
};

enum EMUSIK_FRAME_WINDOWID
{
	MUSIK_FRAME_ID_FX = 20000
};
enum EMUSIK_MENU_ITEMS
{
	//--- main menu ---//
	MUSIK_MENU_PATHS = wxID_HIGHEST,
	MUSIK_MENU_WRITE,
	MUSIK_MENU_VIEW_DIRTY_TAGS,
	MUSIK_MENU_WRITE_TAGS,
	MUSIK_MENU_WRITE_CLEAR_DIRTY,
	MUSIK_MENU_FX,
	MUSIK_MENU_SOURCES_STATE,
	MUSIK_MENU_ACTIVITIES_STATE,
	MUSIK_MENU_PLAYLISTINFO_STATE,
	MUSIK_MENU_ALBUMART_STATE,
	MUSIK_MENU_NOWPLAYINGCONTROL_ONTOP_STATE,
	MUSIK_MENU_STAY_ON_TOP,
	MUSIK_MENU_CLEARDB,
	MUSIK_MENU_REBUILDDB,
	MUSIK_MENU_SIMPLEQUERY,
	MUSIK_MENU_CUSTOMQUERY,

	MUSIK_MENU_SELECT_SOURCES_LIBRARY,
	MUSIK_MENU_SELECT_SOURCES_NOWPLAYING,
	//--- playlist popup menu ---//
	MUSIK_PLAYLIST_CLEARPLAYERLIST,
	MUSIK_PLAYLIST_CONTEXT_PLAYNODE,
	MUSIK_PLAYLIST_CONTEXT_PLAY_INSTANTLY,
	MUSIK_PLAYLIST_CONTEXT_PLAY_ASNEXT,
	MUSIK_PLAYLIST_CONTEXT_PLAY_ENQUEUED,
	MUSIK_PLAYLIST_CONTEXT_PLAY_REPLACE_PLAYERLIST_WITH_SELECTION,
	MUSIK_PLAYLIST_CONTEXT_PLAY_REPLACE_PLAYERLIST,
	MUSIK_PLAYLIST_CONTEXT_OPEN_FOLDER_IN_FILEMANAGER,
	MUSIK_PLAYLIST_CONTEXT_SHOW_IN_LIBRARY_NODE,
	MUSIK_PLAYLIST_CONTEXT_SHOW_IN_LIBRARY_ARTIST,
	MUSIK_PLAYLIST_CONTEXT_SHOW_IN_LIBRARY_ALBUM,
	MUSIK_PLAYLIST_CONTEXT_SHOW_IN_LIBRARY_GENRE,
	MUSIK_PLAYLIST_CONTEXT_SHOW_IN_LIBRARY_YEAR,
	MUSIK_PLAYLIST_CONTEXT_RATENODE,
	MUSIK_PLAYLIST_CONTEXT_RATING,
	MUSIK_PLAYLIST_CONTEXT_TAGNODE = MUSIK_PLAYLIST_CONTEXT_RATING + 1 + (MUSIK_MAX_RATING - MUSIK_MIN_RATING) + 1,
	MUSIK_PLAYLIST_CONTEXT_TAG_TITLE,
	MUSIK_PLAYLIST_CONTEXT_TAG_TRACKNUM,
	MUSIK_PLAYLIST_CONTEXT_TAG_ARTIST,
	MUSIK_PLAYLIST_CONTEXT_TAG_ALBUM,
	MUSIK_PLAYLIST_CONTEXT_TAG_GENRE,
	MUSIK_PLAYLIST_CONTEXT_TAG_YEAR,
	MUSIK_PLAYLIST_CONTEXT_TAG_NOTES,
	MUSIK_PLAYLIST_CONTEXT_DELETENODE,
	MUSIK_PLAYLIST_DELETE_CONTEXT_DELETE_FROM_PLAYLIST,
	MUSIK_PLAYLIST_DELETE_CONTEXT_DELETE_FILES,
	MUSIK_PLAYLIST_DELETE_CONTEXT_DELETE_FROM_DB,
	MUSIK_PLAYLIST_CONTEXT_RENAME_FILES,
	MUSIK_PLAYLIST_CONTEXT_RETAG_FILES,
	MUSIK_PLAYLIST_CONTEXT_REBUILDTAG,
	MUSIK_PLAYLIST_CONTEXT_DISPLAYNODE,
	MUSIK_PLAYLIST_DISPLAY_FIRST,
	MUSIK_PLAYLIST_DISPLAY_LAST = MUSIK_PLAYLIST_DISPLAY_FIRST + NPLAYLISTCOLUMNS - 1,
	MUSIK_PLAYLIST_DISPLAY_SMART,
	MUSIK_PLAYLIST_DISPLAY_FIT,

	//--- sources popup menu ---//
	MUSIK_SOURCE_CONTEXT_CREATE,
	MUSIK_SOURCE_CONTEXT_CREATE_CURRENT_PLAYLIST,
	MUSIK_SOURCE_CONTEXT_STANDARD_PLAYLIST,
	MUSIK_SOURCE_CONTEXT_DYNAMIC_PLAYLIST,
	MUSIK_SOURCE_CONTEXT_CREATE_NETSTREAM,
	MUSIK_SOURCE_CONTEXT_EDIT_QUERY,
	MUSIK_SOURCE_CONTEXT_EDIT_URL,
	MUSIK_SOURCE_CONTEXT_RENAME,
	MUSIK_SOURCE_CONTEXT_DELETE,
	MUSIK_SOURCE_CONTEXT_SHOW_ICONS,
	MUSIK_SOURCE_CONTEXT_COPY_FILES,

	//--- library frame popup menu ---//
	MUSIK_PATHS_MENU_ADD,
	MUSIK_PATHS_MENU_REMOVESEL,
	MUSIK_PATHS_MENU_REMOVEALL,
	MUSIK_PATHS_CLEAR_LIBRARY,
	MUSIK_PATHS_UPDATE_LIBRARY,
	MUSIK_PATHS_REBUILD_LIBRARY,
	MUSIK_PATHS_PURGE_LIBRARY
    
};

enum EMUSIK_LIB_TYPE
{
	MUSIK_LIB_INVALID = -1,
	MUSIK_LIB_ARTIST = 0,
	MUSIK_LIB_ALBUM,
	MUSIK_LIB_GENRE,
	MUSIK_LIB_SONG,
	MUSIK_LIB_YEAR,
	MUSIK_LIB_DECADE
};

enum EMUSIK_FORMAT_TYPE
{
	MUSIK_FORMAT_INVALID = -1,
	MUSIK_FORMAT_MP3 = 0,
	MUSIK_FORMAT_OGG = 1,
	MUSIK_FORMAT_MOD = 2,
	MUSIK_FORMAT_WAV = 3,
	MUSIK_FORMAT_WMA = 4,
	MUSIK_FORMAT_AIFF = 5,
	MUSIK_FORMAT_NETSTREAM = 6,
	MUSIK_FORMAT_MP2 = 7,
	MUSIK_FORMAT_MPC = 8,
	MUSIK_FORMAT_APE = 9,
	MUSIK_FORMAT_FLAC = 10,
	// insert more formats here
	COUNT_MUSIK_FORMAT
};

enum EMUSIK_ACTIVITY_TYPE
{
	MUSIK_LBTYPE_NULL = 0,
	MUSIK_LBTYPE_ARTISTS,
	MUSIK_LBTYPE_ALBUMS,
	MUSIK_LBTYPE_GENRES,
	MUSIK_LBTYPE_YEARS
};

enum EMUSIK_PLAYMODE
{
	MUSIK_PLAYMODE_NORMAL = 0,
	MUSIK_PLAYMODE_REPEATSONG,
	MUSIK_PLAYMODE_REPEATLIST,
	MUSIK_PLAYMODE_SHUFFLE,
	MUSIK_PLAYMODE_AUTO_DJ,
	MUSIK_PLAYMODE_AUTO_DJ_ALBUM

};
enum EMUSIK_SEARCHMODE
{
	MUSIK_SEARCHMODE_ALLWORDS = 0,
	MUSIK_SEARCHMODE_EXACTPHRASE = 1,
	MUSIK_SEARCHMODE_ANYWORDS = 2
};
enum EMUSIK_FUZZYSEARCHMODE
{
	MUSIK_FUZZYSEARCHMODE_NONE = 0,
	MUSIK_FUZZYSEARCHMODE_LOW = 1,
	MUSIK_FUZZYSEARCHMODE_MIDDLE = 2,
	MUSIK_FUZZYSEARCHMODE_HIGH = 3

};
struct MUSIK_UpdateFlags
{
	enum eUpdateFlags
	{
		InsertFilesIntoPlayer = 1,
		EnquequeFilesIntoPlayer	= 2,
		PlayFiles				=4,
		RebuildTags             =8,
		WaitUntilDone			=16
	};
};

#ifdef wxUSE_HOTKEY
enum MUSIKHOTKEYIDS
{
	MUSIK_HOTKEYID_FIRST = 1,
	MUSIK_HOTKEYID_STOP = 1,
	MUSIK_HOTKEYID_PLAYPAUSE = 2,
	MUSIK_HOTKEYID_PREV = 3,
	MUSIK_HOTKEYID_NEXT = 4,
	MUSIK_HOTKEYID_LAST = MUSIK_HOTKEYID_NEXT
};
#endif
#endif//MUSIKDEFINES_H
