#include "myprec.h"
#ifndef MUSIKENGINE_NO_APE_SUPPORT
#include "Shared/All.h"							/* Monkey's Audio include file */
#include "MACLib/MACLib.h"						/* Monkey's Audio include file */
#include "MACLib/APETag.h"
#ifdef _WIN32
#else
#define BUILD_CROSS_PLATFORM
#endif
#include "APEInfo.h"
#include "MusikUtils.h"

#ifdef __VISUALC__
#pragma comment(lib,"MACLib")
#endif

CMyAPEInfo::CMyAPEInfo(void)
{
}
bool CMyAPEInfo::ReadMetaData(CSongMetaData & MetaData) const
{
	return ReadFileData(MetaData) && ReadTagData(MetaData);
}
bool CMyAPEInfo::ReadTagData(CSongMetaData & MetaData) const
{
    CAPETag tag(MetaData.Filename.GetFullPath().wc_str(wxConvFile));
	GetFieldAsUtf8(APE_TAG_FIELD_TITLE,tag,MetaData.Title);
	GetFieldAsUtf8(APE_TAG_FIELD_ARTIST,tag,MetaData.Artist);
	GetFieldAsUtf8(APE_TAG_FIELD_ALBUM,tag,MetaData.Album);
	GetFieldAsUtf8(APE_TAG_FIELD_GENRE,tag,MetaData.Genre);
	GetFieldAsUtf8(APE_TAG_FIELD_NOTES,tag,MetaData.Notes);
	GetFieldAsUtf8(APE_TAG_FIELD_YEAR,tag,MetaData.Year);
	CSongMetaData::StringData s;
	GetFieldAsUtf8(APE_TAG_FIELD_TRACK,tag,s);
	MetaData.nTracknum = atoi(s);
	return true;
}

bool  CMyAPEInfo::WriteMetaData(const CSongMetaData & MetaData,bool bClearAll)
{
	CAPETag tag(MetaData.Filename.GetFullPath().wc_str(wxConvFile));
	if(bClearAll)
		tag.ClearFields();
	tag.SetFieldString(APE_TAG_FIELD_TITLE,MetaData.Title,TRUE);
	tag.SetFieldString(APE_TAG_FIELD_ARTIST,MetaData.Artist,TRUE);
	tag.SetFieldString(APE_TAG_FIELD_ALBUM,MetaData.Album,TRUE);
	tag.SetFieldString(APE_TAG_FIELD_GENRE,MetaData.Genre,TRUE);
	tag.SetFieldString(APE_TAG_FIELD_NOTES,MetaData.Notes,TRUE);
	tag.SetFieldString(APE_TAG_FIELD_YEAR,MetaData.Year,TRUE);
	char buf[20];
	sprintf(buf,"%d",MetaData.nTracknum);
	tag.SetFieldString(APE_TAG_FIELD_TRACK,buf,TRUE);
	return tag.Save() == ERROR_SUCCESS;
}

bool CMyAPEInfo::ReadFileData(CSongMetaData & MetaData) const
{
	int nRetVal=0;
	IAPEDecompress * pAPEDecompress = CreateIAPEDecompress(MetaData.Filename.GetFullPath().wc_str(wxConvFile), &nRetVal);
	if (pAPEDecompress != NULL)
	{
		MetaData.nBitrate = pAPEDecompress->GetInfo(APE_INFO_AVERAGE_BITRATE);
		MetaData.nDuration_ms = pAPEDecompress->GetInfo(APE_DECOMPRESS_LENGTH_MS);
		MetaData.nFilesize = pAPEDecompress->GetInfo(APE_INFO_APE_TOTAL_BYTES);
		delete pAPEDecompress; /* Delete IAPEDecompress interface */
		return true;
	}
	return false;
}
bool CMyAPEInfo::GetFieldAsUtf8(wchar_t *pFieldName,CAPETag &tag, CSongMetaData::StringData &s)
{
	CAPETagField * pAPETagField = tag.GetTagField(pFieldName);
	if (pAPETagField == NULL)
	{
		return false;
	}
	else if (pAPETagField->GetIsUTF8Text() || (tag.GetAPETagVersion() < 2000))
	{
		if (tag.GetAPETagVersion() >= 2000)
			s = pAPETagField->GetFieldValue();
		else
			s = ConvFromISO8859_1ToUTF8(pAPETagField->GetFieldValue());

	}
	else
	{
		wxASSERT(false);
		return false;
	}
	return true;
}
#endif // #ifndef MUSIKENGINE_NO_APE_SUPPORT
