#ifndef PLAYLISTCOLUMN_H
#define PLAYLISTCOLUMN_H

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
    wxString    Label;
    int         Aligned;
    eType       Type;
    eSortOrder  SortOrder;
};

extern const PlaylistColumn g_PlaylistColumn[PlaylistColumn::NCOLUMNS];

#endif
