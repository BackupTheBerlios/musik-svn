#include "wx/wxprec.h"
#include <FLAC/format.h>
#include <FLAC/metadata.h>
#include "FlacInfo.h"
#include "MusikUtils.h"

CFlacInfo::CFlacInfo(void)
{
	m_pDecoder = FLAC__file_decoder_new();

}
CFlacInfo::~CFlacInfo()
{
	FLAC__file_decoder_delete(m_pDecoder);

}

void CFlacInfo::FLACMetaCallback(const FLAC__FileDecoder *WXUNUSED(decoder), const FLAC__StreamMetadata *metadata, void *client_data)
{
	
	CSongMetaData *pMetaData = (CSongMetaData *)client_data;
	if(metadata->type == FLAC__METADATA_TYPE_STREAMINFO) 
	{
		pMetaData->nBitrate = (int)(pMetaData->nFilesize / (125*metadata->data.stream_info.total_samples/metadata->data.stream_info.sample_rate));
		pMetaData->nDuration_ms = (unsigned int)((FLAC__uint64)1000 * metadata->data.stream_info.total_samples / metadata->data.stream_info.sample_rate);
	}
	else if(metadata->type == FLAC__METADATA_TYPE_VORBIS_COMMENT) 
	{
		ReadEntry(metadata,"ARTIST",0,pMetaData->Artist);
		ReadEntry(metadata,"ALBUM",0,pMetaData->Album);
		ReadEntry(metadata,"TITLE",0,pMetaData->Title);
		ReadEntry(metadata,"GENRE",0,pMetaData->Genre);
		ReadEntry(metadata,"COMMENT",0,pMetaData->Notes);
		if(pMetaData->Notes.IsEmpty())
			  ReadEntry(metadata,"DESCRIPTION",0,pMetaData->Notes);
		ReadEntry(metadata,"DATE",0,pMetaData->Year);
		CSongMetaData::StringData s;
		ReadEntry(metadata,"TRACKNUMBER",0,s);
		pMetaData->nTracknum = !s.IsEmpty() ?  atol( s ) : 0;
	}

	return;
}
int  CFlacInfo::ReadEntry( const FLAC__StreamMetadata *metadata,const char * name,int offset,CSongMetaData::StringData & s) 
{
	int entrynr = FLAC__metadata_object_vorbiscomment_find_entry_from(metadata,offset,name);
	if(entrynr != -1)
	{
		FLAC__StreamMetadata_VorbisComment_Entry  & e = metadata->data.vorbis_comment.comments[entrynr];
		CSongMetaData::StringData entry((char*)e.entry,e.length);
		off_t seppos = entry.Find('=');
		if(	seppos > -1 )
		{
			s = entry.Substr(seppos + 1);
		}
	}
	return entrynr;
}
bool CFlacInfo::ReadMetaData(CSongMetaData & MetaData) const
{
	FILE *MyFLACFILE=wxFopen(MetaData.Filename.GetFullPath(),wxT("rb"));
	if(MyFLACFILE == NULL)
		return false;
	fseek(MyFLACFILE,0,SEEK_END);
	MetaData.nFilesize = ftell(MyFLACFILE);
	MetaData.bVBR= false;
	fclose(MyFLACFILE);
	FLAC__file_decoder_set_client_data(m_pDecoder,(void *)&MetaData);
	FLAC__file_decoder_set_md5_checking(m_pDecoder, false);
	FLAC__file_decoder_set_filename(m_pDecoder, ConvFn2A(MetaData.Filename.GetFullPath()));
	FLAC__file_decoder_set_metadata_ignore_all(m_pDecoder);
	FLAC__file_decoder_set_metadata_respond(m_pDecoder, FLAC__METADATA_TYPE_STREAMINFO);
	FLAC__file_decoder_set_metadata_respond(m_pDecoder, FLAC__METADATA_TYPE_VORBIS_COMMENT);
	FLAC__file_decoder_set_metadata_callback(m_pDecoder, FLACMetaCallback);
	FLAC__file_decoder_set_write_callback(m_pDecoder, FLACWriteCallback);
	FLAC__file_decoder_set_error_callback(m_pDecoder, FLACErrorCallback);

	FLAC__FileDecoderState nRetVal = FLAC__file_decoder_init(m_pDecoder);
	if (nRetVal == FLAC__FILE_DECODER_OK)
	{
		FLAC__file_decoder_process_until_end_of_metadata(m_pDecoder);
	}
	FLAC__file_decoder_finish(m_pDecoder);
	return nRetVal == FLAC__FILE_DECODER_OK;
}

FLAC__StreamDecoderWriteStatus CFlacInfo::FLACWriteCallback(const FLAC__FileDecoder *WXUNUSED(decoder), 
																   const FLAC__Frame *WXUNUSED(frame), 
																   const FLAC__int32 * const WXUNUSED(buffer)[], void * WXUNUSED(client_data))
{
	WXUNUSED(decoder);
	WXUNUSED(frame);
	WXUNUSED(buffer);
	WXUNUSED(client_data);
	return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
}
void CFlacInfo::FLACErrorCallback(const FLAC__FileDecoder *WXUNUSED(decoder), FLAC__StreamDecoderErrorStatus WXUNUSED(status), void *WXUNUSED(client_data)) 
{
}

bool CFlacInfo::WriteMetaData(const CSongMetaData & MetaData,bool bClearAll)
{
	FLAC__Metadata_Chain *chain = FLAC__metadata_chain_new();
	if(!chain)
		return false;
	if(!FLAC__metadata_chain_read(chain, ConvFn2A(MetaData.Filename.GetFullPath()))) {
		::wxLogWarning(wxT("%s: ERROR: reading metadata, status = \"%s\""), MetaData.Filename.GetFullPath().c_str(), ConvA2W(FLAC__Metadata_ChainStatusString[FLAC__metadata_chain_status(chain)]).c_str());
		return false;
	}

	bool bRes = SetVorbisComment(chain,MetaData,bClearAll);
	if(bRes)
	{
		FLAC__metadata_chain_sort_padding(chain);
		if(!FLAC__metadata_chain_write(chain, true /*use padding*/, false /* preserve modtime*/))
		{
			::wxLogWarning(wxT("%s: ERROR: writing FLAC file, error = %s"), MetaData.Filename.GetFullPath().c_str(), ConvA2W(FLAC__Metadata_ChainStatusString[FLAC__metadata_chain_status(chain)]).c_str());
		}
	}
	FLAC__metadata_chain_delete(chain);
	return bRes;
}
bool CFlacInfo::SetVorbisComment(FLAC__Metadata_Chain *chain,const CSongMetaData & MetaData,bool bRemoveAll)
{
	FLAC__StreamMetadata *pBlock = 0;
	FLAC__Metadata_Iterator *pIt = FLAC__metadata_iterator_new();
	if(!pIt)
		return false;
	FLAC__metadata_iterator_init(pIt, chain);
	bool bBlockFound = false;
	do {
		pBlock = FLAC__metadata_iterator_get_block(pIt);
		if(pBlock->type == FLAC__METADATA_TYPE_VORBIS_COMMENT)
			bBlockFound = true;
	} while(!bBlockFound && FLAC__metadata_iterator_next(pIt));

	if(!bBlockFound) 
	{
		// no block found  => create one
		pBlock = FLAC__metadata_object_new(FLAC__METADATA_TYPE_VORBIS_COMMENT);
		if(!pBlock)
			return false;
		// skip all other metadata
		while(FLAC__metadata_iterator_next(pIt))
			;
		// insert new block at the end of metadata blocks
		if(!FLAC__metadata_iterator_insert_block_after(pIt, pBlock)) 
		{
			return false;
		}
		// assert pIt points to pBlock
		wxASSERT(FLAC__metadata_iterator_get_block(pIt) == pBlock);
	}
	else if(bRemoveAll)
	{
		if(pBlock->data.vorbis_comment.comments) 
		{
			wxASSERT(pBlock->data.vorbis_comment.num_comments > 0);
			if(!FLAC__metadata_object_vorbiscomment_resize_comments(pBlock, 0)) {
				return false;
			}
		}
	}


	wxASSERT(pBlock);
	wxASSERT(pBlock->type == FLAC__METADATA_TYPE_VORBIS_COMMENT);
	bool bRes =		SetCommentEntry(pBlock,"ARTIST",MetaData.Artist)
				&&	SetCommentEntry(pBlock,"ALBUM",MetaData.Album)
				&&	SetCommentEntry(pBlock,"TITLE",MetaData.Title)
				&&	SetCommentEntry(pBlock,"GENRE",MetaData.Genre)
				&&	SetCommentEntry(pBlock,"DATE",MetaData.Year)
				&&	SetCommentEntry(pBlock,"DESCRIPTION",MetaData.Notes)
				&&	SetCommentEntry(pBlock,"TRACKNUMBER",MetaData.nTracknum)
				;

	FLAC__metadata_iterator_delete(pIt);
	return bRes; 
}
bool CFlacInfo::SetCommentEntry(FLAC__StreamMetadata *pBlock,const char *name, int value)
{
	wxString num;
	num << value;
	return SetCommentEntry(pBlock,name,ConvW2A(num));
}

bool CFlacInfo::SetCommentEntry(FLAC__StreamMetadata *pBlock,const char *name, const char *value)
{
	int entrynr = FLAC__metadata_object_vorbiscomment_find_entry_from(pBlock,0,name);//find first entry matching name
	if((entrynr != -1) && (value == NULL))
	{
		FLAC__metadata_object_vorbiscomment_delete_comment(pBlock, entrynr);//delete first existing
		return true;
	}
	int nEntryLen = strlen(name) + strlen(value) + 1;  // "name=value"
	char *szEntry = (char *)malloc(nEntryLen + 1);
	if(szEntry) 
	{
		FLAC__StreamMetadata_VorbisComment_Entry e;

		sprintf(szEntry, "%s=%s", name, value);

		e.length = nEntryLen;
		e.entry = (FLAC__byte*)szEntry;

		if(entrynr == -1)
			//no existing comment entry
			FLAC__metadata_object_vorbiscomment_insert_comment(pBlock, pBlock->data.vorbis_comment.num_comments, e,false);//takes ownership  of e.entry
		else
			//change existing comment entry
			FLAC__metadata_object_vorbiscomment_set_comment(pBlock, entrynr, e, false);//takes ownership  of e.entry
	}
	return true;
}
