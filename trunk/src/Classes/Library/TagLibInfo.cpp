#include "wx/wxprec.h"
#include <string.h>
#include "TagLibInfo.h"
#include "MusikUtils.h"
#include <fileref.h>
#include <tag.h>
#include <tfile.h>
#include <mpeg/mpegproperties.h>
CTagLibInfo::CTagLibInfo(void)
{
}
bool CTagLibInfo::ReadMetaData(CSongMetaData & MetaData) const
{
#ifdef __WXMSW
	TagLib::FileRef f(  MetaData.Filename.GetFullPath().c_str());
#else
	TagLib::FileRef f(TagLib::Filename((const char*)ConvFn2A(MetaData.Filename.GetFullPath())) );
#endif
	if(f.isNull())
		return false;
	TagLib::Tag *tag = f.tag();
	if(tag) 
	{		
		MetaData.Title = tag->title().toCString(true);
		MetaData.Artist  = tag->artist().toCString(true);
		MetaData.Album = tag->album().toCString(true);
		char szYear[20];
		sprintf(szYear,"%d",tag->year());
		MetaData.Year = szYear; 
		MetaData.Notes =  tag->comment().toCString(true);
		MetaData.nTracknum = tag->track();
		MetaData.Genre = tag->genre().toCString(true);
	}


	TagLib::AudioProperties *properties = f.audioProperties();
	if(properties) 
	{
		if(MetaData.eFormat == MUSIK_FORMAT_MP3)
		{
			MetaData.bVBR = static_cast<TagLib::MPEG::Properties*>(properties)->isVbr();
		}
		MetaData.nFilesize = f.file()->length();
		MetaData.nDuration_ms = properties->length() * 1000;
		MetaData.nBitrate = properties->bitrate();
	}
	return true;
}

bool  CTagLibInfo::WriteMetaData(const CSongMetaData & MetaData,bool bClearAll)
{
#ifdef __WXMSW__
	TagLib::FileRef f( TagLib::Filename(MetaData.Filename.GetFullPath().c_str()));
#else
	TagLib::FileRef f(TagLib::Filename((const char*)ConvFn2A(MetaData.Filename.GetFullPath())) );
#endif
	if(f.isNull())
		return false;
	
	TagLib::Tag *tag = f.tag();
	
	tag->setTitle(TagLib::String(MetaData.Title,TagLib::String::UTF8));
	tag->setArtist(TagLib::String(MetaData.Artist,TagLib::String::UTF8));
	tag->setAlbum(TagLib::String(MetaData.Album,TagLib::String::UTF8));
	tag->setComment(TagLib::String(MetaData.Notes,TagLib::String::UTF8));
	tag->setGenre(TagLib::String(MetaData.Genre,TagLib::String::UTF8));
	tag->setYear(atoi(MetaData.Year));
	tag->setTrack(MetaData.nTracknum);
	return f.save();
}
