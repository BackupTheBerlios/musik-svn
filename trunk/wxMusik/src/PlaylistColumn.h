#ifndef PLAYLISTCOLUMN_H
#define PLAYLISTCOLUMN_H

#include <wx/string.h>
struct PlaylistColumn
{
    enum eType {Textual=1,Numeric,Date,Duration};
    enum eId
    {
        INVALID = -1,
        RATING = 0,
        TRACK,
        TITLE,
        ARTIST,
        ALBUM,
        YEAR,
        GENRE,
        TIMES_PLAYED,
        LAST_PLAYED,
        TIME,
        BITRATE,
        FILENAME,
        NOTES,
        TIMEADDED,
        MODIFIED,
        NCOLUMNS			//--- this is ALWAYS the last entry ---//
    };
    enum eSortOrder
    { 
        SortNone,  
        SortNoCase,
        SortNoCaseNoPrefix,
        SortCase
    };

    eId         Id;
    wxString    DBName;
    wxString    ColQueryMask;
    wxString    Label;
    int         Aligned;
    eType       Type;
    eSortOrder  SortOrder;
    // flags
    long        bEnableActivityBox:1;
    long        bEditableInActivityBox:1;
};

extern const PlaylistColumn g_PlaylistColumn[PlaylistColumn::NCOLUMNS];

#endif
