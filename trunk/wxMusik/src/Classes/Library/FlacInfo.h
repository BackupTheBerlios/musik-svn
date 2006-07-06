/*
*  FlacInfo.h
*
*  Helper functions to retrieve and write Flac (Vorbis) comments of a given file.
*
*  Copyright (c) 2004  Gunnar Roth
*
*  See the file "license.txt" for information on usage and redistribution
*  of this file, and for a DISCLAIMER OF ALL WARRANTIES.
*/
#ifndef FLACINFO_H
#define FLACINFO_H
#include "CInfo.h"
//--- globals ---//
#include "../../MusikGlobals.h"
#include "FLAC/file_decoder.h"
struct FLAC__Metadata_Chain;
class CFlacInfo : public CInfoRead,public CInfoWrite
{
public:
	CFlacInfo(void);
	~CFlacInfo();
	bool ReadMetaData(CSongMetaData & MetaData) const;
	bool WriteMetaData(const CSongMetaData & MetaData,bool bClearAll = false);
private:
	static bool SetVorbisComment(FLAC__Metadata_Chain *chain,const CSongMetaData & MetaData,bool bRemoveAll);
	static bool CFlacInfo::SetCommentEntry(FLAC__StreamMetadata *pBlock,const char *name, int value);
	static bool SetCommentEntry(FLAC__StreamMetadata *pBlock,const char *name, const char *value);
	static int ReadEntry( const FLAC__StreamMetadata *metadata,const char * name,int offset,CSongMetaData::StringData & s);
	static void FLACMetaCallback(const FLAC__FileDecoder *decoder, const FLAC__StreamMetadata *metadata, void *client_data);
	static FLAC__StreamDecoderWriteStatus FLACWriteCallback(const FLAC__FileDecoder *decoder, 
															const FLAC__Frame *frame, 
															const FLAC__int32 * const buffer[], void *client_data);
	static void FLACErrorCallback(const FLAC__FileDecoder *decoder, FLAC__StreamDecoderErrorStatus status, void *client_data); 

	FLAC__FileDecoder *m_pDecoder;
};
#endif
