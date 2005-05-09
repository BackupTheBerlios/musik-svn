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
	}
	MusikSongIdArray(const wxArrayString & Ids)
	{
		Assign(Ids);
	}

	void Assign(const wxArrayString & Ids)
	{
		Clear();
		for(size_t i = 0; i < Ids.GetCount();i++)  
		{
			int id = wxStringToInt(Ids[i]);
			if(id >= 0)
				Add(MusikSongId(id));
		}
	}
	wxLongLong GetTotalFileSize() const;
	int GetTotalPlayingTimeInSeconds() const;

};
#if 1
class PlaylistDataProvider
{
public:
    virtual bool LoadIdArray(const wxString &name,MusikSongIdArray  &idarr)= 0;
    virtual bool QueryIdArray(const wxString & query,MusikSongIdArray  &idarr) = 0;
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
	virtual ~Playlist();

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
        return m_refDataProvider.LoadIdArray(Name(),m_SongIdArray);
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

        m_refDataProvider.QueryIdArray(m_sQuery,m_SongIdArray);          
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

    virtual bool LoadIdArray(const wxString &name,MusikSongIdArray  &idarr);
    virtual bool QueryIdArray(const wxString & query,MusikSongIdArray  &idarr);

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
