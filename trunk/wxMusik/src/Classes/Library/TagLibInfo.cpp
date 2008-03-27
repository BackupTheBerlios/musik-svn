#include "myprec.h"
#include "wx/mstream.h"
#include <string.h>
#include "TagLibInfo.h"
#include "mp3tech.h"
#include "MusikUtils.h"
#include <fileref.h>
#include <tag.h>
#include <tfile.h>
#include <mpeg/mpegproperties.h>
#include <mpeg/mpegfile.h>
#include <mpeg/id3v2/id3v2tag.h>
#include <mpeg/id3v2/frames/attachedpictureframe.h>

#ifdef __VISUALC__
#pragma comment(lib,"TagLib")
#endif

CTagLibInfo::CTagLibInfo(void)
{
}
bool CTagLibInfo::ReadMetaData(CSongMetaData & MetaData) const
{
    bool readAudioProperties = true;
    TagLib::AudioProperties::ReadStyle audioPropertiesStyle = TagLib::AudioProperties::Average;
    if(MetaData.eFormat == MUSIK_FORMAT_MP3)
        readAudioProperties = false; // taglib audio properties reading is quity buggy for vbr files
                                     //( without xing header) and some files with garbage at the start
                                         // we use mp3tech.c code from mp3info 0.8.4 therefore
    { // fileref scope
 	    TagLib::FileRef f((const char *)ConvFn2A(MetaData.Filename.GetFullPath()) ,readAudioProperties,audioPropertiesStyle);
	    if(f.isNull())
		    return false;
	    TagLib::Tag *tag = f.tag();
	    if(tag) 
	    {		
		    MetaData.Title = tag->title().toCString(true);
		    MetaData.Artist  = tag->artist().toCString(true);
		    MetaData.Album = tag->album().toCString(true);
            if(tag->year() > 0)
            {
                char szYear[20];
                sprintf(szYear,"%d",tag->year());
                MetaData.Year = szYear; 
            }
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
    }
    if(MetaData.eFormat == MUSIK_FORMAT_MP3)
    {
        mp3info infomp3;
        memset(&infomp3,0,sizeof(infomp3));
        infomp3.file = wxFopen(MetaData.Filename.GetFullPath(),wxT("rb"));
        if(infomp3.file == NULL)
            return false;
		get_mp3_info(&infomp3,wxGetApp().Prefs.bScanMP3VBRQuick ? SCAN_QUICK : SCAN_NORMAL,0);
        MetaData.nFilesize = infomp3.datasize;
        MetaData.nDuration_ms = infomp3.seconds * 1000;
        MetaData.nBitrate = (int)infomp3.vbr_average;
        if( infomp3.vbr_average < 100 && infomp3.seconds > 300)
        {
            // vbr_average seems to low and song seems to be longer than 5 minutes ,make full scan.
            get_mp3_info(&infomp3,SCAN_NORMAL,1);
            MetaData.nFilesize = infomp3.datasize;
            MetaData.nDuration_ms = infomp3.seconds * 1000;
            MetaData.nBitrate = (int)infomp3.vbr_average;
        }
		MetaData.bVBR = infomp3.vbr > 0 ? true:false;
        fclose(infomp3.file);
    }
	return true;
}

bool  CTagLibInfo::WriteMetaData(const CSongMetaData & MetaData,bool bClearAll)
{
 
    TagLib::FileRef f((const char *)ConvFn2A(MetaData.Filename.GetFullPath()));
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
// on msw we can use the unicode version of taglib TagLib::MPEG::File.
    TagLib::MPEG::File *pMpegfile =new TagLib::MPEG::File(sFilename.c_str(),false);
#else
    TagLib::MPEG::File *pMpegfile =new TagLib::MPEG::File(ConvFn2A(sFilename),false);
#endif    
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
    const TagLib::ByteVector pic = ApicFrame->picture();
	if(pic.size() == 0)
		return false; 
    wxMemoryInputStream vMstream(pic.data(), pic.size());
    return img.LoadFile(vMstream,sMimeType);
}
