#ifndef PLAYLIST_H
#define PLAYLIST_H
#include <map>
#include "MusikId.h"

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
#if 0
class Playlist
{
public:
	Playlist(const wxString & name);
	const wxString & Name()
	{
		return m_sName;
	}
	const MusikSongIdArray & SongIdArray()
	{
		return m_SongIdArray;
	}
	virtual ~Playlist();

protected:
private:
	wxString m_sName;
q	MusikSongIdArray m_SongIdArray;
};

class PlaylistManager
{
public:
	PlaylistManager()
	{
	}
	~PlaylistManager();

	MusikSongIdArray & CreatePlaylist(const wxString & sName);

	
private:
	typedef std::map<wxString,MusikSongIdArray *> tPlaylistMap;

	tPlaylistMap m_mapPlaylists;

};
#endif
#endif
