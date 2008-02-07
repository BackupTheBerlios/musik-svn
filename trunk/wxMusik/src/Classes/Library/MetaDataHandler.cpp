/*
*  MetaDataHandler.cpp
*
*  Object which handles metadata read and write.
*
*  Uses: id3lib, and ogg/vorbis
*  Information about id3lib is available at http://www.id3lib.org
*  Information about ogg/vorbis is available at http://www.vorbis.com/
*  
*  Copyright (c) 2004 Gunnar Roth (gunnar.roth@gmx.de)
*	Contributors: Simon Windmill, Dustin Carter, Casey Langen, Wade Brainerd
*
*  See the file "license.txt" for information on usage and redistribution
*  of this file, and for a DISCLAIMER OF ALL WARRANTIES.
*/
// For compilers that support precompilation, includes "wx/wx.h".
#include "myprec.h"

#include <exception>
//--- mp3 / ogg helpers ---//
#include "MusikUtils.h"
#include "3rd-Party/TagHelper/idtag.h"
#include "MetaDataHandler.h"

#ifndef MUSIKENGINE_NO_APE_SUPPORT
#include "APEInfo.h"
static CMyAPEInfo apeinfo;
#endif

#define USE_TAGLIB

#ifdef USE_TAGLIB
#include "TagLibInfo.h"
static CTagLibInfo taglibinfo;
#ifndef MUSIKENGINE_NO_FLAC_SUPPORT
#include "FlacInfo.h"
static CFlacInfo flacinfo;
#endif
#else
#include "CMP3Info.h"
static CMP3Info mp3info;
#include "COggInfo.h"
static COggInfo ogginfo;
#ifndef MUSIKENGINE_NO_MPC_SUPPORT
#include "MPCInfo.h"
static CMPCInfo mpcinfo;
#endif
#ifndef MUSIKENGINE_NO_FLAC_SUPPORT
#include "FlacInfo.h"
static CFlacInfo flacinfo;
#endif
#endif
#if defined(USE_FMOD3) || defined (USE_FMODEX)
#include "CFMODInfo.h"
static CFMODInfo fmodinfo;
#endif
class CDummyInfo : public CInfoRead
{
public:

	bool ReadMetaData(CSongMetaData & MetaData) const
	{
		FILE *fp = wxFopen( MetaData.Filename.GetFullPath(), wxT("rb") );
		if (!fp) 
			return false;
		CSimpleTagReader tr;
		CSimpleTagReader::CFile trf(fp,true);
		tr.ReadTags(trf,ConvToUTF8(MetaData.Filename.GetFullPath()));
        if(MetaData.Album.IsEmpty())
            MetaData.Album =  tr.TagValue(SIMPLETAG_FIELD_ALBUM);
        if(MetaData.Title.IsEmpty())    
            MetaData.Title =  tr.TagValue(SIMPLETAG_FIELD_TITLE);
        if(MetaData.Notes.IsEmpty())    
            MetaData.Notes =  tr.TagValue(SIMPLETAG_FIELD_NOTES);
        if(MetaData.Artist.IsEmpty())    
            MetaData.Artist =  tr.TagValue(SIMPLETAG_FIELD_ARTIST);
        if(MetaData.Year.IsEmpty())    
            MetaData.Year =  tr.TagValue(SIMPLETAG_FIELD_YEAR);
        if(MetaData.Genre.IsEmpty())    
            MetaData.Genre =  tr.TagValue(SIMPLETAG_FIELD_GENRE);
        if(MetaData.nTracknum == 0)
        {    
            char* track = tr.TagValue(SIMPLETAG_FIELD_TRACK);
            MetaData.nTracknum = track ? atoi(track) : 0;
        }    
		return true;
	}
protected:

private:
};
static CDummyInfo dummyinfo;

static const tSongClass valid_SongClasses[] = 
{
#ifdef USE_TAGLIB
	{wxT("mp3"),wxTRANSLATE("MPEG Layer 3 Audio File"),MUSIK_FORMAT_MP3,&taglibinfo,&taglibinfo}
	,{wxT("ogg"),wxTRANSLATE("OGG Vorbis Audio File"),MUSIK_FORMAT_OGG,&taglibinfo,&taglibinfo}
#ifndef MUSIKENGINE_NO_MPC_SUPPORT
	,{wxT("mpc"),wxTRANSLATE("Musepack Audio File"),MUSIK_FORMAT_MPC,&taglibinfo,&taglibinfo}
#endif
#ifndef MUSIKENGINE_NO_FLAC_SUPPORT
	,{wxT("flac"),wxTRANSLATE("Flac Lossless Audio File"),MUSIK_FORMAT_FLAC,&flacinfo,&flacinfo}
#endif
#ifndef MUSIKENGINE_NO_FAAD2_SUPPORT
	,{wxT("aac"),wxTRANSLATE("AAC Audio File"),MUSIK_FORMAT_AAC,&taglibinfo,&taglibinfo}
	,{wxT("mp4"),wxTRANSLATE("MP4 Audio File"),MUSIK_FORMAT_MP4,&taglibinfo,&taglibinfo}
	,{wxT("m4a"),wxTRANSLATE("MP4A Audio File"),MUSIK_FORMAT_MP4A,&taglibinfo,&taglibinfo}
#endif
	,{wxT("mp2"),wxTRANSLATE("MPEG Layer 2 Audio File"),MUSIK_FORMAT_MP2,&taglibinfo,&taglibinfo}

#else
	{wxT("mp3"),wxTRANSLATE("MPEG Layer 3 Audio File"),MUSIK_FORMAT_MP3,&mp3info,&mp3info}
	,{wxT("ogg"),wxTRANSLATE("OGG Vorbis Audio File"),MUSIK_FORMAT_OGG,&ogginfo,&ogginfo}
#ifndef MUSIKENGINE_NO_MPC_SUPPORT
	,{wxT("mpc"),wxTRANSLATE("Musepack Audio File"),MUSIK_FORMAT_MPC,&mpcinfo,&mpcinfo}
#endif
#ifndef MUSIKENGINE_NO_FLAC_SUPPORT
	,{wxT("flac"),wxTRANSLATE("Flac Lossless Audio File"),MUSIK_FORMAT_FLAC,&flacinfo,&flacinfo}
#endif
	,{wxT("mp2"),wxTRANSLATE("MPEG Layer 2 Audio File"),MUSIK_FORMAT_MP2,&mp3info,&mp3info}
#endif
#ifndef MUSIKENGINE_NO_APE_SUPPORT
	,{wxT("ape"),wxTRANSLATE("Monkey's Audio File"),MUSIK_FORMAT_APE,&apeinfo,&apeinfo}
#endif
#if defined(USE_FMOD3) || defined (USE_FMODEX)

	,{wxT("wav"),wxTRANSLATE("WAVE Audio File"),MUSIK_FORMAT_WAV,&fmodinfo,NULL}
	,{wxT("aiff"),wxTRANSLATE("AIFF Audio File"),MUSIK_FORMAT_AIFF,&fmodinfo,NULL}
#ifdef __WXMSW__
	,{wxT("wma"),wxTRANSLATE("Windows Media Audio File"),MUSIK_FORMAT_WMA,&fmodinfo,NULL}
#endif
#endif
};

const tSongClass * CMetaDataHandler::GetSongClass(const wxString &ext)
{
	for(size_t i = 0; i < WXSIZEOF(valid_SongClasses);i++)
		if(ext.CompareTo(valid_SongClasses[i].szExtension ,wxString::ignoreCase) == 0)
			return &valid_SongClasses[i];
	return NULL;
}
const tSongClass * CMetaDataHandler::GetSongClass(EMUSIK_FORMAT_TYPE eFormat)
{
	for(size_t i = 0; i < WXSIZEOF(valid_SongClasses);i++)
		if(valid_SongClasses[i].eFormat == eFormat)
			return &valid_SongClasses[i];
	return NULL;
}

CMetaDataHandler::RetCode CMetaDataHandler::GetMetaData( CSongMetaData & MetaData  )
{
    RetCode rc = success;
    try
    {
        //--- get format ---//
        wxString ext = MetaData.Filename.GetExt().MakeLower();
        
        const tSongClass *psc = GetSongClass(ext);
        wxASSERT(psc);
        if(psc == NULL)
            return fail;
        MetaData.eFormat = psc->eFormat;
        if(psc->pInfoRead)
        {
            rc = psc->pInfoRead->ReadMetaData(MetaData) ? success:fail;
        }
        else
            rc = notsupported;
    }
    catch(std::exception &e)
    {
        ::wxLogWarning(_("An exception (%s) occurred while reading tag of %s."),ConvA2W(e.what()).c_str(),MetaData.Filename.GetFullPath().c_str());
        rc = fail;
    }

    MetaData.Title.Trim();
    MetaData.Artist.Trim();
    MetaData.Album.Trim();
    MetaData.Genre.Trim();

	if (wxGetApp().Prefs.bAllowTagGuessing && MetaData.Title.Length() == 0 )
	{
		dummyinfo.ReadMetaData(MetaData);
		if ( MetaData.Title.Length() == 0 )
			MetaData.Title = ConvToUTF8( MetaData.Filename.GetFullPath() );
	}
	return rc;

}


CMetaDataHandler::RetCode CMetaDataHandler::WriteMetaData(const CSongMetaData &MetaData,bool bClearAll)
{
	const tSongClass *psc = GetSongClass(MetaData.eFormat);
	wxASSERT(psc);
	if(psc == NULL)
		return fail;
	RetCode rc = success;
    try
    {
        if(psc->pInfoWrite)
            rc  = psc->pInfoWrite->WriteMetaData(MetaData,bClearAll)? success:fail;
        else
            rc = notsupported;
    }
    catch(std::exception & e)
    {
        ::wxLogWarning(_("An exception (%s) occurred while writing tag of %s."),ConvA2W(e.what()).c_str(),MetaData.Filename.GetFullPath().c_str());
        rc = fail;
    }
	return rc;
}
