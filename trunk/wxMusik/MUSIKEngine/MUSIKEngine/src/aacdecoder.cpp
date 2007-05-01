//Copyright (c) 2006, Gunnar Roth
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
#include "aacdecoder.h"
#ifndef MUSIKENGINE_NO_FAAD2_SUPPORT


#include "MUSIKEngine/inc/imusikstreamout.h"
#include <utility>

#ifdef __VISUALC__
#pragma comment(lib,"faad2")
#endif

#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

#ifndef MAKEFOURCC
#ifdef _BIG_ENDIAN
#define MAKEFOURCC(ch0, ch1, ch2, ch3)                              \
                ((uint32_t)(uint8_t)(ch3) | ((uint32_t)(uint8_t)(ch2) << 8) |   \
                ((uint32_t)(uint8_t)(ch1) << 16) | ((uint32_t)(uint8_t)(ch0) << 24 ))
#else
#define MAKEFOURCC(ch0, ch1, ch2, ch3)                              \
                ((uint32_t)(uint8_t)(ch0) | ((uint32_t)(uint8_t)(ch1) << 8) |   \
                ((uint32_t)(uint8_t)(ch2) << 16) | ((uint32_t)(uint8_t)(ch3) << 24 ))
#endif
#endif

uint32_t MUSIKAACDecoder::AACStreamInfo::GetSampleRate(int index) const
{
	static const uint32_t sample_rates[] =
    {
		96000, 88200, 64000, 48000, 44100, 32000,
		24000, 22050, 16000, 12000, 11025, 8000
	};
	
	if (index < 12)
		return sample_rates[index];
	
	return 0;
}

int MUSIKAACDecoder::AACStreamInfo::FindAACIndex()
{
	int cnt = mp4ff_total_tracks(m_pMP4File);
	for( int i = 0 ; i < cnt; i++)
	{
		unsigned char *buf=0;
		unsigned int bufsize=0;
		mp4AudioSpecificConfig mp4ASC;
		mp4ff_get_decoder_config(m_pMP4File,i,&buf,&bufsize);
		if (buf) 
		{
			if (NeAACDecAudioSpecificConfig(buf,bufsize,&mp4ASC)<0) 
			{
				free(buf);
				continue;
			}
			free(buf);
			return i;
		}
		
	}
	return -1;
}


int  MUSIKAACDecoder::AACStreamInfo::read_ADIF_header()
{
	uint8_t buffer[30];

	if (m_pReader->read(buffer,sizeof(buffer))!=sizeof(buffer))
		return -1;

	int skip_size=0;

	/* copyright string */
	if (buffer[4] & 0x80)
		skip_size += 9; /* skip 9 bytes */

	m_BitRate = ((unsigned int)(buffer[4 + skip_size] & 0x0F)<<19)|
		((unsigned int)buffer[5 + skip_size]<<11)|
		((unsigned int)buffer[6 + skip_size]<<3)|
		((unsigned int)buffer[7 + skip_size] & 0xE0);

	return 0;
}

int MUSIKAACDecoder::AACStreamInfo::read_ADTS_header()
{
	uint8_t buffer[10];
	int frames,frame_length;

	int nextsec=0;
	int second=0;
	uint32_t startpos=m_pReader->tell(),pos=startpos;

	for (frames=0;;frames++) 
	{

		if (m_pReader->read(buffer,10)!=10 || buffer[0]!=0xff || (buffer[1]&0xf6)!=0xf0)
			break;

		if (!frames) 
		{
			m_SampleRate = GetSampleRate((buffer[2]&0x3C)>>2);
			m_Channels = ((buffer[2]&0x01)<<2)|((buffer[3]&0xC0)>>6);
		}

		frame_length = ((((unsigned int)buffer[3] & 0x3)) << 11)
			| (((unsigned int)buffer[4]) << 3) | (buffer[5] >> 5);
		if (!frame_length) break;

		if (frames==nextsec) 
		{
			m_pSeektable = (uint32_t *) realloc(m_pSeektable, (second + 1) * sizeof(uint32_t));
			m_pSeektable[second] = pos;
			++second;
			nextsec=second * m_SampleRate/1024; // start frame of next second
		}

		pos += frame_length;
		if (!m_pReader->seek(pos)) break;
	}

	if (frames<2) return -1; // probably not an AAC file!

	m_BitRate = (int)((((pos-startpos)  / frames) * (m_SampleRate/1024.0)) +0.5)*8;
	m_Milliseconds = (1024.f*frames*1000/m_SampleRate);

	return 0;
}

int MUSIKAACDecoder::AACStreamInfo::get_AAC_format()
{
	uint8_t buf[1000];
	int a=0,b=m_pReader->read(buf,sizeof(buf));
	do 
	{
		if (*(uint32_t*)(buf+a)==MAKEFOURCC('A','D','I','F')) 
		{ // "ADIF" signature
			m_initbytes+=a;
			m_pReader->seek(m_initbytes);
			return read_ADIF_header();
		}
		if (buf[a]==0xff && (buf[a+1]&0xf6)==0xf0 && ((buf[a+2]&0x3C)>>2)<12) 
		{ // ADTS header syncword
			m_initbytes+=a;
			m_pReader->seek(m_initbytes);
			return read_ADTS_header();
		}
	} while (++a<b-4);
	return -1;
}


MUSIKAACDecoder::MUSIKAACDecoder(IMUSIKStreamOut * pIMUSIKStreamOut)
:MUSIKDecoder(pIMUSIKStreamOut)
{

}
MUSIKAACDecoder::~MUSIKAACDecoder()
{
}

bool MUSIKAACDecoder::OpenMedia(const char *FileName)
{
	FILE *fp = fopen ( FileName, "rb" );
	if (fp) 
		m_AacInfo.Open(new Reader (fp,true)); 
	else 
		return false;

	uint32_t buf[3];
	m_AacInfo.m_pReader->read(buf,sizeof(buf));
	m_AacInfo.m_pReader->seek(0);
	bool bRet = (buf[1]==MAKEFOURCC('f','t','y','p')) ? CreateMP4Stream() : CreateAACStream() ;
	
	m_Info.SampleCount = 0;


	if (bRet)
	{

		/* convert from samples to ms */
		m_Info.SampleCount = m_AacInfo.m_Length;
		m_Info.channels = m_AacInfo.m_Channels;
		m_Info.frequency = m_AacInfo.m_SampleRate;
		m_Info.bitrate = m_AacInfo.m_BitRate;
        m_Info.bits_per_sample = 16;

		int decoder_buffer_size = 4096 * (m_Info.bits_per_sample>>3)  ;
		return CreateBuffer(decoder_buffer_size);
	}
	Close();
	return false;
}



int MUSIKAACDecoder::DecodeBlocks(unsigned char *buff,int len)
{
	
	NeAACDecFrameInfo info;
	uint8_t* pSamples = NULL;
	do 
	{
		if (m_AacInfo.m_pMP4File) 
		{
			if (m_AacInfo.sampleid >= m_AacInfo.m_NumSamples) 
			{ 
				return 0;
			}
			m_AacInfo.fbuflen += mp4ff_read_sample_v2(m_AacInfo.m_pMP4File,m_AacInfo.mp4track,m_AacInfo.sampleid++,m_AacInfo.fbuf+m_AacInfo.fbuflen);
		} 
		else 
		{
			int32_t readbytes = m_AacInfo.m_pReader->read(m_AacInfo.fbuf+m_AacInfo.fbuflen,sizeof(m_AacInfo.fbuf)-m_AacInfo.fbuflen);
			m_AacInfo.fbuflen+=readbytes;
			if (m_AacInfo.fbuflen <= 0)
				return 0;
		}
		pSamples = (uint8_t*)NeAACDecDecode(m_AacInfo.decoder,&info,m_AacInfo.fbuf,m_AacInfo.fbuflen);				
		m_AacInfo.fbuflen -= info.bytesconsumed;
		memmove(m_AacInfo.fbuf,m_AacInfo.fbuf+info.bytesconsumed,m_AacInfo.fbuflen); // shift remaining data to start of buffer
	} while (0 == info.samples);
	IncDecodeSamplePos(info.samples / info.channels);
	int bytes =info.samples * ( m_Info.bits_per_sample >> 3);
	if(pSamples)
		memcpy(buff,pSamples,bytes);
	return bytes;
}


bool MUSIKAACDecoder::DoSeek(int64_t samplepos)
{
	m_AacInfo.fbuflen=0; // reset buffer
	if (m_AacInfo.m_pMP4File) 
	{
		double seconds=(double)(samplepos)/m_AacInfo.m_SampleRate;
        int32_t toskip = 0;
		m_AacInfo.sampleid=mp4ff_find_sample(m_AacInfo.m_pMP4File,m_AacInfo.mp4track,seconds * m_AacInfo.timescale,&toskip);
		SetDecodeSamplePos(seconds * m_AacInfo.m_SampleRate);
		return true;
		
	} 
	else 
	{
		if(m_AacInfo.m_pSeektable)
		{
			uint32_t seconds = (uint32_t)samplepos/m_AacInfo.m_SampleRate;
			uint32_t fpos = m_AacInfo.m_pSeektable[seconds];
 		    if (m_AacInfo.m_pReader->seek(fpos)) 
 		    {
				SetDecodeSamplePos(seconds * m_AacInfo.m_SampleRate);
				return true;
		    }
		}	
	}
	return false;
}

bool MUSIKAACDecoder::Close()
{
	MUSIKDecoder::Close();
	m_AacInfo.Close();
	return true;
}

bool MUSIKAACDecoder::CreateAACStream()
{
	if (m_AacInfo.get_AAC_format()==-1) 
	{
		return false;
	}

	uint8_t buffer[4608];
	m_AacInfo.m_pReader->seek(m_AacInfo.m_initbytes);
	int outb,inb=m_AacInfo.m_pReader->read(buffer,sizeof(buffer));
	if ((outb=NeAACDecInit(m_AacInfo.decoder,buffer,inb,(unsigned long*)&m_AacInfo.m_SampleRate,&m_AacInfo.m_Channels))<0) 
	{
		return false;
	}

	NeAACDecConfigurationPtr config=NeAACDecGetCurrentConfiguration(m_AacInfo.decoder);
	config->downMatrix = 1;
	config->outputFormat = FAAD_FMT_16BIT;
	if (m_AacInfo.m_Channels > 2) 
		m_AacInfo.m_Channels = 2;

	m_AacInfo.m_initbytes+=outb;

	m_AacInfo.m_pReader->seek(m_AacInfo.m_initbytes);
	return true;
}

bool MUSIKAACDecoder::CreateMP4Stream()
{

	m_AacInfo.mp4cb.read=read_callback;
	m_AacInfo.mp4cb.seek=seek_callback;
	m_AacInfo.mp4cb.user_data=m_AacInfo.m_pReader;
	if (!(m_AacInfo.m_pMP4File=mp4ff_open_read(&m_AacInfo.mp4cb))
		|| (m_AacInfo.mp4track=m_AacInfo.FindAACIndex())==-1) {
		return false;
	}

	uint8_t *buffer;
	unsigned int bs;
	if (mp4ff_get_decoder_config(m_AacInfo.m_pMP4File,m_AacInfo.mp4track,&buffer,&bs)) 
	{
		return false;
	}
	if (NeAACDecInit2(m_AacInfo.decoder,buffer,bs,(unsigned long*)&m_AacInfo.m_SampleRate,&m_AacInfo.m_Channels)<0) {
		free(buffer);
		return false;
	}

	NeAACDecConfigurationPtr config=NeAACDecGetCurrentConfiguration(m_AacInfo.decoder);
	config->downMatrix = 1;
	config->outputFormat = FAAD_FMT_16BIT;
	if (m_AacInfo.m_Channels > 2) 
		m_AacInfo.m_Channels = 2;

	m_AacInfo.timescale=mp4ff_time_scale(m_AacInfo.m_pMP4File,m_AacInfo.mp4track);
	{
		int64_t duration=mp4ff_get_track_duration_use_offsets(m_AacInfo.m_pMP4File,m_AacInfo.mp4track);
		m_AacInfo.m_Length= (duration==-1) ? 0 : duration*m_AacInfo.m_SampleRate/m_AacInfo.timescale;
	}
	m_AacInfo.m_NumSamples=mp4ff_num_samples(m_AacInfo.m_pMP4File,m_AacInfo.mp4track);
	m_AacInfo.sampleid=0;
    m_AacInfo.m_BitRate = mp4ff_get_avg_bitrate(m_AacInfo.m_pMP4File,m_AacInfo.mp4track);
//	ReadMP4Meta(stream); // read tags before decoding
	free(buffer);
	return true;
}

#endif //MUSIKENGINE_NO_FLAC_SUPPORT

