#include "wx/wxprec.h"
#include "MPCInfo.h"
#include "../../MusikUtils.h"
#include "../../3rd Party/TagHelper/idtag.h"
extern "C"_CRTIMP int __cdecl _chsize(int, long);

#include "MUSIKEngine/MPC/mpc_dec.h"
#include "MUSIKEngine/MPC/in_mpc.h"

bool CMPCInfo::ReadMetaData(CSongMetaData & MetaData) const
{
	StreamInfo info;											
	FILE *fp = wxFopen( MetaData.Filename.GetFullPath(), wxT("rb") );
	if (!fp) 
		return false;
	Reader_file reader(fp); 
	if (info.ReadStreamInfo(&reader) != ERROR_CODE_OK)
	{
		return false;
	}
	MetaData.nBitrate = (int)(info.simple.AverageBitrate / 1024.0);
	MetaData.nFilesize = info.simple.TotalFileLength;
	MetaData.nDuration_ms = info.simple.PCMSamples * 1000 / info.simple.SampleFreq;

	CSimpleTagReader tr;
	CSimpleTagReader::CFile trf(fp,false);
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