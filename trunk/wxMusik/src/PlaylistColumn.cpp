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


const PlaylistColumn g_PlaylistColumn[PlaylistColumn::NCOLUMNS] =
{
    {PlaylistColumn::RATING,		wxT( "rating"		),  wxTRANSLATE( "Rating"		),	wxLIST_FORMAT_CENTER,PlaylistColumn::Numeric ,PlaylistColumn::SortCase     },
    {PlaylistColumn::TRACK,			wxT( "tracknum"	    ),  wxTRANSLATE( "Track"		),	wxLIST_FORMAT_LEFT  ,PlaylistColumn::Numeric ,PlaylistColumn::SortCase     },
    {PlaylistColumn::TITLE,			wxT( "title"		),  wxTRANSLATE( "Title"		),	wxLIST_FORMAT_LEFT  ,PlaylistColumn::Textual ,PlaylistColumn::SortNoCase   },
    {PlaylistColumn::ARTIST,		wxT( "artist"		),  wxTRANSLATE( "Artist"		),	wxLIST_FORMAT_LEFT  ,PlaylistColumn::Textual ,PlaylistColumn::SortNoCaseNoPrefix},
    {PlaylistColumn::ALBUM,			wxT( "album"		),  wxTRANSLATE( "Album"		),	wxLIST_FORMAT_LEFT  ,PlaylistColumn::Textual ,PlaylistColumn::SortNoCase   },
    {PlaylistColumn::YEAR,			wxT( "year"		    ),  wxTRANSLATE( "Year"		    ),	wxLIST_FORMAT_CENTER,PlaylistColumn::Textual ,PlaylistColumn::SortCase     },
    {PlaylistColumn::GENRE,			wxT( "genre"		),  wxTRANSLATE( "Genre"		),	wxLIST_FORMAT_LEFT  ,PlaylistColumn::Textual ,PlaylistColumn::SortCase     },
    {PlaylistColumn::TIMES_PLAYED,	wxT( "timesplayed"  ),  wxTRANSLATE( "Times Played"),	wxLIST_FORMAT_RIGHT ,PlaylistColumn::Numeric ,PlaylistColumn::SortCase     },
    {PlaylistColumn::LAST_PLAYED,	wxT( "lastplayed"	),  wxTRANSLATE( "Last Played"	),	wxLIST_FORMAT_RIGHT ,PlaylistColumn::Date    ,PlaylistColumn::SortCase     },
    {PlaylistColumn::TIME,			wxT( "duration"	    ),  wxTRANSLATE( "Time"		    ),	wxLIST_FORMAT_RIGHT ,PlaylistColumn::Duration,PlaylistColumn::SortCase     },
    {PlaylistColumn::BITRATE,		wxT( "bitrate"		),  wxTRANSLATE( "Bitrate"		),	wxLIST_FORMAT_RIGHT ,PlaylistColumn::Numeric ,PlaylistColumn::SortCase     },
    {PlaylistColumn::FILENAME,		wxT( "filename"	    ),  wxTRANSLATE( "Filename"	    ),	wxLIST_FORMAT_LEFT  ,PlaylistColumn::Textual ,PlaylistColumn::SortCase     },
    {PlaylistColumn::NOTES,			wxT( "notes"		),  wxTRANSLATE( "Notes"		),	wxLIST_FORMAT_LEFT	,PlaylistColumn::Textual ,PlaylistColumn::SortCase     },
    {PlaylistColumn::TIMEADDED,		wxT( "timeadded"	),  wxTRANSLATE( "Added on"	    ),  wxLIST_FORMAT_RIGHT ,PlaylistColumn::Date    ,PlaylistColumn::SortCase     }
};	 			
