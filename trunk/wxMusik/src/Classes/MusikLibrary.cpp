/*
 *  MusikLibrary.cpp
 *
 *  Object which controls database manipulation.
 *
 *  Uses: SQLite, id3lib, and ogg/vorbis
 *  Information about SQLite - An Embeddable SQL Database Engine is available at http://www.hwaci.com/sw/sqlite/
 *  Information about id3lib is available at http://www.id3lib.org
 *  Information about ogg/vorbis is available at http://www.vorbis.com/
 *  
 *  Copyright (c) 2003 Casey Langen (casey@bak.rr.com)
 *	Contributors: Simon Windmill, Dustin Carter, Gunnar Roth, Wade Brainerd
 *
 *  See the file "license.txt" for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL WARRANTIES.
*/
// For compilers that support precompilation, includes "wx/wx.h".
#include "myprec.h"
#include "MusikLibrary.h"

//--- globals ---//
#include "../MusikGlobals.h"
#include "../MusikUtils.h"

//--- wx ---//
#include <wx/file.h>
#include <wx/filename.h>
#include "Library/MetaDataHandler.h"
#include "MusikDb_Sqlite3.h"


WX_DECLARE_STRING_HASH_MAP( MusikSongId *, myStringToMusikSongIdPtrMap );

//--- frames ---//
//#include "../Frames/MusikFrame.h"

CMusikSong::CMusikSong()
{
	Rating 		= 0;
	TimesPlayed = 0;
	songid		= -1;
	LastPlayed = TimeAdded	= 0.0;

}

class CMusikLibrary::BusyHandler : public MusikDb::IBusyCallback
{
public:
    BusyHandler()
        :m_CondVar(m_mtx)
    {
        m_bWaiting = false;
    }
    virtual ~BusyHandler()
    {}
    void Signal()
    {
        m_mtx.Lock();
        bool bWaiting = m_bWaiting;
        m_CondVar.Signal();
        m_mtx.Unlock();
        if(bWaiting)
            wxMilliSleep(10);
    }
protected:
    virtual bool OnBusy()
    {
        m_mtx.Lock();
        m_bWaiting = true;
        m_CondVar.Wait();
        m_bWaiting = false;
        m_mtx.Unlock();
        return 1;
    }
    bool m_bWaiting;
    wxMutex m_mtx;
    wxCondition m_CondVar;
};


CMusikLibrary::CMusikLibrary()
	: wxEvtHandler()
    ,m_pBusyHandler(new BusyHandler)
    ,m_pMasterLibrary(NULL)
    ,m_pDB(NULL)
    ,m_bTansactionInProgress(false)
{
	m_nCachedSongCount = -1;
    SetSortOrderColumn( g_PlaylistColumn[PlaylistColumn::ARTIST] );
}

CMusikLibrary::~CMusikLibrary()
{
    if(m_bTansactionInProgress)
        InternalEndTransaction();
	Shutdown();
    if(m_pMasterLibrary)
        m_pMasterLibrary->SignalSlaveTransactionEnd();// if the master ist waiting for some reason, he gets the signal to stop waiting.
}


CMusikLibrary * CMusikLibrary::CreateSlave()
{
    CMusikLibrary * p = new CMusikLibrary();
    p->m_pDB.reset(m_pDB->Clone()); 
    p->m_pMasterLibrary = this;
    return p;
}
void CMusikLibrary::OnSongDataChange(int songid)
{
    wxCriticalSectionLocker lock( m_csCacheAccess );
    OnSongDataChange_NOLOCK(songid);
}

void CMusikLibrary::OnSongDataChange_NOLOCK(int songid)
{
   if(!m_bTansactionInProgress)
   {
       if(m_pMasterLibrary)
           m_pMasterLibrary->OnSongDataChange(songid);
        if(songid == -1)
            m_mapSongCache.clear();
        else
            m_mapSongCache.erase(songid);
   }
   else
   {
       if(m_arrTransactionIdsChanged.size() > 0 && m_arrTransactionIdsChanged[0] == -1)
           return;
       if(songid == -1)
           m_arrTransactionIdsChanged.clear();
       if(m_arrTransactionIdsChanged.size() > 100)
       {
           m_arrTransactionIdsChanged.clear();
           m_arrTransactionIdsChanged.push_back(-1);
           return;
       }
       m_arrTransactionIdsChanged.push_back(songid);
   }
}

void CMusikLibrary::BeginTransaction()
{ 	
    {
        wxCriticalSectionLocker lock( m_csCacheAccess );
        m_arrTransactionIdsChanged.clear();
        m_bTansactionInProgress=true;
    }
    m_pDB->BeginTransaction();
}
void CMusikLibrary::EndTransaction()
{
    InternalEndTransaction();
}
void CMusikLibrary::InternalEndTransaction()
{
    m_pDB->EndTransaction();
    if(m_pMasterLibrary)
        m_pMasterLibrary->SignalSlaveTransactionEnd();
    wxCriticalSectionLocker lock( m_csCacheAccess );
    m_bTansactionInProgress = false;
    for (std::vector<int>::iterator i=m_arrTransactionIdsChanged.begin();i != m_arrTransactionIdsChanged.end(); ++i )
    {
        OnSongDataChange_NOLOCK(*i);
    }
}

void CMusikLibrary::SignalSlaveTransactionEnd()
{
    m_pBusyHandler->Signal();    
}
void CMusikLibrary::Shutdown()
{
    m_pDB.reset();
}

bool CMusikLibrary::Load()
{
	wxString sFilename = MUSIK_DB_FILENAME;
		
	//--- look for database.. if need be, create it and create tables ---//
	static	const char *szCreateVersionQuery =
		"CREATE TABLE IF NOT EXISTS version ( " 	
		"name, major, majorsub ,minor, minorsub "
		" );";
	
	//--- create the tables ---//
	static const char *szCreateSongTableQuery  = 
			"CREATE TABLE IF NOT EXISTS songs ( "	
			"songid INTEGER PRIMARY KEY, "
			"format INTEGER, "		
			"vbr INTEGER, "			
			"filename TEXT NOT NULL UNIQUE, "	
			"artist TEXT, "	
			"title TEXT, "	
			"album TEXT, "	
			"tracknum INTEGER, "	
			"year INTEGER, "		
			"genre TEXT, "	
			"rating INTEGER, "		
			"bitrate INTEGER, "	
			"lastplayed REAL, "	
			"notes TEXT, "	
			"timesplayed INTEGER , "	
			"duration INTEGER, "	
			"timeadded REAL, "	
			"filesize INTEGER, "	
			"dirty INTEGER, "
            "modified REAL"
			" );";
	const char* szCreateSongTableIdxQuery =
			"CREATE INDEX IF NOT EXISTS songs_title_idx on songs (title);"
			"CREATE UNIQUE INDEX IF NOT EXISTS songs_filename_idx on songs (filename);"
			"CREATE INDEX IF NOT EXISTS songs_artist_idx on songs (artist);"
			"CREATE INDEX IF NOT EXISTS songs_album_idx on songs (album);"
			"CREATE INDEX IF NOT EXISTS songs_genre_idx on songs (genre);"
            "CREATE INDEX IF NOT EXISTS songs_year_idx on songs (year);"
//			"CREATE INDEX IF NOT EXISTS songs_tracknum_idx on songs (tracknum);"
			"CREATE INDEX IF NOT EXISTS songs_artist_album_tracknum_idx on songs (artist collate nocase,album collate nocase,tracknum);"
			"CREATE INDEX IF NOT EXISTS songs_timeadded_idx on songs (timeadded);"
			"CREATE INDEX IF NOT EXISTS songs_lastplayed_idx on songs (lastplayed);"
			;
	static	const char *szCreateSongHistoryQuery =
			"CREATE TABLE IF NOT EXISTS songhistory ( "	
				"songid number(10), "
				"date_played timestamp , "
				"percent_played number(10),"
				"selected_by_user number(1) "
				" );";

	static const char* szCreateSongHistoryTableIdxQuery =
		"CREATE INDEX IF NOT EXISTS songhistory_songid_idx on songhistory (songid);"
		"CREATE INDEX IF NOT EXISTS songhistory_date_played_idx on songhistory (date_played);"
		"CREATE INDEX IF NOT EXISTS songhistory_percent_played_idx on songhistory (percent_played);"
		;
	wxLogNull lognull;
    Shutdown();
    bool bConvertFromDB2 = !wxFileExists(sFilename) && wxFileExists(MUSIK_DB_FILENAME2);
    m_pDB.reset(new MusikDb_Sqlite3());
	if(m_pDB.get() && m_pDB->Open(sFilename))
	{
#ifdef __WXMSW__
        m_pDB->Exec("PRAGMA page_size=4096;");
#endif
		// always create table, if it exists an error will be returned by Exec(), but we dont care.
		m_pDB->Exec( szCreateVersionQuery);
		m_pDB->Exec( szCreateSongTableQuery);
		m_pDB->Exec( szCreateSongHistoryQuery);
        if(bConvertFromDB2)
            ConvertFromDB2();
        m_pDB->Exec( szCreateSongTableIdxQuery);
		m_pDB->Exec( szCreateSongHistoryTableIdxQuery);
		m_pDB->Exec( "CREATE TRIGGER song_deleted_trigger DELETE ON songs " 
							"BEGIN "
							"DELETE FROM songhistory WHERE songid = old.songid;"
							"END;");


		//m_pDB->Exec( "PRAGMA synchronous = OFF;");
		SetCacheSize(wxGetApp().Prefs.nDBCacheSize);
		CheckVersion();
		SetAutoDjFilter(wxGetApp().Prefs.sAutoDjFilter );
		m_pDB->Exec( "CREATE VIEW valid_albums as select album,artist,most_lastplayed from ("
			"select album,artist,sum(duration) as sum_duration,max(lastplayed+0) as most_lastplayed "  
			"from songs where album != '' group by album) where sum_duration > 1500000;"
			);	
		m_pDB->Exec("CREATE VIEW autodj_albums as select album,artist,most_lastplayed from ("
			"select album,artist,sum(duration) as sum_duration,max(lastplayed+0) as most_lastplayed "  
			"from autodj_songs where album != '' group by album) where sum_duration > 1500000;"
			);	
        if(m_pMasterLibrary == NULL)
            m_pDB->SetBusyHandler(m_pBusyHandler.get());
	    return true;
    }
	return false;
}
#include "MusikDb_Sqlite.h"

static int db_callbackInsertIntoDb(void *args, int /*numCols*/, char **results, char ** /*columnNames*/)
{
    MusikDb * pDb = (MusikDb*)args;
    pDb->Exec(results[0]);
    return 0;
}

void CMusikLibrary::ConvertFromDB2()
{
    std::auto_ptr<MusikDb> pDb2(new MusikDb_Sqlite);
    if(!m_pDB.get() || !pDb2->Open(MUSIK_DB_FILENAME2))
        return;
    MusikDb::ResultCB cb(m_pDB.get(),db_callbackInsertIntoDb);
    m_pDB->BeginTransaction();
    pDb2->Exec("SELECT 'INSERT INTO ' || '\"songs\"' || ' "
        "VALUES(' || quote(\"songid\") || ', ' || quote(\"format\") || ', ' || "
        "quote(\"vbr\") || ', ' || quote(\"filename\") || ', ' || quote(\"artist\") || ', ' || "
        "quote(\"title\") || ', ' || quote(\"album\") || ', ' || quote(\"tracknum\") || ', ' || "
        "quote(\"year\") || ', ' || quote(\"genre\") || ', ' || quote(\"rating\") || ', ' || "
        "quote(\"bitrate\") || ', ' || quote(\"lastplayed\") || ', ' || quote(\"notes\") || ', ' || "
        "quote(\"timesplayed\") || ', ' || quote(\"duration\") || ', ' || quote(\"timeadded\") || ', ' || "
        "quote(\"filesize\") || ', ' || quote(\"dirty\") || ', ' || quote(\"timeadded\") ||')' "
        "FROM  \"songs\";",cb);
    
    pDb2->Exec("SELECT 'INSERT INTO ' || '\"songhistory\"' || ' "
        "VALUES(' || quote(\"songid\") || ', ' || quote(\"date_played\") || ', ' || "
        "quote(\"percent_played\") || ', ' || quote(\"selected_by_user\") || ')' "
        "FROM  \"songhistory\";",cb);
    m_pDB->EndTransaction();
}
void CMusikLibrary::CheckVersion()
{
    unsigned long oldversion = 0;
    int ver_major = -1;
    int ver_majorsub = -1;
    int ver_minor = -1;
    int ver_minorsub = -1;

    MusikDb::ResultSet rset;

    if(!m_pDB->Exec( "select name,major,majorsub,minor,minorsub from version where name='wxMusik';",&rset))
    {
        wxLogError(wxT("%s"),ConvA2W(rset.Error().Text()).c_str());
        return;
    }
    //--- look and see if there's one row ---//
    if ( rset.Rows() == 1)
    {
        ver_major = atoi(rset.Value(0,1));
        ver_majorsub = atoi(rset.Value(0,2));
        ver_minor = atoi(rset.Value(0,3));
        ver_minorsub = atoi(rset.Value(0,4));
        oldversion = MUSIK_VERSION(ver_major,ver_majorsub,ver_minor,ver_minorsub);
    }

    if(oldversion > 0)
    {

        if(MUSIK_VERSION_CURRENT == oldversion)
        {
            // nothing to do
            return;
        }
    }
    // now lets do conversion depending on the version number

    // nothing to do for now
    
    // update version info
    MusikDb::QueryString qsUpdateVersion("update version set major=%d,majorsub=%d,minor=%d,minorsub=%d where name = 'wxMusik';",
        MUSIK_VERSION_MAJOR,MUSIK_VERSION_MAJORSUB,MUSIK_VERSION_MINOR,MUSIK_VERSION_MINORSUB);
    m_pDB->Exec(qsUpdateVersion);
}

void CMusikLibrary::CheckVersion2()
{
	unsigned long oldversion = 0;
	int ver_major = -1;
	int ver_majorsub = -1;
	int ver_minor = -1;
	int ver_minorsub = -1;

    MusikDb::ResultSet rset;

    if(!m_pDB->Exec( "select name,major,majorsub,minor,minorsub from version where name='wxMusik';",&rset))
    {
        wxLogError(wxT("%s"),ConvA2W(rset.Error().Text()).c_str());
        return;
    }
    //--- look and see if there's one row ---//
    if ( rset.Rows() == 1)
    {
        ver_major = atoi(rset.Value(0,1));
        ver_majorsub = atoi(rset.Value(0,2));
        ver_minor = atoi(rset.Value(0,3));
        ver_minorsub = atoi(rset.Value(0,4));
        oldversion = MUSIK_VERSION(ver_major,ver_majorsub,ver_minor,ver_minorsub);
    }

	if(oldversion > 0)
	{
		
		if(MUSIK_VERSION_CURRENT == oldversion)
		{
			// nothing to do
			return;
		}
        MusikDb::QueryString qsUpdateVersion("update version set major=%d,majorsub=%d,minor=%d,minorsub=%d where name = 'wxMusik';",
            MUSIK_VERSION_MAJOR,MUSIK_VERSION_MAJORSUB,MUSIK_VERSION_MINOR,MUSIK_VERSION_MINORSUB);
		// update version info
		m_pDB->Exec(qsUpdateVersion);
	}
	else
	{
	   // version must be before 0.3.1.0
	   // bring it to version 0.3.1.0
		// check for old datetime format 
		wxArrayString aOldDateStrings;
		Query(wxT(" select timeadded  from songs where timeadded like '%:%' limit 1;"),aOldDateStrings);// does an entry contains a colon?
		if(aOldDateStrings.GetCount() == 1)
		{
			// db contains old string format. m/d/y h:m:s
			//convert it to julianday
			m_pDB->Exec("update songs set lastplayed = cnvMusikOldDTFormatToJulianday(lastplayed), timeadded = cnvMusikOldDTFormatToJulianday(timeadded) where 1;");
		}

		// convert <unknown> and 0 entrys to ''
		m_pDB->Exec( "update songs set album = '' where album = '<unknown>';"
							"update songs set artist = '' where artist = '<unknown>';"		
							"update songs set genre = '' where genre = '<unknown>';"		
							"update songs set year = '' where year = '<unknown>' or year=0;"		
							"update songs set notes = '' where  notes = '<unknown>' ;"		
							"update songs set lastplayed = '' where  lastplayed isnull;"		
							"update songs set tracknum = '' where tracknum = '<unknown>' OR tracknum = 0 OR tracknum isnull;"		
							);


		// convert iso8859-1 encoding to utf-8
		m_pDB->Exec("update songs set artist = cnvISO8859_1ToUTF8(artist),"
							" album = cnvISO8859_1ToUTF8(album),"		
							" title = cnvISO8859_1ToUTF8(title),"		
							" genre = cnvISO8859_1ToUTF8(genre),"		
							" filename = cnvISO8859_1ToUTF8(filename) where 1; "
							);

		// insert version info
        MusikDb::QueryString qsInsertVersion("INSERT INTO version values('wxMusik',%d,%d,%d,%d);",
            MUSIK_VERSION_MAJOR,MUSIK_VERSION_MAJORSUB,MUSIK_VERSION_MINOR,MUSIK_VERSION_MINORSUB);
        m_pDB->Exec(qsInsertVersion);
	}
	// now lets do conversion depending on the version number

	if(oldversion < MUSIK_VERSION(0,3,1,1))
	{
	   // rating is from -9 to 9 now instead of 0 - 5
		m_pDB->Exec("UPDATE songs set rating = ((rating * 9)/5 )% 10;");
	}

}

//---  if true, compares the full path, if false, just looks for the filename itself   ---//
//--- obviously the filename you pass will either be full or just filename accordingly ---//
bool CMusikLibrary::FileInLibrary( const wxString & filename, bool fullpath )
{
    bool result = false;

    if ( fullpath )
    {
		MusikDb::QueryString query( "select distinct songs.songid from songs where filename = %Q;", ( const char* )ConvToUTF8(filename) );
        result = m_pDB->Exec(query,(wxString*)NULL,NULL);
    }
    else
    {
        MusikDb::QueryString query( "select distinct songs.songid from songs where filename like '%%%q%%';", ( const char* )ConvToUTF8(filename) );
        result = m_pDB->Exec(query,(wxString*)NULL,NULL);

    }
	return result;
}

bool CMusikLibrary::AddSongDataFromFile( const wxString & filename )
{
	//-----------------------------------------------------//
	//--- we don't need to worry about duplicate files	---//
	//--- because the MusikLibraryFrame will take care	---//
	//--- of any conflicts.								---//
	//-----------------------------------------------------//

	if ( filename.IsEmpty() )
		return true;

	
	CSongMetaData MetaData;
	MetaData.Filename = filename;
	CMetaDataHandler::RetCode rc  = CMetaDataHandler::GetMetaData( MetaData );
	if(rc == CMetaDataHandler::notsupported)
	{
		::wxLogInfo(_("Parsing of file %s not supported. Setting title to filename."),(const wxChar *)MetaData.Filename.GetFullPath());
		rc = CMetaDataHandler::success; // continue as if success
	}
	if(rc == CMetaDataHandler::success )
	{

		//--- run the query ---//
		m_nCachedSongCount = -1;
        MusikDb::QueryString qInsert("insert into songs values (%Q ,%d, %d, %Q, %Q, %Q, %Q, %d, %Q, %Q, %d, %d, %Q, %Q, %d, %d, julianday('now'), %d, %d ,julianday('now'));",
            NULL,	
            (int)MetaData.eFormat,	
            MetaData.bVBR, 
            ( const char* )ConvToUTF8(MetaData.Filename.GetFullPath()) ,
            ( const char* )MetaData.Artist, 
            ( const char* )MetaData.Title, 
            ( const char* )MetaData.Album, 
            MetaData.nTracknum, 
            ( const char* )MetaData.Year, 
            ( const char* )MetaData.Genre, 
            0,//rating 
            MetaData.nBitrate, 
            "",//lastplayed 
            ( const char* )MetaData.Notes,//notes 
            0,//timesplayed 
            MetaData.nDuration_ms, 
            MetaData.nFilesize,
            0); //dirty
        m_pDB->Exec(qInsert);
	}
	else if(rc == CMetaDataHandler::fail)
	{
		::wxLogWarning(_("Parsing of file %s failed."),(const wxChar *)MetaData.Filename.GetFullPath());
	}
	else
	{
		wxASSERT(false);
	}

	return rc != CMetaDataHandler::fail;
}

bool CMusikLibrary::UpdateSongDataFromFile( const wxString & filename , bool bForce)
{
	if ( filename.IsEmpty() )
		return false;
    if(!bForce)
    {
        double jdnFileInDbMTIME;

        if(m_pDB->Exec(MusikDb::QueryString( "select songs.modified from songs where filename = %Q;",
            ( const char* )ConvToUTF8(filename) ),&jdnFileInDbMTIME))
        {
            wxStructStat st;
            wxStat(filename,&st);
            wxDateTime dt(st.st_mtime);
            if(dt.GetJDN() <= jdnFileInDbMTIME)
            {// file mtime is less than the modified time in the db entry
                return false;
            }
        }
    }
	CSongMetaData MetaData;
	MetaData.Filename = filename;
	CMetaDataHandler::RetCode rc  = CMetaDataHandler::GetMetaData( MetaData );
	if(rc == CMetaDataHandler::notsupported)
	{
		::wxLogInfo(_("Parsing of file %s not supported. Song data is not updated."),(const wxChar *)MetaData.Filename.GetFullPath());
		return true; // we are not able to parse this file, so we return here, to not overwrite the possible user edited data in the db with empty data.
	}
	if(rc == CMetaDataHandler::success )
	{
		int songid = QueryCount(MusikDb::QueryString( "select distinct songs.songid from songs where filename = %Q;", ( const char* )ConvToUTF8(MetaData.Filename.GetFullPath()) ));
		wxASSERT(songid >= 0);
		UpdateFullSongData(songid,MetaData);
	}
	else if(rc == CMetaDataHandler::fail)
	{
		::wxLogWarning(_("Parsing of file %s failed."),(const wxChar *)MetaData.Filename.GetFullPath());
	}
	else
	{
		wxASSERT(false);
	}
    
	return rc != CMetaDataHandler::fail;
}

bool CMusikLibrary::UpdateFullSongData(int songid, const CSongMetaData &MetaData )
{
	//--- run the query ---//
	if(m_pDB->Exec(MusikDb::QueryString("update songs set format=%d, vbr=%d, artist=%Q, title=%Q, album=%Q, tracknum=%d, year=%Q, genre=%Q, notes=%Q, bitrate=%d, duration=%d, filesize=%d, dirty=0,modified=julianday('now') where songid = %d;", 
		(int)MetaData.eFormat,	
		MetaData.bVBR, 
		( const char* )MetaData.Artist, 
		( const char* )MetaData.Title, 
		( const char* )MetaData.Album, 
		MetaData.nTracknum, 
		( const char* )MetaData.Year, 
		( const char* )MetaData.Genre, 
		( const char* )MetaData.Notes, 
		MetaData.nBitrate, 
		MetaData.nDuration_ms, 
		MetaData.nFilesize,
		songid
		)))
	{
		OnSongDataChange(songid);
		return true;
	}
	return false;
}

bool CMusikLibrary::WriteTag(  MusikSongId & songid, bool ClearAll , bool bUpdateDB )
{

	CMetaDataHandler::RetCode rc  = CMetaDataHandler::success;
    CMusikSong & song = songid.SongCopy();
	if(false == wxFileExists(song.MetaData.Filename.GetFullPath()))
	{
		::wxLogWarning(_("Writing tags to file %s failed,because the file does not exist.\nPlease purge the database."),(const wxChar *)song.MetaData.Filename.GetFullPath());
		return false;
	}
	else
	{
		rc = CMetaDataHandler::WriteMetaData(song.MetaData,ClearAll);
	}
	if(rc == CMetaDataHandler::notsupported)
	  ::wxLogInfo(_("Writing tags to file %s is not supported. File is set as clean in database."),(const wxChar *)song.MetaData.Filename.GetFullPath());
	if(( rc != CMetaDataHandler::fail) && bUpdateDB )
	{
		//-----------------------------//
		//--- tag writing succeeded
        //--- so update the db and flag as  clean	
        //-----------------------------//
		UpdateItem( songid , false );
	}
	else if(rc == CMetaDataHandler::fail)
		::wxLogWarning(_("Writing tags to file %s failed."),(const wxChar *)song.MetaData.Filename.GetFullPath());

   return rc != CMetaDataHandler::fail;
}


int CMusikLibrary::ClearDirtyTags()
{
	int nCount = QueryCount("select count(*) from songs where dirty = 1;");
	m_pDB->Exec("update songs set dirty = 0 where dirty = 1;");
	return nCount;
}

int CMusikLibrary::db_callbackAddToIntArray(void *args, int WXUNUSED(numCols), char **results, char ** WXUNUSED(columnNames))
{

	wxArrayInt * p = (wxArrayInt*)args;
	p->Add( atoi(results[0]) ); 
    return 0;
}
int CMusikLibrary::db_callbackAddToStringArray(void *args, int WXUNUSED(numCols), char **results, char ** WXUNUSED(columnNames))
{

	wxArrayString * p = (wxArrayString*)args;
	p->Add( ConvFromUTF8( results[0] )); 
	return 0;
}

void _Add_IN_ClauseForColumn(wxString & query,const PlaylistColumn & Column,const wxArrayString & aList)
{
    bool bQuote = (Column.Type == PlaylistColumn::Textual);

    query << wxString::Format(Column.ColQueryMask,Column.DBName.c_str())
        << wxT(" in (");
    for ( int i = 0; i < (int)aList.GetCount(); i++ )
    {
        if ( i > 0 )
            query << wxT(',');
        if(bQuote)
        {
            query << wxT('\'');
            wxString s (aList.Item( i ));
            s.Replace( wxT("'"), wxT("''") );
            query << s;
        }
        else
            query << aList.Item( i );
        if(bQuote)
            query << wxT('\'');

    }
    query << wxT(')');
}

void CMusikLibrary::GetInfo( const wxArrayString & aList, const PlaylistColumn & ColumnIn, const PlaylistColumn & ColumnOut ,wxArrayString & aReturn, bool bSorted )
{
	aReturn.Clear();
	wxString sInfo;
	wxString query;
	query.Alloc(50 * aList.GetCount()+ 40);
    wxString sColumnOut(wxString::Format(ColumnOut.ColQueryMask,ColumnOut.DBName.c_str()));
    sColumnOut += wxT(" as OutColumn ");
	if(bSorted)
	{
		if( wxGetApp().Prefs.bSortArtistWithoutPrefix && ColumnOut.SortOrder == PlaylistColumn::SortNoCaseNoPrefix)
			query << wxT("select distinct ") << sColumnOut << wxT(",REMPREFIX(") << sColumnOut << wxT(") as RPF from songs where ");
        else
            query = wxT("select distinct ") + sColumnOut + wxT(" from songs where ");
	}
	else
		query = wxT("select distinct ") + sColumnOut + wxT(" from songs where ");
    _Add_IN_ClauseForColumn(query,ColumnIn,aList);
	if(bSorted)
	{
		switch ( ColumnOut.SortOrder )
		{
        case PlaylistColumn::SortNoCase:
            query += wxT(" order by OutColumn collate nocase");
            break;
		case PlaylistColumn::SortNoCaseNoPrefix:
			query += wxT(" order by RPF collate nocase");
			break;

        case PlaylistColumn::SortCase:
			query += wxT(" order by OutColumn");
			break;

        case PlaylistColumn::SortNone:
			break;
		}
	}
	query << wxT(';');
    
    Query(query,aReturn,false);
}
 int CMusikLibrary::db_callbackAddToSongIdArray(void *args, int WXUNUSED(numCols), char **results, char ** WXUNUSED(columnNames))
{

	
	MusikSongIdArray * p = (MusikSongIdArray*)args;
	p->Add(MusikSongId( StringToInt(results[0])));
    return 0;
}
void CMusikLibrary::GetSongs( const wxArrayString & aList, const PlaylistColumn & Column, MusikSongIdArray & aReturn )
{
	aReturn.Clear();
	wxString sInfo;
	aReturn.Alloc(GetSongCount()); // optimize item adding performance,
  	wxString sQuery;
  
	sQuery.Alloc(sQuery.Len() + 30 + aList.GetCount() * 30); // optimization ( the 30 is a wild guess)
    _Add_IN_ClauseForColumn(sQuery,Column,aList);
	QuerySongsWhere( sQuery, aReturn,true); // query sorted
	return;
}

void CMusikLibrary::Query( const wxString & query, wxArrayString & aReturn ,bool bClearArray )
{
	if(bClearArray)
	{
	    aReturn.Clear();
	    //--- run the query ---//
	    aReturn.Alloc( GetSongCount() );// just a guess
	}
    MusikDb::ResultCB cb(&aReturn, &db_callbackAddToStringArray);
    m_pDB->Exec(ConvQueryToMB( query ),cb);
}
void CMusikLibrary::Query( const wxString & query, wxArrayInt & aReturn ,bool bClearArray )
{
	if(bClearArray)
	{

		aReturn.Clear();
		//--- run the query ---//
		aReturn.Alloc( GetSongCount() );
	}
    MusikDb::ResultCB cb(&aReturn, &db_callbackAddToIntArray);
 	m_pDB->Exec( ConvQueryToMB( query ), cb );
}

int CMusikLibrary::db_callbackAddToSongIdMap(void *args, int WXUNUSED(numCols), char **results, char ** WXUNUSED(columnNames))
{
	//-------------------------------------------------------------------------//
	//--- maps filename to CMusingSong objects ptrs, ptrs because this		---//
	//--- way an additional call to a copy constructer is saved when adding	---//
	//--- the objects to the map											---//
	//-------------------------------------------------------------------------//

	myStringToMusikSongIdPtrMap * p = (myStringToMusikSongIdPtrMap*)args;
	(*p)[ConvFromUTF8( results[1] )]= new MusikSongId(StringToInt(results[0]));

    return 0;
}
void CMusikLibrary::GetFilelistSongs( const wxArrayString & aFiles, MusikSongIdArray & aReturn )
{
	aReturn.Clear();
	if(aFiles.GetCount() == 0)
        return;
	wxString sQuery = wxT("select songs.songid,songs.filename from songs where ");

	sQuery.Alloc(sQuery.Len() + aFiles.GetCount() * 30); // optimization ( the 30 is a wild guess)
    _Add_IN_ClauseForColumn(sQuery,g_PlaylistColumn[PlaylistColumn::FILENAME],aFiles);
    sQuery << wxT(";");

	myStringToMusikSongIdPtrMap theMap;
	//---------------------------------------------------------------------//
	//--- we fill the map and afterwards a array from the map because	---//
	//--- we can have multiple filenames in the same list				---//
	//---------------------------------------------------------------------//
    MusikDb::ResultCB cb(&theMap, &db_callbackAddToSongIdMap);
    m_pDB->Exec( ConvQueryToMB( sQuery ),cb);

	aReturn.Alloc( aFiles.GetCount() );
	for ( size_t i = 0; i < aFiles.GetCount(); i++ )
	{
		MusikSongId * pSongid = theMap[ aFiles.Item( i ) ];
	//	wxASSERT_MSG( pSong, wxString(aFiles.Item( i ) + wxT( " is not on the map!" ) ) );

		//---------------------------------------------------------------------//
		//--- add the object(of the map) by value, to create duplicate		---// 
		//--- entries if needed.											---//
		//---------------------------------------------------------------------//
		if( pSongid )
			aReturn.Add( *pSongid ); 
	}

	//-------------------------------------------------------------------------//
	//--- delete all map objects( this is done explicitly, because the map	---//
	//--- contains pointers to objects)										---//
	//-------------------------------------------------------------------------//
	WX_CLEAR_HASH_MAP(myStringToMusikSongIdPtrMap, theMap); 

	return;
}

void CMusikLibrary::SetSortOrderColumn( const PlaylistColumn & Column, bool descending )
{
	m_sSortAllSongsQuery.Empty();
	const wxString & sortstr = Column.DBName;
	m_sSortAllSongsQuery << wxT("select distinct songid");
    if(wxGetApp().Prefs.bSortArtistWithoutPrefix && (Column.SortOrder == PlaylistColumn::SortNoCaseNoPrefix) )
	{
		m_sSortAllSongsQuery << wxT(",REMPREFIX(") << sortstr << wxT(") as RPF") << sortstr;
	}
	m_sSortAllSongsQuery << wxT(" from songs ");

	m_sSortAllSongsQuery << wxT("%s"); // add placeholder for possible where clause

    m_sSortAllSongsQuery << wxT(" order by ");

    if(wxGetApp().Prefs.bSortArtistWithoutPrefix && (Column.SortOrder == PlaylistColumn::SortNoCaseNoPrefix) )
		m_sSortAllSongsQuery << wxT("RPF");
    m_sSortAllSongsQuery << sortstr;
    if(Column.SortOrder != PlaylistColumn::SortCase)
        m_sSortAllSongsQuery << wxT(" collate nocase");
    if ( descending )
		m_sSortAllSongsQuery << wxT(" desc");

	if(sortstr == wxT("artist"))
		m_sSortAllSongsQuery << wxT(" ,album collate nocase, tracknum");
	else if(sortstr == wxT("album")) 
		m_sSortAllSongsQuery << wxT(" ,tracknum,artist collate nocase");
	else if(sortstr == wxT("genre")) 
		m_sSortAllSongsQuery << wxT(" ,artist collate nocase,album collate nocase,tracknum");
	else if(sortstr == wxT("year")) 
		m_sSortAllSongsQuery << wxT(" ,artist collate nocase,album collate nocase,tracknum");
	m_sSortAllSongsQuery << wxT(';');
	return;
}

///////////////////////////////////////////////////////////////////////////////

double CMusikLibrary::GetSum(const wxString & sField, const MusikSongIdArray &  idarray ) const
{
	if ( !idarray.GetCount())
		return 0.0;
    wxString sQuery(wxT("select sum(") + sField +wxT(") from songs where songid in ("));
    idarray.AddCommaSeparated(sQuery);
    sQuery << wxT(");");
	//--- run query ---//
	double  totsize = 0.0;
	m_pDB->Exec(ConvQueryToMB( sQuery ),&totsize);
	return totsize;
}

///////////////////////////////////////////////////////////////////////////////
void CMusikLibrary::RedoLastQuerySongsWhere( MusikSongIdArray & aReturn ,bool bSorted)
{
	QuerySongsWhere(m_lastQueryWhere,aReturn,bSorted);
}
void CMusikLibrary::QuerySongsWhere( const wxString & queryWhere, MusikSongIdArray & aReturn ,bool bSorted,bool bClearArray)
{
	if(bClearArray)
		aReturn.Clear();
	//--- run query ---//
	wxString query;
	wxString myqueryWhere = queryWhere.IsEmpty()  ? wxString(wxT("")) : wxString(wxT(" where ")) + queryWhere;
	m_lastQueryWhere = queryWhere;
	if( bSorted && !m_sSortAllSongsQuery.IsEmpty() )
	{
		query = wxString::Format(  m_sSortAllSongsQuery , (const wxChar *)myqueryWhere );
	}
	else
	{
		query = wxT("select distinct songid from songs ");
		query += myqueryWhere; 
		query += wxT(";");		
	}
	aReturn.Alloc(GetSongCount());
    MusikDb::ResultCB cb(&aReturn,&db_callbackAddToSongIdArray);
    m_pDB->Exec( ConvQueryToMB(query), cb);
	aReturn.Shrink();
	return;
}


void CMusikLibrary::QuerySongsFrom( const wxString & queryFrom, MusikSongIdArray & aReturn ,bool bSorted)
{
	aReturn.Clear();
	//--- run query ---//
	wxString queryTail(queryFrom);
    bool bDistinct = true;
    bDistinct = !queryFrom.StartsWith(wxT("NODISTINCT "),&queryTail);
	wxString myqueryFrom = wxT(" FROM ") + queryTail;

    wxString query;
	if( bSorted && !m_sSortAllSongsQuery.IsEmpty() )
	{
		query = wxString::Format(  m_sSortAllSongsQuery , myqueryFrom.c_str() );
	}
	else
	{
		query = wxT("select");
        if(bDistinct)
            query += wxT(" distinct");
        query += wxT(" songid ");
		query += myqueryFrom; 
		query += wxT(";");		
	}
	aReturn.Alloc(GetSongCount());
    MusikDb::ResultCB cb(&aReturn,&db_callbackAddToSongIdArray);
    m_pDB->Exec( ConvQueryToMB(query), cb );
	aReturn.Shrink();
	return;
}

void CMusikLibrary::UpdateItemLastPlayed( int  songid  )
{

    if(m_pDB->Exec(MusikDb::QueryString( "update songs set lastplayed = julianday('now'), timesplayed = timesplayed + 1 where songid = %d;",songid )))
        OnSongDataChange(songid);
}


void CMusikLibrary::RecordSongHistory( const MusikSongId & songid ,int playedtime)
{
	bool bSelectedByUser = songid.bChosenByUser == 1;
	int nDuration_ms = songid.Song()->MetaData.nDuration_ms;
	int percentplayed = playedtime ? 
                            (playedtime * 100 / (nDuration_ms ? nDuration_ms : playedtime))
                            :0; // be safe against integer division by zero
    m_pDB->Exec(MusikDb::QueryString( "insert into songhistory values ( %d, julianday('now'),%d,%d );",
		                                songid.Id() ,percentplayed ,bSelectedByUser));
}

void CMusikLibrary::UpdateItemResetDirty( int  songid )
{
    m_pDB->Exec(MusikDb::QueryString( "update songs set dirty = 0 where songid = %d;",
                                        songid));
}

int CMusikLibrary::GetSongCount()
{

	if(m_nCachedSongCount == -1 )
	{
		m_nCachedSongCount = QueryCount("select count(*) from songs;");
	}
	return m_nCachedSongCount;
}

bool CMusikLibrary::GetSongFromSongid( int songid, CMusikSong *pSong )
{
    wxCriticalSectionLocker lock( m_csCacheAccess );
    tSongCacheMap::const_iterator i = m_mapSongCache.find(songid);
    if(i == m_mapSongCache.end())
    { // songid is not in cache
        // load to cache
        CMusikSong &SongCopy = m_mapSongCache[songid];
        if(!QuerySongFromSongid(songid,&SongCopy))
        { // songid not found in db, erase from map 
            m_mapSongCache.erase(songid);
            return false;
        }
        *pSong = SongCopy;
        return true;
    }
    *pSong = (*i).second;
    return true;
}

bool CMusikLibrary::QuerySongFromSongid( int songid, CMusikSong *pSong )
{
	pSong->songid = -1;	
	//--- run query ---//
    MusikDb::ResultCB cb(pSong,&db_callbackAssignSongTableColumnDataToSong);
	if( !m_pDB->Exec(MusikDb::QueryString(ConvQueryToMB( wxT("select ") MUSIK_LIB_ALL_SONGCOLUMNS wxT(" from songs where songid = %d;")), songid )
        ,cb))
    {
        return false;
    }
    return pSong->songid == songid;
}
bool CMusikLibrary::UpdateItem( MusikSongId &songinfoid, bool bDirty )
{
   bool bRes = UpdateItem(songinfoid.Id(),songinfoid.SongCopy().MetaData,bDirty);
   if(bRes)
    songinfoid.Check1 = 0;
   return bRes;
}
bool CMusikLibrary::UpdateItem(  int  songid ,const CSongMetaData &MetaData, bool bDirty)
{
    // this only updates user changeable properties of the song.	
    if(!m_pDB->Exec(MusikDb::QueryString("update songs set artist=%Q, title=%Q,"
											"album=%Q, tracknum=%d, year=%Q, genre=%Q,"
											"notes=%Q, dirty=%d, modified=julianday('now') where songid = %d;",
			( const char* )MetaData.Artist, 
			( const char* )MetaData.Title , 
			( const char * )MetaData.Album , 
			MetaData.nTracknum, 
			( const char* )MetaData.Year, 
			( const char* )MetaData.Genre,
			( const char* )MetaData.Notes, 
			(int)bDirty, 
			 songid)))
    {
		wxMessageBox( _( "An error occurred when attempting to update the database" ), MUSIKAPPNAME_VERSION, wxOK | wxICON_ERROR );
        return false;
    }
    OnSongDataChange(songid);
    return true;

}


int CMusikLibrary::GetSongDirCount( wxString sDir )
{
    int result = QueryCount(MusikDb::QueryString( "select count(*) from songs where filename like '%q%%';", ( const char* )ConvToUTF8(sDir) ));
	return result;
}
int CMusikLibrary::QueryCount(const char * szQuery )
{
	int result = 0;
    if(!m_pDB->Exec(szQuery,&result))
        return -1;
    return result;
}

void CMusikLibrary::SetRating( int songid, int nVal )
{
	nVal = wxMin(wxMax(nVal,MUSIK_MIN_RATING),MUSIK_MAX_RATING);
    if(m_pDB->Exec(MusikDb::QueryString( "update songs set rating = %d where songid = %d;",
		            nVal, songid )))
    {
        OnSongDataChange(songid);
    }
}

bool CMusikLibrary::FindOrPurgeMissing( const wxString & filename )
{
	if ( !wxFileExists( filename ) )
	{   
		int songidMissing = QueryCount(MusikDb::QueryString( "select distinct songs.songid from songs where filename = %Q;", ( const char* )ConvToUTF8(filename) ));
		CMusikSong songMissing;
		QuerySongFromSongid(songidMissing,&songMissing);
		wxFileName fn(filename);
		wxString Name;
		Name << wxString(wxFileName::GetPathSeparator()) << fn.GetName() << wxT(".");
		// search for "/xxx." in filenames, files where only the path has been changed or the file type, will be found this way
		int songidCandidate = QueryCount(MusikDb::QueryString( "select distinct songs.songid from songs where filename like '%%%q%%' and songid <> %d ;",
								( const char* )ConvToUTF8(Name), songidMissing));
		CMusikSong songCandidate;
		if(songidCandidate != -1)
		{	
			QuerySongFromSongid(songidCandidate,&songCandidate);
			if(songCandidate.MetaData.nDuration_ms != songMissing.MetaData.nDuration_ms
				|| songCandidate.TimeAdded < songMissing.TimeAdded)
			{
				songCandidate.songid = -1;// mark as nothing found
			}
		}
		if(songCandidate.songid == -1)
		{
			// now we try to find a song entry which has the same title and artist
			songidCandidate = QueryCount(MusikDb::QueryString( "select distinct songs.songid from songs where title = %q and artist = %q and album = %q;", 
														songMissing.MetaData.Title.c_str(),songMissing.MetaData.Artist.c_str(),songMissing.MetaData.Album.c_str()));
			QuerySongFromSongid(songidCandidate,&songCandidate);
			if(songCandidate.MetaData.nDuration_ms != songMissing.MetaData.nDuration_ms
				|| songCandidate.TimeAdded < songMissing.TimeAdded)
			{
				songCandidate.songid = -1;// mark as nothing found
			}
		}
		if(songCandidate.songid == -1)
		{
			// we did not found any candidate, song is really missing, so we delete it from db
			RemoveSong(songidMissing);
			return true;
		}

		
		RemoveSong(songidCandidate);
		m_pDB->Exec(MusikDb::QueryString("update songs set filename=%Q where songid = %d", 
						( const char* )ConvToUTF8(songCandidate.MetaData.Filename.GetFullPath()),
						songidMissing));
		UpdateFullSongData(songidMissing,songCandidate.MetaData);
		return true;
	}
	return false;// db  not modified
}

void CMusikLibrary::RemoveSongDir( const wxString &  sDir )
{
	if(m_pDB->Exec(MusikDb::QueryString("delete from songs where filename like '%q%%'",( const char* )ConvToUTF8(sDir) )))	
    {
        m_nCachedSongCount = -1;
        OnSongDataChange();
    }
}

void CMusikLibrary::RemoveSong( const wxString & sSong	)	
{
    int songid = -1;
	if(m_pDB->Exec(MusikDb::QueryString("select songid from songs where filename = '%q'", 
	               ( const char* )ConvToUTF8( sSong ) ), &songid))
	{
        RemoveSong(songid);	   
	}
}
void CMusikLibrary::RemoveSong( int songid )	
{
	if(m_pDB->Exec(MusikDb::QueryString("delete from songs where songid = %d", songid)))
    {
    	m_nCachedSongCount = -1;
        OnSongDataChange(songid);
    }
}

void CMusikLibrary::RemoveAll()
{ 
	if(m_pDB->Exec(MusikDb::QueryString( "delete from songs;")))
    {
	    m_nCachedSongCount = 0;
        OnSongDataChange();
    }
}

bool CMusikLibrary::ReplaceMask( wxString *sTarget,const  wxString & sMask, const wxString &sReplaceBy,const  wxString &sDefault,bool bReplaceAll )
{
	wxString sCheck( sReplaceBy ); 
	sCheck.Replace( wxT( " " ), wxT( "" ), true );

	if ( sCheck.IsEmpty() )
		sTarget->Replace( sMask, sDefault, bReplaceAll );
	else
		sTarget->Replace( sMask, sReplaceBy, bReplaceAll );
	return true;

}


bool CMusikLibrary::RenameFile( CMusikSong & song )
{
	//--------------------------------//
	//--- new filename information ---//
	//--------------------------------//
	
	wxString sPrePath	= song.MetaData.Filename.GetPath( wxPATH_GET_VOLUME | wxPATH_GET_SEPARATOR );
	wxString sFile		= wxGetApp().Prefs.sAutoRename;

	wxString sCheck;

	//---------------------------------------------//
	//--- the song's root directory. this will	---//
	//--- only affect us if there is directory	---//
	//--- seperation stuff going on.			---//
	//---------------------------------------------//
	wxString sRootPath = sPrePath;
	if ( wxGetApp().Prefs.sAutoRename.Find( wxFileName::GetPathSeparator() ) > - 1 )
	{
		sRootPath = MusikGetRootPath(song.MetaData.Filename.GetPath());
		if(sRootPath.IsEmpty())
			return false;
	}

	//--------------------------------------------//
	//--- format the output filename (without) ---//
	//--- path. e.g.. 01 - STP - Meat Plow.mp3 ---//
	//--------------------------------------------//

	//----------------------------------------------//
	//--- replace masked values					 ---//
	//----------------------------------------------//
	wxString sForbiddenChars = GetForbiddenChars();
	wxString sTitle = ConvFromUTF8( song.MetaData.Title );
	//--- on some platforms, we can't have certain characters, so blank them out ---//
	ReplaceChars(sTitle,sForbiddenChars);
	if ( !ReplaceMask( &sFile, wxT( "%1" ),sTitle) )
		return false;
	wxString sArtist = ConvFromUTF8( song.MetaData.Artist );
	ReplaceChars(sArtist,sForbiddenChars);
	if ( !ReplaceMask( &sFile, wxT( "%2" ),sArtist) )
		return false;
	wxString sAlbum = ConvFromUTF8( song.MetaData.Album );
	ReplaceChars(sAlbum,sForbiddenChars);
	if ( !ReplaceMask( &sFile, wxT( "%3" ),sAlbum) )
		return false;
	wxString sGenre = ConvFromUTF8( song.MetaData.Genre );
	ReplaceChars(sGenre,sForbiddenChars);
	if ( !ReplaceMask( &sFile, wxT( "%4" ),sGenre) )
		return false;
	wxString sYear = ConvFromUTF8( song.MetaData.Year );
	ReplaceChars(sYear,sForbiddenChars);
	if ( !ReplaceMask( &sFile, wxT( "%5" ),sYear) )
		return false;
	wxString sTrackNum=wxString::Format(wxT("%.2d"), song.MetaData.nTracknum );
	if ( !ReplaceMask( &sFile, wxT( "%6" ), sTrackNum ) )
		return false;

	//--- final name ---//
	wxFileName newfilename;
	newfilename.AssignDir(sRootPath);
	newfilename.SetName(sFile);
	newfilename.SetExt(song.MetaData.Filename.GetExt());
//	newfilename.Normalize();
	//--- filename already the same? return ---//
	if ( song.MetaData.Filename == newfilename )
	{
		return true;
	}

	//-----------------------------------------//
	//--- create needed directories and do	---//
	//--- any sort of filename modification	---//
	//--- that is needed					---//
	//-----------------------------------------//
	if(!wxFileName::Mkdir(newfilename.GetPath(),0777,wxPATH_MKDIR_FULL))
		return false;


	//-----------------------------------------//
	//--- file does need to be renamed, so	---//
	//--- rename it, then return			---//
	//-----------------------------------------//
	if ( wxRenameFile( song.MetaData.Filename.GetFullPath(), newfilename.GetFullPath() ) )
	{
		song.MetaData.Filename = newfilename;
        if( m_pDB->Exec(MusikDb::QueryString("update songs set filename =%Q where songid = %d;",
				( const char* )ConvToUTF8( song.MetaData.Filename.GetFullPath() ),
				 song.songid )))
        {
            OnSongDataChange(song.songid);
            return true;
        }
	}
	
	//-----------------------------------------//
	//--- how could we get here? we can't	---//
	//--- so just return false				---//
	//-----------------------------------------//
	return false;
}

bool CMusikLibrary::RetagFile(const CMusikTagger & tagger, CMusikSong & Song )
{
	
	if(!tagger.Retag(&Song))
		return false;
	UpdateItem( Song.songid,Song.MetaData, true );
	return true;
}

//-----------------------------------//
//--- pre-defined queries to make ---//
//---   life a little bit easier  ---//
//-----------------------------------//

void CMusikLibrary::GetAllSongs( MusikSongIdArray & aReturn, bool bSorted )
{
	//QuerySongsWhere( wxT(""), aReturn ,bSorted); for some unknown reason linux (suse 8.2 with wxGTK2.5.1 segfaults
	QuerySongsWhere( wxString(), aReturn ,bSorted);
}

void CMusikLibrary::GetAllOfColumn( const PlaylistColumn & Column, wxArrayString & aReturn, bool bSorted  )
{
    wxString sColumn = wxString::Format(Column.ColQueryMask,Column.DBName.c_str());
	if(bSorted)
	{
        if(wxGetApp().Prefs.bSortArtistWithoutPrefix && Column.SortOrder == PlaylistColumn::SortNoCaseNoPrefix)
            Query( wxT("select distinct ") + sColumn + wxT(",UPPER(REMPREFIX(") + sColumn + wxT(")) as UP from songs order by UP;"), aReturn );
		else if(Column.SortOrder == PlaylistColumn::SortNoCase)	
			Query( wxT("select distinct ") + sColumn + wxT(",UPPER(") + sColumn + wxT(") as UP from songs order by UP;"), aReturn );
        else
            Query( wxT("select distinct ") +  sColumn + wxT(" from songs order by ")+ sColumn +  wxT(";"), aReturn );
	}
	else
		Query( wxT("select distinct ") +  sColumn + wxT(" from songs;"), aReturn );


}

bool CMusikLibrary::SetAutoDjFilter(const wxString & sFilter)
{ 
    wxLogNull lognull;
	m_pDB->Exec("DROP VIEW autodj_songs;");
    return m_pDB->Exec(MusikDb::QueryString("CREATE VIEW autodj_songs as select * from songs where %s;",
				   ( const char* )ConvToUTF8(sFilter)));	
}
bool CMusikLibrary::SetCacheSize(int size)
{
   return m_pDB->Exec(MusikDb::QueryString("PRAGMA cache_size = %d;",size));
}
