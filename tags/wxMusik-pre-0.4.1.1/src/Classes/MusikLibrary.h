/*
 *  MusikLibrary.h
 *
 *  Object which controls database manipulation.
 *
 *  Uses: SQLite, id3lib, and oggvorbis
 *  Information about SQLite - An Embeddable SQL Database Engine is available at http://www.hwaci.com/sw/sqlite/
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
#include "wx/wxprec.h"
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include <wx/dynarray.h>
#include <wx/datetime.h>
#include <wx/regex.h>
#include <wx/filename.h>

#include "../MusikUtils.h"

class CMusikLibrary : public wxEvtHandler
{
public:
	CMusikLibrary();
	~CMusikLibrary();

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
	void UpdateItem				( CMusikSong &newsonginfo, bool bDirty );
	void UpdateItemLastPlayed	( const CMusikSong & song  );
	void UpdateItemResetDirty	( const CMusikSong & song );
	void SetRating				( int songid, int nVal );

	//--------------------//
	//--- writing tags ---//
	//--------------------//
	bool RenameFile			( CMusikSong & song );
	bool RetagFile			( const CMusikTagger & tagger, CMusikSong* song );
	bool ReplaceMask		( wxString *sTarget,const  wxString & sMask, const wxString &sReplaceBy,const  wxString &sDefault = wxT("-Unknown-"),bool bReplaceAll = true  );
	int  ClearDirtyTags		( );
	bool WriteTag			(  CMusikSong & song, bool ClearAll, bool bUpdateDB = true );
	
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
	void GetFilelistSongs		( const wxArrayString & aFiles, CMusikSongArray & aReturn );
	bool GetSongFromSongid	( int songid, CMusikSong *pSong );

	void SetSortOrderField( int nField, bool descending = false);
	double GetTotalPlaylistSize();

	//--------------------//
	//--- getting info ---//
	//--------------------//
	int	GetTimesPlayed	( const CMusikSong & song );
	int	GetSongCount	();
	int	GetSongDirCount ( wxString sDir );

	//------------//
	//--- misc ---//
	//------------//
	bool FileInLibrary		( const wxString & filename, bool fullpath );

	void RecordSongHistory( const CMusikSong & song ,int playedtime);
	//------------------------//
	//--- generic querying ---//
	//------------------------//
	void Query				( const wxString & query, wxArrayString & aReturn ,bool bClearArray = true);
	void Query				( const wxString & query, wxArrayInt & aReturn ,bool bClearArray = true);
	void QuerySongsWhere	( const wxString & query, CMusikSongArray & aReturn ,bool bSorted = false,bool bClearArray=true);	  //if bSorted ==true,  use SetSortOrderField to set sorting string
	void QuerySongsFrom		( const wxString & queryFrom, CMusikSongArray & aReturn ,bool bSorted = false);
	void RedoLastQuerySongsWhere( CMusikSongArray & aReturn ,bool bSorted = false);
	//-----------------------------------//
	//--- pre-defined queries to make ---//
	//---   life a little bit easier  ---//
	//-----------------------------------//
	void GetAllSongs		( CMusikSongArray & aReturn	,bool bSorted = true);
	void GetAllArtists		( wxArrayString & aReturn ,bool bSorted = true);
	void GetAllAlbums		( wxArrayString & aReturn ,bool bSorted = true);
	void GetAllGenres		( wxArrayString & aReturn ,bool bSorted = true);
	void GetAllYears		( wxArrayString & aReturn );
	void GetInfo			( const wxArrayString & aInfo, int nInType, int nOutType, wxArrayString & aReturn, bool bSorted = true);
	void GetSongs			( const wxArrayString & aInfo, int nInType, CMusikSongArray & aReturn );
	int QueryCount			(const char * szQuery );

	//misc
	bool SetAutoDjFilter(const wxString & sFilter);

private:

	void CheckVersion();

	sqlite		  *m_pDB;
	void CreateDB();

	wxString  m_sSortAllSongsQuery;
	wxString m_lastQueryWhere;
	int m_nCachedSongCount;

	void VerifyYearList ( const wxArrayString & aList,wxArrayString & aVerifiedList );

	wxCriticalSection m_csDBAccess; // to lock all accesses to m_pDB. 
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
	static int sqlite_callbackAddToSongArray(void *args, int numCols, char **results, char ** columnNames);
	static int sqlite_callbackAddToSongMap(void *args, int numCols, char **results, char ** columnNames);
};

#endif
