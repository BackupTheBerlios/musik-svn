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
#include "wx/wxprec.h"
#include "MusikLibrary.h"

//--- globals ---//
#include "../MusikGlobals.h"
#include "../MusikUtils.h"

//--- wx ---//
#include <wx/file.h>
#include <wx/filename.h>

#include "3rd-Party/Bitap/libbitap.h"
#include <wx/arrimpl.cpp>

#include "Library/MetaDataHandler.h"

WX_DEFINE_OBJARRAY( CMusikSongArray )
WX_DECLARE_STRING_HASH_MAP( CMusikSong *, myStringToMusikSongPtrMap );

//--- frames ---//
//#include "../Frames/MusikFrame.h"

CMusikSong::CMusikSong()
{
	Rating 		= 0;
	TimesPlayed = 0;
	Check1 		= 0;
	songid		= -1;
	LastPlayed = TimeAdded	= 0.0;
	bChosenByUser = 1;
	bForcePlay = 0;

}


CMusikLibrary::CMusikLibrary()
	: wxEvtHandler()
{
	m_nCachedSongCount = -1;
	m_pDB = NULL;
	SetSortOrderField( PLAYLISTCOLUMN_ARTIST );
}

CMusikLibrary::~CMusikLibrary()
{
	Shutdown();
}

void CMusikLibrary::Shutdown()
{
	//--- close database if necessary ---//
	wxCriticalSectionLocker lock( m_csDBAccess );
	if ( m_pDB )
	{
		sqlite_close( m_pDB );
		m_pDB = NULL;
	}	
}

bool CMusikLibrary::Load()
{
	wxString sFilename = MUSIK_DB_FILENAME;
		
	//--- look for database.. if need be, create it and create tables ---//
	char *errmsg = NULL;
	static	const char *szCreateVersionQuery =
		"CREATE TABLE version ( " 	
		"name, major, majorsub ,minor, minorsub "
		" );";
	
	//--- create the tables ---//
	static const char *szCreateSongTableQuery  = 
			"CREATE TABLE songs ( "	
			"songid INTEGER PRIMARY KEY, "
			"format number(1), "		
			"vbr number(1), "			
			"filename varchar(255), "	
			"artist varchar(255), "	
			"title varchar(255), "	
			"album varchar(255), "	
			"tracknum number(3), "	
			"year varchar(255), "		
			"genre varchar(255), "	
			"rating number(1), "		
			"bitrate number(10), "	
			"lastplayed timestamp, "	
			"notes varchar(255), "	
			"timesplayed number(5), "	
			"duration number(10), "	
			"timeadded timestamp, "	
			"filesize number(10), "	
			"dirty number(10) "		
			" );";
	const char* szCreateSongTableIdxQuery =
			"CREATE INDEX songs_title_idx on songs (title);"
			"CREATE UNIQUE INDEX songs_filename_idx on songs (filename);"
			"CREATE INDEX songs_artist_idx on songs (artist);"
			"CREATE INDEX songs_album_idx on songs (album);"
			"CREATE INDEX songs_genre_idx on songs (genre);"
			"CREATE INDEX songs_tracknum_idx on songs (tracknum);"
			"CREATE INDEX songs_artist_album_tracknum_idx on songs (artist,album,tracknum);"
			"CREATE INDEX songs_timeadded_idx on songs (timeadded);"
			"CREATE INDEX songs_lastplayed_idx on songs (lastplayed);"
			;
	static	const char *szCreateSongHistoryQuery =
			"CREATE TABLE songhistory ( "	
				"songid number(10), "
				"date_played timestamp , "
				"percent_played number(10),"
				"selected_by_user number(1) "
				" );";

	static const char* szCreateSongHistoryTableIdxQuery =
		"CREATE INDEX songhistory_songid_idx on songhistory (songid);"
		"CREATE INDEX songhistory_date_played_idx on songhistory (date_played);"
		"CREATE INDEX songhistory_percent_played_idx on songhistory (percent_played);"
		;
	
	m_pDB = sqlite_open( wxConvertWX2MB(sFilename), 0666, &errmsg );

	if( m_pDB )
	{

		// always create table, if it exists an error will be returned by sqlite_exec, but we dont care.
		sqlite_exec( m_pDB, szCreateVersionQuery, NULL, NULL, NULL );
		sqlite_exec( m_pDB, szCreateSongTableQuery, NULL, NULL, NULL );
		sqlite_exec( m_pDB, szCreateSongTableIdxQuery, NULL, NULL, NULL );
		sqlite_exec( m_pDB, szCreateSongHistoryQuery, NULL, NULL, NULL );
		sqlite_exec( m_pDB, szCreateSongHistoryTableIdxQuery, NULL, NULL, NULL );
		sqlite_exec( m_pDB,	"CREATE TRIGGER song_deleted_trigger DELETE ON songs " 
							"BEGIN "
							"DELETE FROM songhistory WHERE songid = old.songid;"
							"END;", NULL, NULL, NULL );


		CreateDBFuncs();
		sqlite_exec( m_pDB, "PRAGMA synchronous = OFF;", NULL, NULL, NULL );
		sqlite_exec( m_pDB, "PRAGMA cache_size = 10000;", NULL, NULL, NULL );
		CheckVersion();
		SetAutoDjFilter(wxGetApp().Prefs.sAutoDjFilter );
		sqlite_exec( m_pDB,	"CREATE VIEW valid_albums as select album,artist,most_lastplayed from ("
			"select album,artist,sum(duration) as sum_duration,max(lastplayed+0) as most_lastplayed "  
			"from songs where album != '' group by album) where sum_duration > 1500000;"
			, NULL, NULL, NULL );	
		sqlite_exec( m_pDB,	"CREATE VIEW autodj_albums as select album,artist,most_lastplayed from ("
			"select album,artist,sum(duration) as sum_duration,max(lastplayed+0) as most_lastplayed "  
			"from autodj_songs where album != '' group by album) where sum_duration > 1500000;"
			, NULL, NULL, NULL );	

	}
	if ( errmsg )
			free( errmsg );
	return m_pDB != NULL;
}

void CMusikLibrary::CheckVersion()
{
	
	int ver_major = -1;
	int ver_majorsub = -1;
	int ver_minor = -1;
	int ver_minorsub = -1;
	//--- run query ---//
	const char *pTail;
	sqlite_vm *pVM;

	sqlite_compile( m_pDB, "select name,major,majorsub,minor,minorsub from version where name='wxMusik';", &pTail, &pVM, NULL );
	char *errmsg = NULL;
	int numcols = 0;
	const char **coldata;
	const char **coltypes;
	unsigned long oldversion = 0;

	//--- look and see if there's one row ---//
	if ( sqlite_step( pVM, &numcols, &coldata, &coltypes ) == SQLITE_ROW )
	{
		ver_major = atoi(coldata[1]);
		ver_majorsub = atoi(coldata[2]);
		ver_minor = atoi(coldata[3]);
		ver_minorsub = atoi(coldata[4]);
		oldversion = MUSIK_VERSION(ver_major,ver_majorsub,ver_minor,ver_minorsub);
	}
	//--- close up ---//
	sqlite_finalize( pVM, &errmsg );
	sqlite_freemem(errmsg);

	if(oldversion > 0)
	{
		
		if(MUSIK_VERSION_CURRENT == oldversion)
		{
			// nothing to do
			return;
		}
		// update version info
		sqlite_exec_printf( m_pDB,	"update version set major=%d,majorsub=%d,minor=%d,minorsub=%d where name = 'wxMusik';",
			NULL,NULL,NULL,MUSIK_VERSION_MAJOR,MUSIK_VERSION_MAJORSUB,MUSIK_VERSION_MINOR,MUSIK_VERSION_MINORSUB
			);
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
			sqlite_exec( m_pDB, "update songs set lastplayed = cnvMusikOldDTFormatToJulianday(lastplayed), timeadded = cnvMusikOldDTFormatToJulianday(timeadded) where 1;", NULL, NULL, NULL );

		}

		// convert <unknown> and 0 entrys to ''
		sqlite_exec( m_pDB, "update songs set album = '' where album = '<unknown>';"
							"update songs set artist = '' where artist = '<unknown>';"		
							"update songs set genre = '' where genre = '<unknown>';"		
							"update songs set year = '' where year = '<unknown>' or year=0;"		
							"update songs set notes = '' where  notes = '<unknown>' ;"		
							"update songs set lastplayed = '' where  lastplayed isnull;"		
							"update songs set tracknum = '' where tracknum = '<unknown>' OR tracknum = 0 OR tracknum isnull;"		
							, NULL, NULL, NULL );


		// convert iso8859-1 encoding to utf-8
		sqlite_exec( m_pDB, "update songs set artist = cnvISO8859_1ToUTF8(artist),"
							" album = cnvISO8859_1ToUTF8(album),"		
							" title = cnvISO8859_1ToUTF8(title),"		
							" genre = cnvISO8859_1ToUTF8(genre),"		
							" filename = cnvISO8859_1ToUTF8(filename) where 1; "
							, NULL, NULL, NULL );

		// insert version info
		sqlite_exec_printf( m_pDB,	"INSERT INTO version values('wxMusik',%d,%d,%d,%d);",
			NULL,NULL,NULL,MUSIK_VERSION_MAJOR,MUSIK_VERSION_MAJORSUB,MUSIK_VERSION_MINOR,MUSIK_VERSION_MINORSUB
			);

	}
	// now lets do conversion depending on the version number

	if(oldversion < MUSIK_VERSION(0,3,1,1))
	{
	   // rating is from -9 to 9 now instead of 0 - 5
		sqlite_exec_printf( m_pDB,	"UPDATE songs set rating = ((rating * 9)/5 )% 10;",
			NULL,NULL,NULL);
	}

}
void CMusikLibrary::CreateDBFuncs()
{
	wxASSERT(m_pDB);
	static struct 
	{
		char *zName;
		int nArg;
		int dataType;
		void (*xFunc)(sqlite_func*,int,const char**);
		void * userdata;
	} aFuncs[] = 
			{
				{ "remprefix",      1, SQLITE_TEXT, remprefixFunc, 0 },
				{ "cnvISO8859_1ToUTF8",     1, SQLITE_TEXT, cnvISO8859_1ToUTF8Func, 0 },
				{ "wxjulianday", 1, SQLITE_TEXT, wxjuliandayFunc,0 },// for backward compatibility
			    { "cnvMusikOldDTFormatToJulianday",	1, SQLITE_NUMERIC, cnvMusikOldDTFormatToJuliandayFunc, 0 },
				{ "fuzzycmp",     3, SQLITE_TEXT, fuzzycmpFunc ,0},
			};
	/*  static struct {
	char *zName;
	int nArg;
	int dataType;
	void (*xStep)(sqlite_func*,int,const char**);
	void (*xFinalize)(sqlite_func*);
	} aAggs[] = {
	{ "min",    1, 0,              minStep,      minMaxFinalize },
	{ "max",    1, 0,              maxStep,      minMaxFinalize },
	{ "sum",    1, SQLITE_NUMERIC, sumStep,      sumFinalize    },
	{ "avg",    1, SQLITE_NUMERIC, sumStep,      avgFinalize    },
	{ "count",  0, SQLITE_NUMERIC, countStep,    countFinalize  },
	{ "count",  1, SQLITE_NUMERIC, countStep,    countFinalize  },
	};
	*/
	size_t i;

	for(i=0; i<sizeof(aFuncs)/sizeof(aFuncs[0]); i++)
	{
		sqlite_create_function(m_pDB, aFuncs[i].zName,aFuncs[i].nArg, aFuncs[i].xFunc, aFuncs[i].userdata);
		if( aFuncs[i].xFunc )
		{
			sqlite_function_type(m_pDB, aFuncs[i].zName, aFuncs[i].dataType);
		}
	}
	/* 
	for(i=0; i<sizeof(aAggs)/sizeof(aAggs[0]); i++){
	sqlite_create_aggregate(m_pDB, aAggs[i].zName,
			aAggs[i].nArg, aAggs[i].xStep, aAggs[i].xFinalize, 0);
	sqlite_function_type(m_pDB, aAggs[i].zName, aAggs[i].dataType);
	}
	*/
}
void CMusikLibrary::cnvISO8859_1ToUTF8Func(sqlite_func *context, int argc, const char **argv)
{
	if( argc<1 || argv[0]==0 ) return;
	const wxCharBuffer buf = ConvFromISO8859_1ToUTF8(argv[0]);
	int len =strlen(buf);
	sqlite_set_result_string(context,buf, len);
}
void CMusikLibrary::remprefixFunc(sqlite_func *context, int argc, const char **argv)
{
	if( argc<1 || argv[0]==0 ) return;
//	char * pPrefixArray[] = sqlite_user_data(context);
	static const char * pPrefixArray[] =
					{
						"The ",
						"Der ",
						"Die ",
						"Das ",
						"Le ",
						"La "
					};
	int argvlen = strlen(argv[0]);
	for(size_t i=0; i<sizeof(pPrefixArray)/sizeof(pPrefixArray[0]); i++)
	{
		int prefixlen = strlen(pPrefixArray[i]);
		if(strncasecmp(pPrefixArray[i],argv[0],prefixlen) == 0)
		{
			if(argvlen >= prefixlen)
			{
				sqlite_set_result_string(context, argv[0]+ prefixlen, argvlen - prefixlen);
				return;
			}
		}
	}
  sqlite_set_result_string(context, argv[0], argvlen);
}
void CMusikLibrary::cnvMusikOldDTFormatToJuliandayFunc(sqlite_func *context, int argc, const char **argv)
{
  if( argc<1 || argv[0]==0 ) return;
#ifdef __WXMSW__
  wxDateTime x;
  if( x.ParseFormat(ConvA2W(argv[0])))
  {
    sqlite_set_result_double(context, x.GetJulianDayNumber());
  }
#else
  struct tm tm;
  const char *result = strptime(argv[0], "%c", &tm);
  if ( !result )
        return;
  wxDateTime x(tm);
  sqlite_set_result_double(context, x.GetJulianDayNumber());
#endif
}
void CMusikLibrary::wxjuliandayFunc(sqlite_func *context, int argc, const char **argv)
{
	if( argc<1 || argv[0]==0 ) return;
	sqlite_set_result_string(context,argv[0], strlen(argv[0]));
}

void CMusikLibrary::fuzzycmpFunc(sqlite_func *context, int argc, const char **argv)
{
	if( argc!=3 || argv[0]==0 || argv[1]==0) return;
	int AllowedErrors  = argv[2]?atoi(argv[2]):0;
	const char *Pattern = argv[0];
	const char *Text =   argv[1];
	int Result = 0;
	bitapType b;
	if(NewBitap(&b,(const unsigned char*)Pattern)>=0)
	{
		if (NULL != FindWithBitap(&b, (const unsigned char*)Text,strlen (Text), AllowedErrors,NULL,NULL))
		{
			Result = 1;
		}
	}
	DeleteBitap(&b);
	sqlite_set_result_int(context,Result);
}


//---  if true, compares the full path, if false, just looks for the filename itself   ---//
//--- obviously the filename you pass will either be full or just filename accordingly ---//
bool CMusikLibrary::FileInLibrary( const wxString & filename, bool fullpath )
{
	bool result = false;

	char *query;
	if ( fullpath )
		query = sqlite_mprintf( "select songid from songs where filename = %Q;", ( const char* )ConvToUTF8(filename) );
	else
		query = sqlite_mprintf( "select songid from songs where filename like '%%%q%%';", ( const char* )ConvToUTF8(filename) );
	
	//--- run query ---//
	const char *pTail;
	sqlite_vm *pVM;
	wxCriticalSectionLocker lock( m_csDBAccess );
	sqlite_compile( m_pDB, query, &pTail, &pVM, NULL );
	char *errmsg = NULL;
	int numcols = 0;
	const char **coldata;
	const char **coltypes;

	//--- look and see if there's one row ---//
	if ( sqlite_step( pVM, &numcols, &coldata, &coltypes ) == SQLITE_ROW )
		result = true;

	//--- close up ---//
	sqlite_finalize( pVM, &errmsg );
	sqlite_freemem( query );
	sqlite_freemem(errmsg);
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
		wxCriticalSectionLocker lock( m_csDBAccess );
		m_nCachedSongCount = -1;
		sqlite_exec_printf( m_pDB, "insert into songs values (%Q ,%d, %d, %Q, %Q, %Q, %Q, %d, %Q, %Q, %d, %d, %Q, %Q, %d, %d, julianday('now'), %d, %d );", NULL, NULL, NULL, 
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

bool CMusikLibrary::UpdateSongDataFromFile( const wxString & filename )
{
	if ( filename.IsEmpty() )
		return true;

	
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

		//--- run the query ---//
		wxCriticalSectionLocker lock( m_csDBAccess );
		sqlite_exec_printf( m_pDB, "update songs set format=%d, vbr=%d, artist=%Q, title=%Q, album=%Q, tracknum=%d, year=%Q, genre=%Q, notes=%Q, bitrate=%d, duration=%d, filesize=%d, dirty=0 where filename = %Q;", NULL, NULL, NULL, 
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
			( const char* )ConvToUTF8(MetaData.Filename.GetFullPath())
			);
	
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


bool CMusikLibrary::WriteTag(  CMusikSong & song, bool ClearAll , bool bUpdateDB )
{

	CMetaDataHandler::RetCode rc  = CMetaDataHandler::success;
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
		//--- flag item as clean	---//
		//-----------------------------//
		UpdateItem( song , false );
	}
	else if(rc == CMetaDataHandler::fail)
		::wxLogWarning(_("Writing tags to file %s failed."),(const wxChar *)song.MetaData.Filename.GetFullPath());

   return rc != CMetaDataHandler::fail;
}


int CMusikLibrary::ClearDirtyTags()
{
	int nCount = QueryCount("select count(*) from songs where dirty = 1;");
	wxCriticalSectionLocker lock( m_csDBAccess );
	sqlite_exec_printf( m_pDB, "update songs set dirty = 0 where dirty = 1;",
			NULL, NULL, NULL);
	return nCount;
}




void CMusikLibrary::VerifyYearList( const wxArrayString & aList,wxArrayString & aVerifiedList )
{
	aVerifiedList.Clear();
	for ( int i = 0; i < (int)aList.GetCount(); i++ )
	{
		if ( ( !aList.Item( i ).IsEmpty() ) && ( aList.Item( i ).IsNumber() ) )
			aVerifiedList.Add( aList.Item( i ) );
	}
	return;
}
int CMusikLibrary::sqlite_callbackAddToIntArray(void *args, int WXUNUSED(numCols), char **results, char ** WXUNUSED(columnNames))
{

	wxArrayInt * p = (wxArrayInt*)args;
	p->Add( atoi(results[0]) ); 
    return 0;
}
int CMusikLibrary::sqlite_callbackAddToStringArray(void *args, int WXUNUSED(numCols), char **results, char ** WXUNUSED(columnNames))
{

	wxArrayString * p = (wxArrayString*)args;
	p->Add( ConvFromUTF8( results[0] )); 
	return 0;
}
void CMusikLibrary::GetInfo( const wxArrayString & aList, int nInType, int nOutType ,wxArrayString & aReturn, bool bSorted )
{
	aReturn.Clear();
	wxString sInfo;
	if(nInType == MUSIK_LIB_INVALID || nOutType == MUSIK_LIB_INVALID)
	{
		wxASSERT(0);
		return;
	}
	wxString query;
	query.Alloc(50 * aList.GetCount()+ 40);
	switch ( nOutType )
	{
	case MUSIK_LIB_ARTIST:
		if(bSorted)
		{
			if( wxGetApp().Prefs.bSortArtistWithoutPrefix)
				query = wxT("select distinct artist,UPPER(REMPREFIX(artist)) as UP from songs where ");
			else
				query = wxT("select distinct artist,UPPER(artist) as UP from songs where ");
		}
		else
			query = wxT("select distinct artist from songs where ");
		break;

	case MUSIK_LIB_ALBUM:
		if(bSorted)
			query = wxT("select distinct album,UPPER(album) as UP from songs where ");
		else
			query = wxT("select distinct album from songs where ");
		break;

	case MUSIK_LIB_GENRE:
		if(bSorted)
			query = wxT("select distinct genre,UPPER(genre) as UP from songs where ");
		else
			query = wxT("select distinct genre from songs where ");
		break;

	case MUSIK_LIB_YEAR:
		query = wxT("select distinct year from songs where ");
		break;

	case MUSIK_LIB_DECADE:
		break;
	}

	
	for ( int i = 0; i < (int)aList.GetCount(); i++ )
	{
		wxString itemstr = aList.Item( i );
		itemstr.Replace( wxT("'"), wxT("''") );

		if ( i > 0 )
			query += wxT("or ");

		switch ( nInType )
		{
		case MUSIK_LIB_ARTIST:
			query += wxT("artist = '");
			break;

		case MUSIK_LIB_ALBUM:
			query += wxT("album = '");
			break;

		case MUSIK_LIB_GENRE:
			query += wxT("genre = '");
			break;

		case MUSIK_LIB_YEAR:
			query += wxT("year = '");
			break;

		case MUSIK_LIB_DECADE:
			break;
		}
		query += itemstr;
		query += wxT("' ");

	}

	if(bSorted)
	{
		switch ( nOutType )
		{
		case MUSIK_LIB_ARTIST:
		case MUSIK_LIB_ALBUM:
		case MUSIK_LIB_GENRE:
			query += wxT("order by UP");
			break;

		case MUSIK_LIB_YEAR:
			query += wxT("order by year");
			break;

		case MUSIK_LIB_DECADE:
			break;
		}
	}
	query += wxT(";");

	//--- run query ---//
	{
		wxCriticalSectionLocker lock( m_csDBAccess );
		sqlite_exec(m_pDB, ConvQueryToMB( query ), &sqlite_callbackAddToStringArray, &aReturn, NULL);
	}
	if ( nOutType == MUSIK_LIB_YEAR )
	{
		wxArrayString aVerifiedList;
		VerifyYearList( aReturn , aVerifiedList );
		aReturn = aVerifiedList;
	}
	
}
 int CMusikLibrary::sqlite_callbackAddToSongArray(void *args, int WXUNUSED(numCols), char **results, char ** WXUNUSED(columnNames))
{

	
	CMusikSongArray * p = (CMusikSongArray*)args;

	CMusikSong *pLibItem = new CMusikSong();
	_AssignSongTableColumnDataToSong(pLibItem,(const char**)results);
	p->Add( pLibItem );

    return 0;
}
void CMusikLibrary::GetSongs( const wxArrayString & aList, int nInType, CMusikSongArray & aReturn )
{
	aReturn.Clear();
	wxString sInfo;
	aReturn.Alloc(GetSongCount()); // optimize item adding performance,
  	wxString sQuery;
  
   
  	if ( nInType == MUSIK_LIB_ARTIST )
    		sQuery +=wxT("artist in(");
	else if ( nInType == MUSIK_LIB_ALBUM )
		sQuery += wxT( "album in( ");
	else if ( nInType == MUSIK_LIB_GENRE )
		sQuery += wxT( "genre in( ");
	else if ( nInType == MUSIK_LIB_YEAR )
		sQuery += wxT( "year in(");
	sQuery.Alloc(sQuery.Len() + 30 + aList.GetCount() * 30); // optimization ( the 30 is a wild guess)

	//--- for each item in the input list, we're going to query the database ---//
	for ( size_t i = 0; i < aList.GetCount(); i++ )
	{	
    	aList.Item( i ).Replace( wxT( "'" ), wxT( "''" ), true );
  	 	sQuery += wxT("'");
		sQuery += aList.Item( i );
		
		//--- not at the end ---//
		if ( i != aList.GetCount() - 1 )
			sQuery += wxT("', ");
		 
		//--- at the end ---//
		else
			sQuery += wxT("' ) ");
 	}
	QuerySongsWhere( sQuery, aReturn,true); // query sorted
	return;
}

void CMusikLibrary::Query( const wxString & query, wxArrayString & aReturn ,bool bClearArray )
{
	if(bClearArray)
	{

	aReturn.Clear();
	//--- run the query ---//
	aReturn.Alloc( GetSongCount() );
	}
	wxCriticalSectionLocker lock( m_csDBAccess );
	sqlite_exec(m_pDB, ConvQueryToMB( query ), &sqlite_callbackAddToStringArray, &aReturn, NULL);
}
void CMusikLibrary::Query( const wxString & query, wxArrayInt & aReturn ,bool bClearArray )
{
	if(bClearArray)
	{

		aReturn.Clear();
		//--- run the query ---//
		aReturn.Alloc( GetSongCount() );
	}
	wxCriticalSectionLocker lock( m_csDBAccess );
	sqlite_exec(m_pDB, ConvQueryToMB( query ), &sqlite_callbackAddToIntArray, &aReturn, NULL);
}

int CMusikLibrary::sqlite_callbackAddToSongMap(void *args, int WXUNUSED(numCols), char **results, char ** WXUNUSED(columnNames))
{
	//-------------------------------------------------------------------------//
	//--- maps filename to CMusingSong objects ptrs, ptrs because this		---//
	//--- way an additional call to a copy constructer is saved when adding	---//
	//--- the objects to the map											---//
	//-------------------------------------------------------------------------//

	myStringToMusikSongPtrMap * p = (myStringToMusikSongPtrMap*)args;

	CMusikSong *pLibItem = new CMusikSong();
	_AssignSongTableColumnDataToSong(pLibItem,(const char **)results);
	(*p)[pLibItem->MetaData.Filename.GetFullPath()]= pLibItem;

    return 0;
}
void CMusikLibrary::GetFilelistSongs( const wxArrayString & aFiles, CMusikSongArray & aReturn )
{
	aReturn.Clear();
	
	wxString sQuery = wxT("select ")  MUSIK_LIB_ALL_SONGCOLUMNS  wxT(" from songs where filename in (");

	sQuery.Alloc(sQuery.Len() + aFiles.GetCount() * 30); // optimization ( the 30 is a wild guess)
	for ( size_t i = 0; i < aFiles.GetCount(); i++ )
	{
		//--- if song has a ' ---//	
		wxString filename( aFiles.Item( i ) );
		filename.Replace( wxT("'"), wxT("''"), TRUE );

		sQuery += wxT("'");
		sQuery += filename;
		//--- not at the end ---//
		if ( i != aFiles.GetCount() - 1 )
			sQuery += wxT("', ");
		//--- at the end ---//
		else
			sQuery += wxT("' );");
	}

	myStringToMusikSongPtrMap theMap;
	//---------------------------------------------------------------------//
	//--- we fill the map and afterwards a array from the map because	---//
	//--- we can have multiple filenames in the same list				---//
	//---------------------------------------------------------------------//
	{
		wxCriticalSectionLocker lock( m_csDBAccess );
		sqlite_exec(m_pDB, ConvQueryToMB( sQuery ), &sqlite_callbackAddToSongMap, &theMap, NULL);
	}

	aReturn.Alloc( aFiles.GetCount() );
	for ( size_t i = 0; i < aFiles.GetCount(); i++ )
	{
		CMusikSong *pSong = theMap[ aFiles.Item( i ) ];
	//	wxASSERT_MSG( pSong, wxString(aFiles.Item( i ) + wxT( " is not on the map!" ) ) );

		//---------------------------------------------------------------------//
		//--- add the object(of the map) by value, to create duplicate		---// 
		//--- entries if needed.											---//
		//---------------------------------------------------------------------//
		if( pSong )
			aReturn.Add( *pSong ); 
	}

	//-------------------------------------------------------------------------//
	//--- delete all map objects( this is done explicitly, because the map	---//
	//--- contains pointers to objects)										---//
	//-------------------------------------------------------------------------//
	WX_CLEAR_HASH_MAP(myStringToMusikSongPtrMap, theMap); 

	return;
}

void CMusikLibrary::SetSortOrderField( int nField, bool descending )
{
	m_sSortAllSongsQuery.Empty();
	
	bool numeric = false;
	const wxString & sortstr = g_PlaylistColumnDBNames[ nField ];	
	if ( ( sortstr == wxT("duration") ) || ( sortstr == wxT("tracknum") ) || ( sortstr == wxT("timesplayed") ) || ( sortstr == wxT("bitrate") ) || ( sortstr == wxT("lastplayed") ) )
		numeric = true;

	if ( !numeric )
	{
		m_sSortAllSongsQuery = wxT("select ") MUSIK_LIB_ALL_SONGCOLUMNS wxT(", UPPER(");
		if(wxGetApp().Prefs.bSortArtistWithoutPrefix && (sortstr == wxT("artist")) )
		{
			m_sSortAllSongsQuery += wxT("REMPREFIX(");
			m_sSortAllSongsQuery += sortstr;
			m_sSortAllSongsQuery += wxT(")");
		}
		else
			m_sSortAllSongsQuery += sortstr;

		m_sSortAllSongsQuery += wxT(") as up");
		m_sSortAllSongsQuery += sortstr;		
		m_sSortAllSongsQuery += wxT(" from songs ");
	}
	else
		m_sSortAllSongsQuery = wxT("select ") MUSIK_LIB_ALL_SONGCOLUMNS wxT(" from songs ");

	m_sSortAllSongsQuery += wxT("%s"); // add placeholder for possible where clause

	if ( !numeric )
		m_sSortAllSongsQuery += wxT(" order by up");
	else
		m_sSortAllSongsQuery += wxT(" order by ");
	m_sSortAllSongsQuery += sortstr;
	if ( descending )
		m_sSortAllSongsQuery += wxT(" desc");

	if(sortstr == wxT("artist"))
		m_sSortAllSongsQuery += wxT(" ,album , tracknum");
	else if(sortstr == wxT("album")) 
		m_sSortAllSongsQuery += wxT(" ,tracknum,artist");
	else if(sortstr == wxT("genre")) 
		m_sSortAllSongsQuery += wxT(" ,artist,album,tracknum");
	else if(sortstr == wxT("year")) 
		m_sSortAllSongsQuery += wxT(" ,artist,album,tracknum");
	m_sSortAllSongsQuery += wxT(";");

	return;
}

///////////////////////////////////////////////////////////////////////////////

double CMusikLibrary::GetTotalPlaylistSize()
{
	wxString sQuery;
	
	sQuery = wxT("select sum(filesize) from songs where songid in (");

	size_t count = g_Playlist.GetCount();
	if ( count < 1 )
		return 0.0;

	sQuery.Alloc(sQuery.Len() + count * 10); // optimization ( the 10 is a wild guess)
	for ( size_t i = 0; i < count ; i++ )
	{
		//--- if song has a ' ---//	
		const CMusikSong& song = g_Playlist.Item ( i );
		
		sQuery += wxString::Format(wxT("%d"),song.songid);
		//--- not at the end ---//
		if ( i != count - 1 )
			sQuery += wxT(", ");
		//--- at the end ---//
		else
		{
			sQuery += wxT(" );");
		}
	}


	//--- run query ---//
	const char *pTail;
	sqlite_vm *pVM;

	wxCriticalSectionLocker lock( m_csDBAccess );
	sqlite_compile( m_pDB, ConvQueryToMB( sQuery ), &pTail, &pVM, NULL );
	char *errmsg = NULL;
	int numcols = 0;
	const char **coldata;
	const char **coltypes;

	double  totsize = 0.0;
	if ( sqlite_step( pVM, &numcols, &coldata, &coltypes ) == SQLITE_ROW )
	{
		totsize = atof( coldata[0] );		
	}

	//--- close up ---//
	sqlite_finalize( pVM, &errmsg );
	sqlite_freemem(errmsg);

	return totsize;
}

///////////////////////////////////////////////////////////////////////////////
void CMusikLibrary::RedoLastQuerySongsWhere( CMusikSongArray & aReturn ,bool bSorted)
{
	QuerySongsWhere(m_lastQueryWhere,aReturn,bSorted);
}
void CMusikLibrary::QuerySongsWhere( const wxString & queryWhere, CMusikSongArray & aReturn ,bool bSorted,bool bClearArray)
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
		query = wxT("select ") MUSIK_LIB_ALL_SONGCOLUMNS wxT(" from songs");
		query += myqueryWhere; 
		query += wxT(";");		
	}
	
	const wxCharBuffer pQuery = ConvQueryToMB(query);
	aReturn.Alloc(GetSongCount());
	{
		// keep lock as short as possible by using {} scope
		wxCriticalSectionLocker lock( m_csDBAccess );
		sqlite_exec(m_pDB, pQuery, &sqlite_callbackAddToSongArray, &aReturn, NULL);
	}

	aReturn.Shrink();
	return;
}


void CMusikLibrary::QuerySongsFrom( const wxString & queryFrom, CMusikSongArray & aReturn ,bool bSorted)
{
	aReturn.Clear();
	//--- run query ---//
	wxString query;
	wxString myqueryFrom = wxT(" FROM ") + queryFrom;
	if( bSorted && !m_sSortAllSongsQuery.IsEmpty() )
	{
		query = wxString::Format(  m_sSortAllSongsQuery , (const wxChar *)myqueryFrom );
	}
	else
	{
		query = wxT("select ") MUSIK_LIB_ALL_SONGCOLUMNS;
		query += myqueryFrom; 
		query += wxT(";");		
	}

	const wxCharBuffer pQuery = ConvQueryToMB(query);
	aReturn.Alloc(GetSongCount());
	{
		// keep lock as short as possible by using {} scope
		wxCriticalSectionLocker lock( m_csDBAccess );
		sqlite_exec(m_pDB, pQuery, &sqlite_callbackAddToSongArray, &aReturn, NULL);
	}

	aReturn.Shrink();
	return;
}

void CMusikLibrary::UpdateItemLastPlayed( const CMusikSong & song )
{

	wxCriticalSectionLocker lock( m_csDBAccess );
	sqlite_exec_printf( m_pDB, "update songs set lastplayed = julianday('now'), timesplayed = timesplayed + 1 where songid = %d;",
		NULL, NULL, NULL, song.songid );
}


void CMusikLibrary::RecordSongHistory( const CMusikSong & song ,int playedtime)
{

	{
		bool bSelectedByUser = song.bChosenByUser == 1;
		
		int percentplayed = playedtime * 100 / (song.MetaData.nDuration_ms ? song.MetaData.nDuration_ms : playedtime); // be safe against integer division by zero
		wxCriticalSectionLocker lock( m_csDBAccess );
		sqlite_exec_printf( m_pDB, "insert into songhistory values ( %d, julianday('now'),%d,%d );",
			NULL, NULL, NULL,song.songid ,percentplayed ,bSelectedByUser);

	}
}

void CMusikLibrary::UpdateItemResetDirty( const CMusikSong & song )
{
	{
		wxCriticalSectionLocker lock( m_csDBAccess );
		sqlite_exec_printf( m_pDB, "update songs set dirty = 0 where songid = %d;",
			NULL, NULL, NULL,  song.songid );
	}
}
int CMusikLibrary::GetTimesPlayed( const CMusikSong & song )
{
	char *query = sqlite_mprintf( "select timesplayed from songs where songid = %d;", song.songid );
	int result = QueryCount(query);
	sqlite_freemem( query );
	return result;
}

int CMusikLibrary::GetSongCount()
{

	if(m_nCachedSongCount == -1 )
	{
		char *query = "select count(*) from songs;";
		m_nCachedSongCount = QueryCount(query);
	}
	return m_nCachedSongCount;
}

bool CMusikLibrary::GetSongFromSongid( int songid, CMusikSong *pSong )
{
	char *query = sqlite_mprintf(ConvQueryToMB( wxT("select ") MUSIK_LIB_ALL_SONGCOLUMNS wxT(" from songs where songid = %d;")), songid );
		
	//--- run query ---//
	const char *pTail;
	sqlite_vm *pVM;

	wxCriticalSectionLocker lock( m_csDBAccess );
	sqlite_compile( m_pDB, query, &pTail, &pVM, NULL );
	char *errmsg = NULL;
	int numcols = 0;
	const char **coldata;
	const char **coltypes;

	//--- look and see if there's one row ---//
	bool bFoundSong = false;
	if ( sqlite_step( pVM, &numcols, &coldata, &coltypes ) == SQLITE_ROW )
	{
		_AssignSongTableColumnDataToSong(pSong,coldata);
		bFoundSong = true;
	}

	//--- close up ---//
	sqlite_finalize( pVM, &errmsg );
	sqlite_freemem( query );
	sqlite_freemem(errmsg);

	return bFoundSong;
}

void CMusikLibrary::UpdateItem( CMusikSong & newsonginfo, bool bDirty )
{
	// this only updates user changeable properties of the song.	
	int result = 0;
	{// keep lock as short as possible by using {} scope
		wxCriticalSectionLocker lock( m_csDBAccess );
		result = sqlite_exec_printf( m_pDB, "update songs set filename=%Q, artist=%Q, title=%Q,"
											"album=%Q, tracknum=%d, year=%Q, genre=%Q, rating=%d,"
											"notes=%Q, dirty=%d where songid = %d;", NULL, NULL, NULL, 
			( const char* )ConvToUTF8( newsonginfo.MetaData.Filename.GetFullPath() ), 
			( const char* )newsonginfo.MetaData.Artist, 
			( const char* )newsonginfo.MetaData.Title , 
			( const char * )newsonginfo.MetaData.Album , 
			newsonginfo.MetaData.nTracknum, 
			( const char* )newsonginfo.MetaData.Year, 
			( const char* )newsonginfo.MetaData.Genre,
			newsonginfo.Rating, 
			( const char* )newsonginfo.MetaData.Notes, 
			(int)bDirty, 
			 newsonginfo.songid);
	}
	if ( result != SQLITE_OK )
		wxMessageBox( _( "An error occurred when attempting to update the database" ), MUSIKAPPNAME_VERSION, wxOK | wxICON_ERROR );

	newsonginfo.Check1 = 0;
}


int CMusikLibrary::GetSongDirCount( wxString sDir )
{
	char *query = sqlite_mprintf( "select count(*) from songs where filename like '%q%%';", ( const char* )ConvToUTF8(sDir) );
	int result = QueryCount(query);
	sqlite_freemem( query );
	return result;
}
int CMusikLibrary::QueryCount(const char * szQuery )
{
	int result = -1;
	//--- run query ---//
	const char *pTail;
	sqlite_vm *pVM;

	wxCriticalSectionLocker lock( m_csDBAccess );
	sqlite_compile( m_pDB, szQuery, &pTail, &pVM, NULL );
	char *errmsg = NULL;
	int numcols = 0;
	const char **coldata;
	const char **coltypes;

	//--- look and see if there's one row ---//
	if ( sqlite_step( pVM, &numcols, &coldata, &coltypes ) == SQLITE_ROW )
		result = coldata[0] ? atoi( coldata[0] ) : 0;

	//--- close up ---//
	sqlite_finalize( pVM, &errmsg );
	sqlite_freemem(errmsg);
	return result;
}

void CMusikLibrary::SetRating( int songid, int nVal )
{
	nVal = wxMin(wxMax(nVal,MUSIK_MIN_RATING),MUSIK_MAX_RATING);

	wxCriticalSectionLocker lock( m_csDBAccess );
	sqlite_exec_printf( m_pDB, "update songs set rating = %d where songid = %d;",
		NULL, NULL, NULL, nVal, songid );

}

bool CMusikLibrary::CheckAndPurge( const wxString & filename )
{
	if ( !wxFileExists( filename ) )
	{   
		RemoveSong( filename );
		return true;
	}
	return false;
}

void CMusikLibrary::RemoveSongDir( const wxString &  sDir )
{
	wxCriticalSectionLocker lock( m_csDBAccess );
	sqlite_exec_printf( m_pDB, "delete from songs where filename like '%q%%'", NULL, NULL, NULL, ( const char* )ConvToUTF8(sDir) );	
	m_nCachedSongCount = -1;
}

void CMusikLibrary::RemoveSong( const wxString & sSong	)	
{
	wxCriticalSectionLocker lock( m_csDBAccess );
	sqlite_exec_printf( m_pDB, "delete from songs where filename = '%q'", NULL, NULL, NULL, ( const char* )ConvToUTF8( sSong ) );
	m_nCachedSongCount = -1;
}
void CMusikLibrary::RemoveSong( int songid )	
{
	wxCriticalSectionLocker lock( m_csDBAccess );
	sqlite_exec_printf( m_pDB, "delete from songs where songid = %d", NULL, NULL, NULL,songid);
	m_nCachedSongCount = -1;
}

void CMusikLibrary::RemoveAll()
{ 
	wxCriticalSectionLocker lock( m_csDBAccess );
	sqlite_exec_printf( m_pDB, "delete from songs;", NULL, NULL, NULL );	
	m_nCachedSongCount = 0;
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
		if ( g_Paths.GetCount() == 0 )
			return false;

		sCheck = song.MetaData.Filename.GetPath();
		for ( size_t i = 0; i < g_Paths.GetCount(); i++ )
		{
			if ( sCheck.Find( g_Paths.Item( i ) ) > -1 )
			{
				sRootPath = g_Paths.Item( i );
				break;
			}

			else if ( i == g_Paths.GetCount() - 1 )
				return false;
		}
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
	newfilename.Normalize();
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
		{
			wxCriticalSectionLocker lock( m_csDBAccess );
			sqlite_exec_printf( m_pDB, "update songs set filename =%Q where songid = %d;",
				NULL, NULL, NULL,
				( const char* )ConvToUTF8( song.MetaData.Filename.GetFullPath() ),
				 song.songid );
		}
		return true;
	}
	
	//-----------------------------------------//
	//--- how could we get here? we can't	---//
	//--- so just return false				---//
	//-----------------------------------------//
	return false;
}

bool CMusikLibrary::RetagFile(const CMusikTagger & tagger, CMusikSong* Song )
{
	
	if(!tagger.Retag(Song))
		return false;
	UpdateItem( *Song, true );
	return true;
}

//-----------------------------------//
//--- pre-defined queries to make ---//
//---   life a little bit easier  ---//
//-----------------------------------//
void CMusikLibrary::GetAllYears(wxArrayString & years)
{
	years.Clear();
	Query( wxT("select distinct year from songs order by year;"), years );
	wxArrayString verifiedyears;
	VerifyYearList( years, verifiedyears );
	years = verifiedyears;
	return;
}

void CMusikLibrary::GetAllSongs( CMusikSongArray & aReturn, bool bSorted )
{
	//QuerySongsWhere( wxT(""), aReturn ,bSorted); for some unknown reason linux (suse 8.2 with wxGTK2.5.1 segfaults
	QuerySongsWhere( wxString(), aReturn ,bSorted);
}

void CMusikLibrary::GetAllArtists( wxArrayString & aReturn, bool bSorted  )
{
	if(bSorted)
	{
		if(wxGetApp().Prefs.bSortArtistWithoutPrefix)
			Query( wxT("select distinct artist,UPPER(REMPREFIX(artist)) as UP from songs order by UP;"), aReturn );
		else	
			Query( wxT("select distinct artist,UPPER(artist) as UP from songs order by UP;"), aReturn );
	}
	else
		Query( wxT("select distinct artist from songs;"), aReturn );


}

void CMusikLibrary::GetAllAlbums( wxArrayString & aReturn, bool bSorted  )
{
	if(bSorted)
		Query( wxT("select distinct album,UPPER(album) as UP from songs order by UP;"), aReturn );
	else
		Query( wxT("select distinct album from songs;"), aReturn );
}

void CMusikLibrary::GetAllGenres( wxArrayString & aReturn, bool bSorted )	
{ 
	if(bSorted)
		Query( wxT("select distinct genre,UPPER(genre) as UP from songs order by UP;"), aReturn );					
	else
		Query( wxT("select distinct genre as UP from songs;"), aReturn );					
}

bool CMusikLibrary::SetAutoDjFilter(const wxString & sFilter)
{ 
	wxCriticalSectionLocker lock( m_csDBAccess );
	sqlite_exec( m_pDB,	"DROP VIEW autodj_songs;", NULL, NULL, NULL );
	int res = sqlite_exec_printf( m_pDB,	"CREATE VIEW autodj_songs as select * from songs where %s;"
				   , NULL, NULL, NULL ,( const char* )ConvToUTF8(sFilter));	
	return res == SQLITE_OK;
}
