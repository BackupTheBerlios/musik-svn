//--- For compilers that support precompilation, includes "wx/wx.h". ---//
#include "wx/wxprec.h"
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif 
#include "CMP3Info.h"
#include "id3/misc_support.h"
#include "id3/readers.h"

CMP3Info::CMP3Info()
{

}
bool CMP3Info::ReadMetaData(CSongMetaData & MetaData) const
{
	//--- load and link mp3 ---//
#ifdef __WXMSW__
	FILE * f = wxFopen(MetaData.Filename.GetFullPath(), wxT("rb"));
	if(f == NULL)
		return false;
	ifstream stream(f);
#else
	// ifstream::ifstream(FILE *f) seems to be only available on windows
	ifstream stream;
	stream.open(ConvFn2A(MetaData.Filename.GetFullPath()),ios::in|ios::binary);
	if(!stream)
		return false;
#endif
	ID3_IFStreamReader reader(stream);		
	ID3_Tag		id3Tag;
	id3Tag.Link( reader, (flags_t)ID3TT_ALL );

	stream.close();
#ifdef __WXMSW__
	fclose(f);
#endif
	MetaData.Artist.Attach	( ID3_GetArtist	( &id3Tag ));
	MetaData.Title.Attach	( ID3_GetTitle	( &id3Tag ));
	MetaData.Album.Attach	( ID3_GetAlbum	( &id3Tag ));
	MetaData.Year.Attach	( ID3_GetYear	( &id3Tag ));
	MetaData.Notes.Attach	( ID3_GetComment( &id3Tag ));
	MetaData.Genre = ID3_V1GENRE2DESCRIPTION(ID3_GetGenreNum( &id3Tag ));
	if(MetaData.Genre.IsEmpty())
		MetaData.Genre.Attach	( ID3_GetGenre	( &id3Tag ));

	MetaData.nTracknum =ID3_GetTrackNum( &id3Tag );

	MetaData.Artist = ConvFromISO8859_1ToUTF8(MetaData.Artist);
	MetaData.Title = ConvFromISO8859_1ToUTF8(MetaData.Title);
	MetaData.Album = ConvFromISO8859_1ToUTF8(MetaData.Album);
	MetaData.Notes = ConvFromISO8859_1ToUTF8(MetaData.Notes);
	MetaData.Genre = ConvFromISO8859_1ToUTF8(MetaData.Genre);
	// bitrate
	const Mp3_Headerinfo* mp3header = id3Tag.GetMp3HeaderInfo();
	if ( mp3header ) 
	{
		MetaData.nDuration_ms	= mp3header->time * 1000;
		MetaData.bVBR			= mp3header->vbr_bitrate ? true : false;
		MetaData.nBitrate		= mp3header->bitrate / 1000;
	}
	MetaData.nFilesize	  =  id3Tag.GetFileSize();
	return true;

}

bool  CMP3Info::WriteMetaData(const CSongMetaData & MetaData,bool bClearAll)
{
	ID3_Tag	id3Tag;
	id3Tag.Link( ( const char* )ConvFn2A( MetaData.Filename.GetFullPath () ) , (flags_t)ID3TT_ALL );

	//--- iterate through and delete ALL TAG INFO ---//
	if ( bClearAll )
	{
		ID3_Tag::Iterator* iter = id3Tag.CreateIterator();
		ID3_Frame* frame = NULL;
		while (NULL != (frame = iter->GetNext()))
		{
			frame = id3Tag.RemoveFrame(frame);
			delete frame;
		}
	}

	//--- clear only fields of interest ---//
	else if ( !bClearAll )
	{
		ID3_RemoveTitles	( &id3Tag ); 
		ID3_RemoveArtists	( &id3Tag );
		ID3_RemoveAlbums	( &id3Tag );
		ID3_RemoveTracks	( &id3Tag );
		ID3_RemoveYears		( &id3Tag );
		ID3_RemoveGenres	( &id3Tag );
		ID3_RemoveComments	( &id3Tag );

	}

	//--- tag ---//
	ID3_AddTitle	( &id3Tag,  ConvFromUTF8ToISO8859_1( MetaData.Title ),	true ); 
	ID3_AddArtist	( &id3Tag, ConvFromUTF8ToISO8859_1( MetaData.Artist ),	true );
	ID3_AddAlbum	( &id3Tag, ConvFromUTF8ToISO8859_1( MetaData.Album ),	true );
	ID3_AddYear		( &id3Tag, ConvFromUTF8ToISO8859_1( MetaData.Year ), 	true );
	ID3_AddComment	( &id3Tag, ConvFromUTF8ToISO8859_1( MetaData.Notes ), 	true );
	ID3_AddTrack	( &id3Tag, MetaData.nTracknum,				true );

	int genreid = GetGenreID( MetaData.Genre );
	if( genreid == -1 )
		ID3_AddGenre( &id3Tag, ConvFromUTF8ToISO8859_1( MetaData.Genre ),	true ); // write ID3V2 string genre tag
	else
		ID3_AddGenre( &id3Tag, genreid,	true );											// write ID3V1 integer genre id

	//--- write to file ---//
	return (id3Tag.Update() != ID3TT_NONE);

}
