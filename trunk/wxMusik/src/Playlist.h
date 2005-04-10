#ifndef PLAYLIST_H
#define PLAYLIST_H
#include <map>
#include "MusikId.h"

WX_DECLARE_OBJARRAY( MusikSongId, CMusikSongIdArray );

class CMusikSongArray : public CMusikSongIdArray
{
public:
	CMusikSongArray()
	{
	}
	CMusikSongArray(const wxArrayString & Ids)
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
class PlaylistManager
{
public:
	PlaylistManager()
	{
	}
	~PlaylistManager();

	CMusikSongArray & CreatePlaylist(const wxString & sName);


private:
	typedef std::map<wxString,CMusikSongArray *> tPlaylistMap;

	tPlaylistMap m_mapPlaylists;

};
#endif
#endif
