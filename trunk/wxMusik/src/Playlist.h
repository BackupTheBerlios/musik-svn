/*
*  Playlist.h
*  
*  Copyright (c) 2005 Gunnar Roth (gunnar.roth@gmx.de)
*	
*
*  See the file "license.txt" for information on usage and redistribution
*  of this file, and for a DISCLAIMER OF ALL WARRANTIES.
*/
#ifndef PLAYLIST_H
#define PLAYLIST_H
#include <map>
#include "MusikId.h"
#include "PlaylistColumn.h"

WX_DECLARE_OBJARRAY( MusikSongId, MusikSongIdArrayBase );

class MusikSongIdArray : public MusikSongIdArrayBase
{
public:
	MusikSongIdArray()
	{
        m_nCurIndex = 0;
	}
	MusikSongIdArray(const wxArrayString & Ids)
	{
        m_nCurIndex = 0;
		Assign(Ids);
	}

	void Assign(const wxArrayString & Ids)
	{
		Clear();
        Alloc(Ids.GetCount());
		for(size_t i = 0; i < Ids.GetCount();i++)  
		{
			int id = wxStringToInt(Ids[i]);
			if(id >= 0)
				Add(MusikSongId(id));
		}
	}
    int MoveEntrys(int nMoveTo ,const wxArrayInt &arrToMove);
    void CurrentIndex(size_t nCurIndex)
    {
        m_nCurIndex = GetCount() ? wxMin(wxMax(0,nCurIndex),GetCount() - 1) : 0;
    }
    size_t CurrentIndex() const
    {
        return GetCount() ? m_nCurIndex : 0;
    }
    void IncrCurrentIndex() 
    {
        if(m_nCurIndex < GetCount() - 1)
            m_nCurIndex++;
    }
    void DecrCurrentIndex() 
    {
        if(m_nCurIndex > 0)
            m_nCurIndex--;
    }

    void Insert(const MusikSongId& lItem,  size_t uiIndex, size_t nInsert = 1)
    {
        if((int)uiIndex <= m_nCurIndex)
            m_nCurIndex += nInsert;
        MusikSongIdArrayBase::Insert(lItem,uiIndex,nInsert);

    }
    void Insert(const MusikSongId * pItem, size_t uiIndex)
    {
        if((int)uiIndex <= m_nCurIndex) 
            m_nCurIndex ++;
         MusikSongIdArrayBase::Insert(pItem,uiIndex);
    }
    void Empty() { m_nCurIndex = 0;  MusikSongIdArrayBase::Empty(); }
    void Clear() { m_nCurIndex = 0;  MusikSongIdArrayBase::Clear(); }
    MusikSongId* Detach(size_t uiIndex)
    {
        if((int)uiIndex < m_nCurIndex) 
            m_nCurIndex --;
        return MusikSongIdArrayBase::Detach(uiIndex);
    }
    void RemoveAt(size_t uiIndex, size_t nRemove = 1)
    {
        if((int)uiIndex < m_nCurIndex)
            if((int)(uiIndex + nRemove - 1) < m_nCurIndex)
                m_nCurIndex -= nRemove;
            else
                m_nCurIndex = uiIndex;
        return MusikSongIdArrayBase::RemoveAt(uiIndex,nRemove);
    }
	wxLongLong GetTotalFileSize() const;
	int GetTotalPlayingTimeInSeconds() const;
    wxString AsCommaSeparatedString()
    {
        wxString sList;
        sList.Alloc(GetCount() * 7);
        for(size_t i = 0 ; i < GetCount();i++)
        {
            sList << Item(i);
            if(i != GetCount() - 1)
                sList << wxT(",");
        }
        return sList;
    }
private:
    size_t m_nCurIndex;
};
#if 1
class Playlist;

class PlaylistDataProvider
{
public:
    virtual bool RetrieveIdArray(Playlist & pl,MusikSongIdArray  &idarr)= 0;
    virtual bool Save(Playlist & pl) const  = 0;
};

class Playlist
{
public:
	Playlist(PlaylistDataProvider & DataProvider,const wxString & name)
        :m_refDataProvider(DataProvider)
        ,m_sName(name)
    {

    }
	const wxString & Name()
	{
		return m_sName;
	}

	virtual const MusikSongIdArray & SongIdArray() const = 0;
    virtual ~Playlist(){}

protected:
    PlaylistDataProvider & m_refDataProvider;
private:
	wxString m_sName;
};

class StaticPlaylist : public Playlist 
{
public:
    StaticPlaylist(PlaylistDataProvider & DataProvider,const wxString & name)
        :Playlist(DataProvider,name)
    {
        Init();
    }
    const MusikSongIdArray & SongIdArray() const
    {
        return m_SongIdArray;
    }
    void Add( MusikSongIdArray & ids ); 
    void Insert( MusikSongIdArray & ids ,size_t nInsertBefore); 
    void Move(size_t nIndexMoveTo ,const wxArrayInt &arrIndexToMove );
    void Remove(size_t nRemove);
protected:
    void Init()
    {
        Load();
    }

    bool Load()
    {
        return m_refDataProvider.RetrieveIdArray(*this,m_SongIdArray);
    }
    MusikSongIdArray m_SongIdArray;
};

class DynamicPlaylist : public Playlist 
{
public:
    DynamicPlaylist(PlaylistDataProvider & DataProvider,const wxString & name,const wxString & query)
        :Playlist(DataProvider,name)
        ,m_sQuery(query)
        ,m_SortColumn(PlaylistColumn::INVALID)
        ,m_bSortAscending(true)
    {
        
    }
    wxString & Query() 
    {
        return m_sQuery;
    }
    const wxString & Query() const
    {
        return m_sQuery;
    }

    bool SetSortColumn(PlaylistColumn::eId id,bool bSortAscending)
    {
        m_SortColumn = id;
        m_bSortAscending = bSortAscending;
        return true;
    }
    bool GetSortColumn(PlaylistColumn::eId &id,bool & bSortAscending)
    {
        id = m_SortColumn;
        bSortAscending = m_bSortAscending;
        return true;
    }
    const MusikSongIdArray & SongIdArray()
    {
        Realize();
        return m_SongIdArray;
    }

protected:
    void Realize()
    {

        m_refDataProvider.RetrieveIdArray(*this,m_SongIdArray);          
    }

 
    wxString m_sQuery;
    MusikSongIdArray m_SongIdArray;
    PlaylistColumn::eId m_SortColumn;
    bool m_bSortAscending;
};
class CMusikLibrary;

class LibraryPlaylistDataProvider : public PlaylistDataProvider
{
public:
    LibraryPlaylistDataProvider(CMusikLibrary &Lib)
        :m_refLib(Lib)
    {
    }

    virtual bool RetrieveIdArray(Playlist & pl,MusikSongIdArray  &idarr);
    virtual bool Save(Playlist & pl) const;

protected:
    CMusikLibrary &m_refLib;

};

class PlaylistManager
{
public:
	PlaylistManager(CMusikLibrary & lib)
        :m_DataProvider(lib)
	{
	}
	~PlaylistManager();

    Playlist & GetPlaylist(const wxString &name);


protected:
private:
	typedef std::map< wxString,Playlist * > tPlaylistMap;

	tPlaylistMap m_mapPlaylists;

    LibraryPlaylistDataProvider m_DataProvider;
friend class StaticPlaylist;
};
#endif
#endif
