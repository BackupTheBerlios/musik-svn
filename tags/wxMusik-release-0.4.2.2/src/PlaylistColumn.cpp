/*
*  PlaylistColumn.cpp
*  
*  Copyright (c) 2005 Gunnar Roth (gunnar.roth@gmx.de)
*	
*
*  See the file "license.txt" for information on usage and redistribution
*  of this file, and for a DISCLAIMER OF ALL WARRANTIES.
*/
// For compilers that support precompilation, includes "wx/wx.h".
#include "myprec.h"

#include "PlaylistColumn.h"
#include <wx/listbase.h>
#include <wx/intl.h>

const PlaylistColumn g_PlaylistColumn[PlaylistColumn::NCOLUMNS] =
{                                                                                                                                                                                                             //bEnableActivityBox
                                                                                                                                                                                                                       //bEditableInActivityBox                             
    {PlaylistColumn::RATING,        wxT( "rating"	   ), wxT("%s")                          ,wxTRANSLATE( "Rating"	)       ,wxLIST_FORMAT_CENTER   ,PlaylistColumn::Numeric ,PlaylistColumn::SortCase          ,true  ,true   },   
    {PlaylistColumn::TRACK,         wxT( "tracknum"	   ), wxT("%s")                          ,wxTRANSLATE( "Track"		)   ,wxLIST_FORMAT_LEFT     ,PlaylistColumn::Numeric ,PlaylistColumn::SortCase          ,true  ,true   },
    {PlaylistColumn::TITLE,         wxT( "title"       ), wxT("%s")                          ,wxTRANSLATE( "Title"		)   ,wxLIST_FORMAT_LEFT     ,PlaylistColumn::Textual ,PlaylistColumn::SortNoCase        ,true  ,true   },
    {PlaylistColumn::ARTIST,        wxT( "artist"      ), wxT("%s")                          ,wxTRANSLATE( "Artist"		)   ,wxLIST_FORMAT_LEFT     ,PlaylistColumn::Textual ,PlaylistColumn::SortNoCaseNoPrefix,true  ,true   },
    {PlaylistColumn::ALBUM,	        wxT( "album"       ), wxT("%s")                          ,wxTRANSLATE( "Album"		)   ,wxLIST_FORMAT_LEFT     ,PlaylistColumn::Textual ,PlaylistColumn::SortNoCase        ,true  ,true   },
    {PlaylistColumn::YEAR,          wxT( "year"        ), wxT("%s")                          ,wxTRANSLATE( "Year"		)   ,wxLIST_FORMAT_CENTER   ,PlaylistColumn::Textual ,PlaylistColumn::SortCase          ,true  ,true   },
    {PlaylistColumn::GENRE,         wxT( "genre"       ), wxT("%s")                          ,wxTRANSLATE( "Genre"		)   ,wxLIST_FORMAT_LEFT     ,PlaylistColumn::Textual ,PlaylistColumn::SortCase          ,true  ,true   },
    {PlaylistColumn::TIMES_PLAYED,  wxT( "timesplayed" ), wxT("%s")                          ,wxTRANSLATE( "Times Played")  ,wxLIST_FORMAT_RIGHT    ,PlaylistColumn::Numeric ,PlaylistColumn::SortCase          ,true  ,false  },
    {PlaylistColumn::LAST_PLAYED,   wxT( "lastplayed"  ), wxT("julianday(%s,'start of day')"),wxTRANSLATE( "Last Played")   ,wxLIST_FORMAT_RIGHT    ,PlaylistColumn::Date    ,PlaylistColumn::SortCase          ,true  ,false  },
    {PlaylistColumn::TIME,          wxT( "duration"    ), wxT("round(%s/60000)")             ,wxTRANSLATE( "Time"		)   ,wxLIST_FORMAT_RIGHT    ,PlaylistColumn::Duration,PlaylistColumn::SortCase          ,true  ,false  },
    {PlaylistColumn::BITRATE,       wxT( "bitrate"     ), wxT("%s")                          ,wxTRANSLATE( "Bitrate"    )   ,wxLIST_FORMAT_RIGHT    ,PlaylistColumn::Numeric ,PlaylistColumn::SortCase          ,true  ,false  },
    {PlaylistColumn::FILENAME,      wxT( "filename"    ), wxT("%s")                          ,wxTRANSLATE( "Filename"	)   ,wxLIST_FORMAT_LEFT     ,PlaylistColumn::Textual ,PlaylistColumn::SortCase          ,false ,false  },
    {PlaylistColumn::NOTES,         wxT( "notes"       ), wxT("%s")                          ,wxTRANSLATE( "Notes"		)   ,wxLIST_FORMAT_LEFT     ,PlaylistColumn::Textual ,PlaylistColumn::SortCase          ,false ,false  },
    {PlaylistColumn::TIMEADDED,	    wxT( "timeadded"   ), wxT("julianday(%s,'start of day')"),wxTRANSLATE( "Added on"	)   ,wxLIST_FORMAT_RIGHT    ,PlaylistColumn::Date    ,PlaylistColumn::SortCase          ,true  ,false  }
};	 			
