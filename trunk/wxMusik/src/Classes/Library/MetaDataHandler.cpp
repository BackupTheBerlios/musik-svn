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
#include "wx/wxprec.h"
//--- mp3 / ogg helpers ---//
#include "MusikUtils.h"
#include "3rd-Party/TagHelper/idtag.h"
#include "MetaDataHandler.h"

#include "APEInfo.h"

#define USE_TAGLIB

#ifdef USE_TAGLIB
#include "TagLibInfo.h"
static CTagLibInfo taglibinfo;
#else
#include "CMP3Info.h"
#include "COggInfo.h"
#include "MPCInfo.h"
#include "FlacInfo.h"
static CMP3Info mp3info;
static COggInfo ogginfo;
static CMPCInfo mpcinfo;
static CFlacInfo flacinfo;
#endif
#include "CFMODInfo.h"
static CFMODInfo fmodinfo;
static CMyAPEInfo apeinfo;

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
		MetaData.Album =  tr.TagValue(SIMPLETAG_FIELD_ALBUM);
		MetaData.Title =  tr.TagValue(SIMPLETAG_FIELD_TITLE);
		MetaData.Notes =  tr.TagValue(SIMPLETAG_FIELD_NOTES);
		MetaData.Artist =  tr.TagValue(SIMPLETAG_FIELD_ARTIST);
		MetaData.Year =  tr.TagValue(SIMPLETAG_FIELD_YEAR);
		MetaData.Genre =  tr.TagValue(SIMPLETAG_FIELD_GENRE);
		char* track = tr.TagValue(SIMPLETAG_FIELD_TRACK);
		MetaData.nTracknum = track ? atoi(track) : 0;
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
	,{wxT("mpc"),wxTRANSLATE("Musepack Audio File"),MUSIK_FORMAT_MPC,&taglibinfo,&taglibinfo}
	,{wxT("flac"),wxTRANSLATE("Flac Lossless Audio File"),MUSIK_FORMAT_FLAC,&taglibinfo,&taglibinfo}
	,{wxT("mp2"),wxTRANSLATE("MPEG Layer 2 Audio File"),MUSIK_FORMAT_MP2,&taglibinfo,&taglibinfo}

#else
	{wxT("mp3"),wxTRANSLATE("MPEG Layer 3 Audio File"),MUSIK_FORMAT_MP3,&mp3info,&mp3info}
	,{wxT("ogg"),wxTRANSLATE("OGG Vorbis Audio File"),MUSIK_FORMAT_OGG,&ogginfo,&ogginfo}
	,{wxT("mpc"),wxTRANSLATE("Musepack Audio File"),MUSIK_FORMAT_MPC,&mpcinfo,&mpcinfo}
	,{wxT("flac"),wxTRANSLATE("Flac Lossless Audio File"),MUSIK_FORMAT_FLAC,&flacinfo,&flacinfo}
	,{wxT("mp2"),wxTRANSLATE("MPEG Layer 2 Audio File"),MUSIK_FORMAT_MP2,&mp3info,&mp3info}
#endif
	,{wxT("ape"),wxTRANSLATE("Monkey's Audio File"),MUSIK_FORMAT_APE,&apeinfo,&apeinfo}
	,{wxT("wav"),wxTRANSLATE("WAVE Audio File"),MUSIK_FORMAT_WAV,&fmodinfo,NULL}
	,{wxT("aiff"),wxTRANSLATE("AIFF Audio File"),MUSIK_FORMAT_AIFF,&fmodinfo,NULL}
#ifdef __WXMSW__
	,{wxT("wma"),wxTRANSLATE("Windows Media Audio File"),MUSIK_FORMAT_WMA,&fmodinfo,NULL}
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

	//--- get format ---//
	wxString ext = MetaData.Filename.GetExt().MakeLower();
	
	const tSongClass *psc = GetSongClass(ext);
	wxASSERT(psc);
	if(psc == NULL)
		return fail;
	MetaData.eFormat = psc->eFormat;
	RetCode rc = success;
	if(psc->pInfoRead)
	{
		rc = psc->pInfoRead->ReadMetaData(MetaData) ? success:fail;
	}
	else
		rc = notsupported;

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
	if(psc->pInfoWrite)
		rc  = psc->pInfoWrite->WriteMetaData(MetaData,bClearAll)? success:fail;
	else
		rc = notsupported;

	return rc;
}
