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

#ifndef FLACDECODER_H
#define FLACDECODER_H

#include "MUSIKEngine/inc/decoder.h"

#ifdef MUSIKENGINE_DO_NOT_USE_OWN_DECODERS
#define MUSIKENGINE_NO_FLAC_SUPPORT
#endif

#ifndef MUSIKENGINE_NO_FLAC_SUPPORT

#include <FLAC/format.h>
#include <FLAC/stream_decoder.h>
#include <plugin_common/defs.h>
extern "C"
{
#include <plugin_common/dither.h>
#include "share/replaygain_synthesis.h"
}
#include "share/grabbag.h"


#define FLAC_SAMPLES_PER_WRITE 576

class MUSIKFLACDecoder :public MUSIKDecoder
{
	typedef struct {
		struct {
			bool enable;
			bool album_mode;
			int  preamp;
			bool hard_limit;
		} replaygain;
		struct {
			struct {
				bool dither_24_to_16;
			} normal;
			struct {
				bool dither;
				int  noise_shaping; /* value must be one of NoiseShaping enum, c.f. plugin_common/replaygain_synthesis.h */
				int  bps_out;
			} replaygain;
		} resolution;
	} output_config_t;
	struct FLACStreamInfo
	{
		FLAC__StreamDecoder					*Decoder;
		FLAC__StreamMetadata_StreamInfo		streaminfo;	/* FLAC: metadata infos */
		bool								flac_abort;	/* FLAC: abort flac when an error occured */
		bool								has_replaygain;
		double								replay_scale;
		output_config_t						cfg;
		FLAC__int32							reservoir_[FLAC_PLUGIN__MAX_SUPPORTED_CHANNELS][FLAC__MAX_BLOCK_SIZE * 2/*for overflow*/];
		FLAC__int32 *						reservoir__[FLAC_PLUGIN__MAX_SUPPORTED_CHANNELS];

		unsigned int						wide_samples_in_reservoir_;
		DitherContext dither_context;
		bool is_big_endian_host_;
		FLACStreamInfo()
		{
			memset(&streaminfo,0,sizeof(streaminfo));
			memset(&cfg,0,sizeof(cfg));
			memset(&dither_context,0,sizeof(dither_context));
			flac_abort = false;
			has_replaygain = false;
			replay_scale = 1.0;
			wide_samples_in_reservoir_ = 0;
			FLAC__uint32 test = 1;
			is_big_endian_host_ = (*((FLAC__byte*)(&test)))? false : true;	
			reservoir__[0] = reservoir_[0];
			reservoir__[1] = reservoir_[1]; /*@@@ kind of a hard-coded hack */
		}

	};

	MUSIKFLACDecoder(IMUSIKStreamOut * pIMUSIKStreamOut);

public:
	~MUSIKFLACDecoder();

	virtual bool CanSeek()
	{
		return true;
	}

	bool Close();
	virtual const char * Type()
	{
		return "Free Lossless Audio Codec (FLAC) File";
	}

protected:
	virtual bool OpenMedia(const char *FileName);

	int DecodeBlocks(unsigned char *buff,int len);
	bool DoSeek(int64_t samplepos);

	static FLAC__StreamDecoderWriteStatus FLACWriteCallback(const FLAC__StreamDecoder *decoder, 
		const FLAC__Frame *frame, 
		const FLAC__int32 * const buffer[], void *client_data);
	static void FLACMetaCallback(const FLAC__StreamDecoder *decoder, const FLAC__StreamMetadata *metadata, void *client_data);
	static void FLACErrorCallback(const FLAC__StreamDecoder *decoder, FLAC__StreamDecoderErrorStatus status, void *client_data);

private:
	void safe_decoder_finish_()
	{
		if(m_FlacInfo.Decoder && FLAC__stream_decoder_get_state(m_FlacInfo.Decoder) != FLAC__STREAM_DECODER_UNINITIALIZED)
			FLAC__stream_decoder_finish(m_FlacInfo.Decoder );
	}

	void safe_decoder_delete_()
	{
		if(m_FlacInfo.Decoder ) {
			safe_decoder_finish_();
			FLAC__stream_decoder_delete(m_FlacInfo.Decoder );
		}
		m_FlacInfo.Decoder = NULL;
	}

	FLACStreamInfo m_FlacInfo;

	friend class MUSIKEngine;
};
#endif //!MUSIKENGINE_NO_FLAC_SUPPORT

#endif
