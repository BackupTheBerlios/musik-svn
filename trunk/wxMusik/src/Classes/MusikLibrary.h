/*
 *  MusikLibrary.h
 *
 *  Object which controls Library manipulation.
 *
 *  Uses:  id3lib, and oggvorbis
 *  Information about id3lib is available at http://www.id3lib.org
 *  Information about oggvorbis is available at http://www.vorbis.com/
 *  
 *  Copyright (c) 2003 Casey Langen (casey@bak.rr.com)
 *	Contributors: Simon Windmill, Dustin Carter, Gunnar Roth, Wade Brainerd
 *
 *  See the file "license.txt" for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL WARRANTIES.
*/

#ifndef MUSIK_LIBRARY_H
#define MUSIK_LIBRARY_H

//--- sql ---//
#include "sqlite.h"

//--- wx ---//
#include "myprec.h"
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include <wx/dynarray.h>
#include <wx/datetime.h>
#include <wx/regex.h>
#include <wx/filename.h>

#include <map>
#include "MusikUtils.h"
#include "Playlist.h"

class CMusikLibrary : public wxEvtHandler
{
public:
	CMusikLibrary();
	~CMusikLibrary();

    void OnSongDataChange(int songid = -1)
    {
        wxCriticalSectionLocker lock( m_csCacheAccess );
        if(songid == -1)
            m_mapSongCache.clear();
        else
            m_mapSongCache.erase(songid);
    }
	//---------------//
	//--- loading ---//
	//---------------//
	bool Load		();
	void Shutdown	();
	bool AddSongDataFromFile	( const wxString & filename );
	bool UpdateSongDataFromFile	( const wxString & filename );
	void BeginTransaction()
	{ 	wxCriticalSectionLocker lock( m_csDBAccess );
		sqlite_exec_printf( m_pDB, "begin transaction;", NULL, NULL, NULL );
	}
	void EndTransaction()
	{ 		
		wxCriticalSectionLocker lock( m_csDBAccess );
		sqlite_exec_printf( m_pDB, "end transaction;", NULL, NULL, NULL );
	}
	//----------------//
	//--- updating ---//
	//----------------//
    bool UpdateItem				( MusikSongId &songinfoid, bool bDirty );
	bool UpdateItem				( const CMusikSong &newsonginfo, bool bDirty );
	void UpdateItemLastPlayed	( int  songid   );
	void UpdateItemResetDirty	( int  songid  );
	void SetRating				( int songid, int nVal );

	//--------------------//
	//--- writing tags ---//
	//--------------------//
	bool RenameFile			( CMusikSong & song );
	bool RetagFile			( const CMusikTagger & tagger, CMusikSong & song );
	bool ReplaceMask		( wxString *sTarget,const  wxString & sMask, const wxString &sReplaceBy,const  wxString &sDefault = wxT("-Unknown-"),bool bReplaceAll = true  );
	int  ClearDirtyTags		( );
	bool WriteTag			(  MusikSongId & songid, bool ClearAll, bool bUpdateDB = true );
	
	//----------------//
	//--- removing ---//
	//----------------//
	bool CheckAndPurge	( const wxString & sFilename	);
	void RemoveSongDir	( const wxString & sDir			);
	void RemoveSong		( const wxString & sSong		);
	void RemoveSong		( int songid					);
	void RemoveAll		( );
	
	//---------------------//
	//--- getting items ---//
	//---------------------//
	void GetFilelistSongs		( const wxArrayString & aFiles, MusikSongIdArray & aReturn );
	bool QuerySongFromSongid	( int songid, CMusikSong *pSong );
    bool GetSongFromSongid	( int songid, CMusikSong *pSong );

	void SetSortOrderColumn( const PlaylistColumn & Column, bool descending = false);
	double GetSum(const wxString & sField, const MusikSongIdArray &  idarray )const ;

	//--------------------//
	//--- getting info ---//
	//--------------------//
	int	GetSongCount	();
	int	GetSongDirCount ( wxString sDir );

	//------------//
	//--- misc ---//
	//------------//
	bool FileInLibrary		( const wxString & filename, bool fullpath );

	void RecordSongHistory( const MusikSongId & songid ,int playedtime);
	//------------------------//
	//--- generic querying ---//
	//------------------------//
	void Query				( const wxString & query, wxArrayString & aReturn ,bool bClearArray = true);
	void Query				( const wxString & query, wxArrayInt & aReturn ,bool bClearArray = true);
	void QuerySongsWhere	( const wxString & query, MusikSongIdArray & aReturn ,bool bSorted = false,bool bClearArray=true);	  //if bSorted ==true,  use SetSortOrderColumn to set sorting string
	void QuerySongsFrom		( const wxString & queryFrom, MusikSongIdArray & aReturn ,bool bSorted = false);
	void RedoLastQuerySongsWhere( MusikSongIdArray & aReturn ,bool bSorted = false);
	//-----------------------------------//
	//--- pre-defined queries to make ---//
	//---   life a little bit easier  ---//
	//-----------------------------------//
	void GetAllSongs		( MusikSongIdArray & aReturn	,bool bSorted = true);
    void GetAllOfColumn		( const PlaylistColumn & Column,wxArrayString & aReturn ,bool bSorted = true);
	void GetInfo			( const wxArrayString & aInfo, const PlaylistColumn & ColumnIn, const PlaylistColumn & ColumnOut , wxArrayString & aReturn, bool bSorted = true);
	void GetSongs			( const wxArrayString & aInfo, const PlaylistColumn & Column, MusikSongIdArray & aReturn );
	int QueryCount			(const char * szQuery );

	//misc
	bool SetAutoDjFilter(const wxString & sFilter);
    bool SetCacheSize(int size);
private:


	void CheckVersion();

	sqlite		  *m_pDB;
	void CreateDB();

    mutable wxCriticalSection m_csCacheAccess;
    typedef std::map<int,CMusikSong>  tSongCacheMap;
    tSongCacheMap m_mapSongCache;

	wxString  m_sSortAllSongsQuery;
	wxString m_lastQueryWhere;
	int m_nCachedSongCount;

	mutable wxCriticalSection m_csDBAccess; // to lock all accesses to m_pDB. 
									// used instead of wxMutex, because this is faster on windows. on linux 
									// a wxMutex is used automatically instead
	void CreateDBFuncs();
	static void remprefixFunc(sqlite_func *context, int argc, const char **argv);
	static void cnvISO8859_1ToUTF8Func(sqlite_func *context, int argc, const char **argv);
	static void wxjuliandayFunc(sqlite_func *context, int argc, const char **argv);
	static void cnvMusikOldDTFormatToJuliandayFunc(sqlite_func *context, int argc, const char **argv);
	static void fuzzycmpFunc(sqlite_func *context, int argc, const char **argv);				
	inline static void _AssignSongTableColumnDataToSong(CMusikSong * pSong, const char **coldata)
	{
		pSong->songid		= StringToInt		( coldata[0] );
		pSong->MetaData.Filename		= ConvFromUTF8		( coldata[1] );
		pSong->MetaData.Title			=					  coldata[2];
		pSong->MetaData.nTracknum		= StringToInt		( coldata[3] );
		pSong->MetaData.Artist			=					  coldata[4];
		pSong->MetaData.Album			=					  coldata[5] ;
		pSong->MetaData.Genre			=					  coldata[6] ;
		pSong->MetaData.nDuration_ms	= StringToInt		( coldata[7] );
		pSong->MetaData.eFormat			= (EMUSIK_FORMAT_TYPE) 
										  StringToInt		( coldata[8] );
		pSong->MetaData.bVBR			= StringToInt		( coldata[9] ) ? true: false;
		pSong->MetaData.Year			=					  coldata[10];
		pSong->Rating					= StringToInt		( coldata[11] );
		pSong->MetaData.nBitrate		= StringToInt		( coldata[12] );
		pSong->LastPlayed				= CharStringToDouble( coldata[13] );
		pSong->MetaData.Notes			=					  coldata[14];
		pSong->TimesPlayed				= StringToInt		( coldata[15] );	
		pSong->TimeAdded				= CharStringToDouble( coldata[16] );
		pSong->MetaData.nFilesize		= StringToInt		( coldata[17] );
	}
	static int sqlite_callbackAddToIntArray(void *args, int numCols, char **results, char ** columnNames);
	static int sqlite_callbackAddToStringArray(void *args, int numCols, char **results, char ** columnNames);
	static int sqlite_callbackAddToSongIdArray(void *args, int numCols, char **results, char ** columnNames);
	static int sqlite_callbackAddToSongIdMap(void *args, int numCols, char **results, char ** columnNames);
};
            
#endif
