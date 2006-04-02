//Copyright (c) 2004, Gunnar Roth
//All rights reserved.
//This code is derived from :

/* in_flac - Winamp2 FLAC input plugin
* Copyright (C) 2000,2001,2002,2003,2004  Josh Coalson
* and from
* libxmms-flac - XMMS FLAC input plugin
* Copyright (C) 2000,2001,2002,2003,2004  Josh Coalson
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/
#include "flacdecoder.h"
#ifndef MUSIKENGINE_NO_FLAC_SUPPORT
#include "MUSIKEngine/inc/imusikstreamout.h"
#include <utility>

#ifdef __VISUALC__
#pragma comment(lib,"libFLAC_static")
#endif

#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

MUSIKFLACDecoder::MUSIKFLACDecoder(IMUSIKStreamOut * pIMUSIKStreamOut)
:MUSIKDecoder(pIMUSIKStreamOut)
{

}
MUSIKFLACDecoder::~MUSIKFLACDecoder()
{
	safe_decoder_delete_();
}

bool MUSIKFLACDecoder::OpenMedia(const char *FileName)
{
	FILE *MyFLACFILE=fopen(FileName,"rb");
	fseek(MyFLACFILE,0,SEEK_END);
	m_Info.FileSize = ftell(MyFLACFILE);
	fclose(MyFLACFILE);
	m_FlacInfo.flac_abort = false;
	m_Info.SampleCount = 0;
	FLAC__FileDecoder *decoder  = FLAC__file_decoder_new();
	m_FlacInfo.Decoder = decoder;
	FLAC__file_decoder_set_client_data(decoder,(void *)&m_FlacInfo);
	FLAC__file_decoder_set_md5_checking(decoder, false);
	FLAC__file_decoder_set_filename(decoder, FileName);
	FLAC__file_decoder_set_metadata_ignore_all(decoder);
	FLAC__file_decoder_set_metadata_respond(decoder, FLAC__METADATA_TYPE_STREAMINFO);
	FLAC__file_decoder_set_metadata_respond(decoder, FLAC__METADATA_TYPE_VORBIS_COMMENT);
	FLAC__file_decoder_set_metadata_callback(decoder, FLACMetaCallback);
	FLAC__file_decoder_set_write_callback(decoder, FLACWriteCallback);
	FLAC__file_decoder_set_error_callback(decoder,FLACErrorCallback);
	FLAC__FileDecoderState nRetVal = FLAC__file_decoder_init(decoder);
	if (nRetVal == FLAC__FILE_DECODER_OK)
	{
		FLAC__file_decoder_process_until_end_of_metadata(decoder);

		int decoder_buffer_size = (FLAC_SAMPLES_PER_WRITE * m_FlacInfo.streaminfo.channels * m_FlacInfo.streaminfo.bits_per_sample)*4;
		/* convert from samples to ms */
		m_Info.SampleCount = m_FlacInfo.streaminfo.total_samples;
		m_Info.channels = m_FlacInfo.streaminfo.channels;
		m_Info.frequency = m_FlacInfo.streaminfo.sample_rate;
		m_Info.bitrate = (int)(m_Info.FileSize / (125*m_FlacInfo.streaminfo.total_samples/m_FlacInfo.streaminfo.sample_rate));
		if(m_FlacInfo.cfg.resolution.replaygain.bps_out == 0)
            m_FlacInfo.cfg.resolution.replaygain.bps_out = m_FlacInfo.streaminfo.bits_per_sample;
        m_Info.bits_per_sample = m_FlacInfo.has_replaygain && m_FlacInfo.cfg.replaygain.enable ?
				m_FlacInfo.cfg.resolution.replaygain.bps_out :
					m_FlacInfo.cfg.resolution.normal.dither_24_to_16 ? min(m_Info.bits_per_sample, 16) : 
																		m_FlacInfo.streaminfo.bits_per_sample;
		if (m_FlacInfo.has_replaygain && m_FlacInfo.cfg.replaygain.enable && m_FlacInfo.cfg.resolution.replaygain.dither)
			FLAC__replaygain_synthesis__init_dither_context(&m_FlacInfo.dither_context, m_FlacInfo.streaminfo.bits_per_sample, m_FlacInfo.cfg.resolution.replaygain.noise_shaping);

		return CreateBuffer(decoder_buffer_size);
	}
	Close();
	return false;
}



int MUSIKFLACDecoder::DecodeBlocks(unsigned char *buff,int /*len*/)
{
	const unsigned channels = m_Info.channels;
	const unsigned bits_per_sample = m_FlacInfo.streaminfo.bits_per_sample;
	const unsigned target_bps = m_Info.bits_per_sample;
	
	while(m_FlacInfo.wide_samples_in_reservoir_ < FLAC_SAMPLES_PER_WRITE) 
	{
		if(FLAC__file_decoder_get_state(m_FlacInfo.Decoder) == FLAC__FILE_DECODER_END_OF_FILE) {
			return 0;
		}
		else if(!FLAC__file_decoder_process_single(m_FlacInfo.Decoder)) {
			//show_error("Error while processing frame (%s).", FLAC__FileDecoderStateString[FLAC__file_decoder_get_state(m_FlacInfo.Decoder)]);
			return 0;
		}
	}

	if(m_FlacInfo.wide_samples_in_reservoir_ ) 
	{
		const unsigned n = min(m_FlacInfo.wide_samples_in_reservoir_, FLAC_SAMPLES_PER_WRITE);
		//const unsigned delta = n * channels;
		int bytes;
		unsigned i;

		if(m_FlacInfo.cfg.replaygain.enable && m_FlacInfo.has_replaygain) {
			bytes = (int)FLAC__replaygain_synthesis__apply_gain(
				buff,
				!m_FlacInfo.is_big_endian_host_,
				false, /* unsigned_data_out */
				m_FlacInfo.reservoir__,
				n,
				channels,
				bits_per_sample,
				target_bps,
				m_FlacInfo.replay_scale,
				m_FlacInfo.cfg.replaygain.hard_limit,
				m_FlacInfo.cfg.resolution.replaygain.dither,
				&m_FlacInfo.dither_context
				);
		}
		else if(m_FlacInfo.is_big_endian_host_) 
		{
			bytes = (int)FLAC__plugin_common__pack_pcm_signed_big_endian(
																			buff,
																			m_FlacInfo.reservoir__,
																			n,
																			channels,
																			bits_per_sample,
																			target_bps
																			);
		}
		else 
		{
				bytes = (int)FLAC__plugin_common__pack_pcm_signed_little_endian(
																				buff,
																				m_FlacInfo.reservoir__,
																				n,
																				channels,
																				bits_per_sample,
																				target_bps
																				);
		}

		for (i = 0; i < channels; i++)
			memmove(&m_FlacInfo.reservoir_[i][0], &m_FlacInfo.reservoir_[i][n], sizeof(m_FlacInfo.reservoir_[0][0]) * m_FlacInfo.wide_samples_in_reservoir_);
		m_FlacInfo.wide_samples_in_reservoir_ -= n;

		IncDecodeSamplePos(n);
		return bytes;
	}
	return 0;
}
FLAC__StreamDecoderWriteStatus MUSIKFLACDecoder::FLACWriteCallback(const FLAC__FileDecoder *decoder, 
																	const FLAC__Frame *frame, 
																	const FLAC__int32 * const buffer[], void *client_data)
{
	FLACStreamInfo* pFlacInfo=(FLACStreamInfo *)client_data;
	const unsigned channels = 2, wide_samples = frame->header.blocksize;
	unsigned channel;

	(void)decoder;

	if(pFlacInfo->flac_abort)
		return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
	for (channel = 0; channel < channels; channel++)
		memcpy(&pFlacInfo->reservoir_[channel][pFlacInfo->wide_samples_in_reservoir_], buffer[channel], sizeof(buffer[0][0]) * wide_samples);
	pFlacInfo->wide_samples_in_reservoir_ += wide_samples;

	return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

void MUSIKFLACDecoder::FLACMetaCallback(const FLAC__FileDecoder * /*decoder*/, const FLAC__StreamMetadata *metadata, void *client_data)
{
	/* TO-DO: Meta data */
	FLACStreamInfo *pFlacInfo=(FLACStreamInfo *)client_data;
	if(metadata->type == FLAC__METADATA_TYPE_STREAMINFO) 
	{
		pFlacInfo->streaminfo = metadata->data.stream_info;
	}
	else if(metadata->type == FLAC__METADATA_TYPE_VORBIS_COMMENT) 
	{
//#if temporary_disabled
		double gain, peak;
		if(grabbag__replaygain_load_from_vorbiscomment(metadata, pFlacInfo->cfg.replaygain.album_mode, &gain, &peak)) {
			pFlacInfo->has_replaygain = true;
			pFlacInfo->replay_scale = grabbag__replaygain_compute_scale_factor(peak, gain, (double)pFlacInfo->cfg.replaygain.preamp, !pFlacInfo->cfg.replaygain.hard_limit);
		}
//#endif
	}

	return;
}


void MUSIKFLACDecoder::FLACErrorCallback(const FLAC__FileDecoder * /*decoder*/, FLAC__StreamDecoderErrorStatus status, void *client_data) 
{
	/* TO-DO: Error handling */
	FLACStreamInfo *pFlacInfo=(FLACStreamInfo *)client_data;
	if (status != FLAC__STREAM_DECODER_ERROR_STATUS_LOST_SYNC)
	{
		pFlacInfo->flac_abort = true;
	}

	return ;
}


bool MUSIKFLACDecoder::DoSeek(int64_t samplepos)
{
	
	if(FLAC__file_decoder_seek_absolute(m_FlacInfo.Decoder, (FLAC__uint64)samplepos)) {
		SetDecodeSamplePos(samplepos);
		return true;
	}
	return false;
}

bool MUSIKFLACDecoder::Close()
{
	MUSIKDecoder::Close();
	FLAC__file_decoder_delete(m_FlacInfo.Decoder);
	return true;
}

#endif //MUSIKENGINE_NO_FLAC_SUPPORT

