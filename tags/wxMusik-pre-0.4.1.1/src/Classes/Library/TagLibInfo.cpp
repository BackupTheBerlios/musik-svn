#include "wx/wxprec.h"
#include "wx/mstream.h"
#include <string.h>
#include "TagLibInfo.h"
#include "MusikUtils.h"
#include <fileref.h>
#include <tag.h>
#include <tfile.h>
#include <mpeg/mpegproperties.h>
#include <mpeg/mpegfile.h>
#include <mpeg/id3v2/id3v2tag.h>
#include <mpeg/id3v2/frames/attachedpictureframe.h>

CTagLibInfo::CTagLibInfo(void)
{
}
bool CTagLibInfo::ReadMetaData(CSongMetaData & MetaData) const
{
#ifdef __WXMSW__
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

bool CTagLibInfo::LoadImage(const wxString & sFilename, wxImage & img)
{
#ifdef __WXMSW__
    TagLib::Filename fn(sFilename.c_str());
#else
    TagLib::Filename fn((const char*)ConvFn2A(sFilename));
#endif
    TagLib::MPEG::File *pMpegfile =new TagLib::MPEG::File( fn ,false);
	TagLib::FileRef f(pMpegfile);  // to take care of deletion.
    
    if(pMpegfile->isValid() == false || pMpegfile->ID3v2Tag() == NULL)
        return false;

    TagLib::ID3v2::Tag *tag = pMpegfile->ID3v2Tag();
    const TagLib::ID3v2::FrameList & ApicFrameList = tag->frameList("APIC");
    if(ApicFrameList.isEmpty() 
        || dynamic_cast<const TagLib::ID3v2::AttachedPictureFrame *>(ApicFrameList.front()) == NULL 
      )
        return false;
    const TagLib::ID3v2::AttachedPictureFrame *ApicFrame = static_cast<const TagLib::ID3v2::AttachedPictureFrame *>(ApicFrameList.front());
    wxString sMimeType(ConvA2W(ApicFrame->mimeType().toCString()));
    wxMemoryInputStream vMstream(ApicFrame->picture().data(), ApicFrame->picture().size());
    return img.LoadFile(vMstream,sMimeType);
}