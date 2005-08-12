//Copyright (c) 2004, Gunnar Roth
//All rights reserved.
//
//Redistribution and use in source and binary forms, with or without modification, are permitted provided 
//that the following conditions are met:
//
//    * Redistributions of source code must retain the above copyright notice, this list of conditions and 
//      the following disclaimer.
//    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions 
//      and the following disclaimer in the documentation and/or other materials provided with the distribution.
//    * Neither the name of the Gunnar Roth nor the names of its contributors may be used to endorse or 
//      promote products derived from this software without specific prior written permission.
//
//THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
//INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
//DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
//SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
//SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
//WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
//OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef MUSIKENGINE_NO_MPC_SUPPORT

#include "mpcdecoder.h"
#include "MUSIKEngine/inc/imusikstreamout.h"

#ifdef MP3DEC_NEW
#ifdef MUSIKENGINE_USE_LIBMUSEPACK_103
MUSIKMPCDecoder::MUSIKMPCDecoder(IMUSIKStreamOut * pIMUSIKStreamOut)
:MUSIKDecoder(pIMUSIKStreamOut)
{
	memset(&m_MPCInfo,0,sizeof(m_MPCInfo));
	m_MPCInfo.Frames = -1;
}

MUSIKMPCDecoder::~MUSIKMPCDecoder()
{
	delete m_MPCInfo.Decoder;
	delete m_MPCInfo.Reader;
}

bool MUSIKMPCDecoder::DoSeek( int64_t samplepos)
{

	int done = 0;
	if ( m_MPCInfo.Decoder  ) 
    {// only perform jump when bitstream is still allocated
		if ( m_MPCInfo.Decoder->SeekSample(samplepos) ) 
        {
            SetDecodeSamplePos(samplepos);
            return true;
		}
	} 
	return false;
}
int MUSIKMPCDecoder::DecodeBlocks(unsigned char *buff,int len)
{


	int bytes = 0;
	unsigned valid_samples = m_MPCInfo.Decoder->Decode(m_MPCInfo.reservoir_);
	if (valid_samples == (unsigned)(-1))
	{
		//decode error
		return -1;		
	}
	else if ( valid_samples > 0 )
	{
		bytes = CopySamplesToBuffer(m_MPCInfo.reservoir_,valid_samples * /* stereo */ 2,buff);
	}
	IncDecodePos((valid_samples*1000 +  m_Info.frequency/2)/ m_Info.frequency);
	return bytes;

}



bool MUSIKMPCDecoder::OpenMedia(const char *FileName)
{


	FILE *fp = fopen ( FileName, "rb" );
	if (fp) 
		m_MPCInfo.Reader = new MPC_reader_impl (fp,true); 
	else 
		return false;
	if (m_MPCInfo.Info.ReadStreamInfo(m_MPCInfo.Reader) != ERROR_CODE_OK)
	{
		Close();
		return false;
	}
	
	m_MPCInfo.Decoder = new MPC_decoder(m_MPCInfo.Reader);
	

	
	if (!m_MPCInfo.Decoder->Initialize(&m_MPCInfo.Info))
	{
		Close();
		return false;
	}
	m_Info.channels	 = m_MPCInfo.Info.simple.Channels;
	m_Info.frequency   = (int) m_MPCInfo.Info.simple.SampleFreq;
	m_Info.FileSize = m_MPCInfo.Info.simple.TotalFileLength;
	m_Info.SampleCount = m_MPCInfo.Info.GetLengthSamples();


	m_Info.bitrate	 = (int) m_MPCInfo.Info.simple.AverageBitrate;
	m_Info.bits_per_sample  = 16;
    return CreateBuffer(MPC_decoder::DecodeBufferLength *2);
}

bool MUSIKMPCDecoder::Close()
{    
	return MUSIKDecoder::Close();
}

#else // MUSIKENGINE_USE_LIBMUSEPACK_103

/*
  Our implementations of the mpc_reader callback functions.
*/

mpc_int32_t
MUSIKMPCDecoder::MPCStream::read_impl(void *data, void *ptr, mpc_int32_t size)
{
    reader_data *d = (reader_data *) data;
    return fread(ptr, 1, size, d->file);
}

mpc_bool_t
MUSIKMPCDecoder::MPCStream::seek_impl(void *data, mpc_int32_t offset)
{
    reader_data *d = (reader_data *) data;
    return d->seekable ? !fseek(d->file, offset, SEEK_SET) : false;
}

mpc_int32_t
MUSIKMPCDecoder::MPCStream::tell_impl(void *data)
{
    reader_data *d = (reader_data *) data;
    return ftell(d->file);
}

mpc_int32_t
MUSIKMPCDecoder::MPCStream::get_size_impl(void *data)
{
    reader_data *d = (reader_data *) data;
    return d->size;
}

mpc_bool_t
MUSIKMPCDecoder::MPCStream::canseek_impl(void *data)
{
    reader_data *d = (reader_data *) data;
    return d->seekable;
}
MUSIKMPCDecoder::MPCStream::MPCStream()
{
    memset(&data,0,sizeof(data));
    memset(sample_buffer,0,sizeof(sample_buffer));
    // set reader functions
    reader.read = read_impl;
    reader.seek = seek_impl;
    reader.tell = tell_impl;
    reader.get_size = get_size_impl;
    reader.canseek = canseek_impl;
    reader.data = &data;

    mpc_streaminfo_init(&info);
    mpc_decoder_setup(&decoder, &reader);    
}

bool MUSIKMPCDecoder::MPCStream::Init(FILE * input)
{
    data.file = input;
    data.seekable = true;
    fseek(data.file, 0, SEEK_END);
    data.size = ftell(data.file);
    fseek(data.file, 0, SEEK_SET);
    if( mpc_streaminfo_read(&info, &reader) != ERROR_CODE_OK )
        return false;
    mpc_decoder_setup(&decoder, &reader);
    return mpc_decoder_initialize(&decoder, &info);
}

void MUSIKMPCDecoder::MPCStream::Close()
{
    if(data.file != NULL)
        fclose(data.file);
}

MUSIKMPCDecoder::MUSIKMPCDecoder(IMUSIKStreamOut * pIMUSIKStreamOut)
:MUSIKDecoder(pIMUSIKStreamOut)
{
}

MUSIKMPCDecoder::~MUSIKMPCDecoder()
{
}

bool MUSIKMPCDecoder::DoSeek(int64_t samplepos)
{	
    bool bRes = m_MPCStream.SeekSample(samplepos); 
    if( bRes )
        SetDecodeSamplePos(samplepos);
	return bRes;
}
int MUSIKMPCDecoder::DecodeBlocks(unsigned char *buff,int len)
{
	int bytes = 0;
	unsigned valid_samples = m_MPCStream.Decode();
	if (valid_samples == (unsigned)(-1))
	{
		//decode error
		return -1;		
	}
	else if ( valid_samples > 0 )
	{
		bytes = CopySamplesToBuffer(m_MPCStream.sample_buffer,valid_samples * m_Info.channels,buff);
	}
	IncDecodeSamplePos(valid_samples);
	return bytes;

}



bool MUSIKMPCDecoder::OpenMedia(const char *FileName)
{


	FILE *fp = fopen ( FileName, "rb" );
	if (fp) 
    {
		if(!m_MPCStream.Init(fp))
        {
            Close();
            return false;
        }
    }
	else 
		return false;
 	m_Info.channels	 = m_MPCStream.info.channels;
	m_Info.frequency = (int) m_MPCStream.info.sample_freq;
	m_Info.FileSize  = m_MPCStream.info.total_file_length;
	m_Info.SampleCount  = m_MPCStream.info.pcm_samples;
	m_Info.bitrate	 = (int) m_MPCStream.info.average_bitrate;
	m_Info.bits_per_sample  = 16;
    return CreateBuffer(MPC_DECODER_BUFFER_LENGTH *sizeof(MPC_SAMPLE_FORMAT));
}


bool MUSIKMPCDecoder::Close()
{
	bool res = MUSIKDecoder::Close();
    m_MPCStream.Close();
    return res;
}


#endif// else MUSIKENGINE_USE_LIBMUSEPACK_103

#ifdef MPC_FIXED_POINT
static int shift_signed(MPC_SAMPLE_FORMAT val,int shift)
{
	if (shift>0) val <<= shift;
	else if (shift<0) val >>= -shift;
	return (int)val;
}
#endif


int MUSIKMPCDecoder::CopySamplesToBuffer(const MPC_SAMPLE_FORMAT * p_buffer,unsigned p_size,unsigned char *pDestbuf)
{
	unsigned n;
	int clip_min = - 1<<(m_Info.bits_per_sample-1),
		clip_max = (1<<(m_Info.bits_per_sample-1)) - 1,
		float_scale = 1<<(m_Info.bits_per_sample-1);
	short *pBuff = (short*)pDestbuf;
	for(n=0;n<p_size;n++)
	{
		int val;
#ifdef MPC_FIXED_POINT
		val = shift_signed( p_buffer[n], m_Info.bits_per_sample - MPC_FIXED_POINT_SCALE_SHIFT ) ;
#else
		val = (int)( p_buffer[n] * float_scale );
#endif
		if (val<clip_min) val = clip_min;
		else if (val>clip_max) val = clip_max;
		pBuff[n]= (val & 0xFFFF);
	}
	return p_size * (m_Info.bits_per_sample >> 3);
}











#else // MP3DEC_NEW

#include "MPC\in_mpc.h"


#define FRAMELEN        (36 * 32)

#define MPC_BUFFER_SIZE (FRAMELEN * 2 * 4  * 2)

#ifdef __VISUALC__
#pragma comment(lib,"mpclib")
#endif


MUSIKMPCDecoder::MUSIKMPCDecoder(IMUSIKStreamOut * pIMUSIKStreamOut)
:MUSIKDecoder(pIMUSIKStreamOut)
{
	memset(&m_MPCInfo,0,sizeof(m_MPCInfo));
	m_MPCInfo.Frames = -1;
}

MUSIKMPCDecoder::~MUSIKMPCDecoder()
{
	delete m_MPCInfo.Decoder;
}

bool MUSIKMPCDecoder::DoSeek(int64_t samplepos)
{

	
	if ( m_MPCInfo.Decoder  ) 
    {// only perform jump when bitstream is still allocated
		m_MPCInfo.Decoder->SetPrebuf(0);
        int nTimeMS = (int)(samplepos/m_MPCInfo.Decoder->sfreq_khz);
        int done = 1;
		if (m_MPCInfo.Decoder->perform_jump (&done, &m_MPCInfo.Frames, nTimeMS) ) 
        {
            SetDecodeSamplePos(samplepos);
            m_MPCInfo.Decoder->SetPrebuf(1);
            m_MPCInfo.Decoder->DoPrebuf();
            return true;
		}
	}
	return (false);
}
int MUSIKMPCDecoder::DecodeBlocks(unsigned char *buff,int len)
{

	MPC_decoder *curDecoder =m_MPCInfo.Decoder;
	const int MaxBrokenFrames = 0;

	unsigned int RING = curDecoder->Zaehler;
	int vbr_ring = (RING << 5) + curDecoder->pos;
	int valid_samples = curDecoder->DecodeFrame((char*) buff );	

	// ------------------ ERROR CONCEALMENT ---------------	
	if ( curDecoder->FrameWasValid == 0 )
	{
		// error occurred in bitstream
		// one more invalid frame
		++m_MPCInfo.NumberOfConsecutiveBrokenFrames;                          
		if ( m_MPCInfo.NumberOfConsecutiveBrokenFrames > MaxBrokenFrames )
		{
			// too many broken frames -> cancel decoding
			valid_samples = 0;
		} 
		else 
		{
			// conceal error -> send zeroes, try to re-sync
			curDecoder->Zaehler   = curDecoder->RecommendedResyncPos >> 5;
			curDecoder->pos       = curDecoder->RecommendedResyncPos & 31;
			// set current decoded word
			curDecoder->dword     = curDecoder->Speicher [curDecoder->Zaehler];
			// filling broken frames with zeroes
			valid_samples     = FRAMELEN; // 4608
			memset (buff, 0, len);
		}
	} 
	else 
	{
		m_MPCInfo.NumberOfConsecutiveBrokenFrames = 0;
	}
	// ----------------------------------------------------

	curDecoder->UpdateBuffer(RING);
	++   m_MPCInfo.Frames;

	IncDecodeSamplePos(valid_samples);
	return valid_samples * 4;;

}



bool MUSIKMPCDecoder::OpenMedia(const char *FileName)
{
	m_MPCInfo.Info.Clear();
	m_MPCInfo.Info.SetFilename(FileName);


	FILE *fp = fopen ( FileName, "rb" );
	if (fp) 
		m_MPCInfo.Reader = new Reader_file (fp); 
	else 
		return false;
	if (m_MPCInfo.Info.ReadStreamInfo(m_MPCInfo.Reader) != ERROR_CODE_OK)
	{
		Close();
		return false;
	}
	m_MPCInfo.Reader->seek(0l,SEEK_SET);
	m_MPCInfo.Decoder = new MPC_decoder(m_MPCInfo.Reader);
	m_MPCInfo.Decoder->Reset_Globals();
	m_MPCInfo.Decoder->SetStreamInfo(&m_MPCInfo.Info);

	m_MPCInfo.Decoder->ScaleOutput ( ProcessReplayGain (PluginSettings.ReplayGainMode, &m_MPCInfo.Info) );

	if (!m_MPCInfo.Decoder->FileInit())
	{
		Close();
		return false;
	}
	m_MPCInfo.Decoder->SetPrebuf(1);
	m_MPCInfo.Decoder->DoPrebuf();
	m_Info.channels	 = m_MPCInfo.Info.simple.Channels;
	m_Info.frequency   = (int) m_MPCInfo.Info.simple.SampleFreq;
	m_Info.FileSize = m_MPCInfo.Info.simple.TotalFileLength;
	m_Info.SampleCount = m_MPCInfo.Info.simple.PCMSamples;


	m_Info.bitrate	 = (int) m_MPCInfo.Info.simple.AverageBitrate;
	m_Info.bits_per_sample  = 16;
	return CreateBuffer(MPC_BUFFER_SIZE);
}

bool MUSIKMPCDecoder::Close()
{
	return MUSIKDecoder::Close();
}

#endif // else // MP3DEC_NEW

#endif //MUSIKENGINE_NO_MPC_SUPPORT
