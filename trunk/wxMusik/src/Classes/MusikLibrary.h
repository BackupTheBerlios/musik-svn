/*
 *  MusikLibrary.h
 *
 *  Object which controls Library manipulation.
 *   
 *  Copyright (c) 2003 Casey Langen (casey@bak.rr.com)
 *	Contributors: Simon Windmill, Dustin Carter, Gunnar Roth, Wade Brainerd
 *
 *  See the file "license.txt" for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL WARRANTIES.
*/

#ifndef MUSIK_LIBRARY_H
#define MUSIK_LIBRARY_H

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
#include <vector>
#include "MusikUtils.h"
#include "Playlist.h"

class MusikDb;

class CMusikLibrary : public wxEvtHandler
{
public:

	CMusikLibrary();
	~CMusikLibrary();

    CMusikLibrary * CreateSlave();
    void OnSongDataChange(int songid = -1);
	//---------------//
	//--- loading ---//
	//---------------//
	bool Load		();
	void Shutdown	();
	bool AddSongDataFromFile	( const wxString & filename );
	bool UpdateSongDataFromFile	( const wxString & filename ,bool bForce = false);
	void BeginTransaction();
	void EndTransaction();
    void SignalSlaveTransactionEnd();
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
    class BusyHandler;
    std::auto_ptr<BusyHandler> m_pBusyHandler;
    CMusikLibrary *m_pMasterLibrary;

    void ConvertFromDB2();
	void CheckVersion2();
    void CheckVersion();
    void InternalEndTransaction();

    std::auto_ptr<MusikDb> m_pDB;
    bool m_bTansactionInProgress;
    std::vector<int> m_arrTransactionIdsChanged;
    mutable wxCriticalSection m_csCacheAccess;
    typedef std::map<int,CMusikSong>  tSongCacheMap;
    tSongCacheMap m_mapSongCache;

	wxString  m_sSortAllSongsQuery;
	wxString m_lastQueryWhere;
	int m_nCachedSongCount;

    inline static int db_callbackAssignSongTableColumnDataToSong(void *args, int WXUNUSED(numCols), char **results, char ** WXUNUSED(columnNames))
  	{
        CMusikSong * pSong = (CMusikSong *)args;
		pSong->songid		= StringToInt		( results[0] );
		pSong->MetaData.Filename		= ConvFromUTF8		( results[1] );
		pSong->MetaData.Title			=					  results[2];
		pSong->MetaData.nTracknum		= StringToInt		( results[3] );
		pSong->MetaData.Artist			=					  results[4];
		pSong->MetaData.Album			=					  results[5] ;
		pSong->MetaData.Genre			=					  results[6] ;
		pSong->MetaData.nDuration_ms	= StringToInt		( results[7] );
		pSong->MetaData.eFormat			= (EMUSIK_FORMAT_TYPE) 
										  StringToInt		( results[8] );
		pSong->MetaData.bVBR			= StringToInt		( results[9] ) ? true: false;
		pSong->MetaData.Year			=					  results[10];
		pSong->Rating					= StringToInt		( results[11] );
		pSong->MetaData.nBitrate		= StringToInt		( results[12] );
		pSong->LastPlayed				= CharStringToDouble( results[13] );
		pSong->MetaData.Notes			=					  results[14];
		pSong->TimesPlayed				= StringToInt		( results[15] );	
		pSong->TimeAdded				= CharStringToDouble( results[16] );
		pSong->MetaData.nFilesize		= StringToInt		( results[17] );
        return 0;
	}
	static int db_callbackAddToIntArray(void *args, int numCols, char **results, char ** columnNames);
	static int db_callbackAddToStringArray(void *args, int numCols, char **results, char ** columnNames);
	static int db_callbackAddToSongIdArray(void *args, int numCols, char **results, char ** columnNames);
	static int db_callbackAddToSongIdMap(void *args, int numCols, char **results, char ** columnNames);
};
            
#endif
