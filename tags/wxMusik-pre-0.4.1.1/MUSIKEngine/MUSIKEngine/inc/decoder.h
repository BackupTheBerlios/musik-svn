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

#ifndef MUSIKDECODER_H
#define MUSIKDECODER_H
#include <string.h>
#include <memory.h>
#include <stdlib.h>
#include "engine.h"

class IMUSIKStreamOut;
#if _WIN32
typedef __int64 int64_t;
#endif
class   MUSIKDecoder
{
public:
	struct INFO
	{
		int			frequency;
		int			channels;
		int			bitrate;
		int			bits_per_sample;
		int64_t			FileSize;
		int			LengthMS;
	};

protected:
	MUSIKDecoder(IMUSIKStreamOut * pIMUSIKStreamOut)
	{
		memset(&m_Info,0,sizeof(m_Info));
		m_seek_needed = -1;
		m_pIMUSIKStreamOut = pIMUSIKStreamOut;
		m_sample_buffer = NULL;
		m_sample_buffer_filled = 0;
		m_sample_buffer_size = 0;
		m_decode_pos_ms = 0;
	}
public:

	virtual ~MUSIKDecoder();


	virtual bool Start();
	virtual int GetTime()
	{
		if(m_seek_needed != -1) // if an seek is in progress, return the expected position.
			return m_seek_needed;
		return (m_decode_pos_ms);// return the current position
	}

	virtual int GetLengthMs()
	{
		return m_Info.LengthMS;
	}
	virtual int64_t GetLength()
	{
		return m_Info.FileSize;
	}
	virtual bool CanSeek() = 0;
	virtual bool SetTime( int nTimeMS)
	{
		if(false == CanSeek())
			return false;
		m_seek_needed = nTimeMS;
		return true;
	}

	virtual INFO *GetInfo()
	{
		return & m_Info;
	}
	virtual bool Close();

	virtual const char * Type() = 0;

	IMUSIKStreamOut *  StreamOut()
	{
		return m_pIMUSIKStreamOut;
	}

	bool DoFillBuffer(unsigned char * buff,int len);

protected:
	virtual bool OpenMedia(const char *FileName)=0;
	virtual int DecodeBlocks(unsigned char *buff,int len)=0;
	virtual bool DoSeek(int nTimeMS)=0;
    bool SeekIfNeeded();
	void SetDecodePos(int pos)
	{
		m_decode_pos_ms = pos;
	}
	void IncDecodePos(int pos)
	{
		m_decode_pos_ms += pos;
	}

	bool CreateBuffer(int decoderbuffersize,int streambuffersize = -1);

	INFO m_Info;



private:

	unsigned char*	m_sample_buffer;
	int				m_sample_buffer_size;
	int				m_sample_buffer_filled;
	int				m_seek_needed;

	int				m_decode_pos_ms;
	int				m_streambuffersize;
	IMUSIKStreamOut * m_pIMUSIKStreamOut;
	friend class MUSIKEngine;
};


#endif
