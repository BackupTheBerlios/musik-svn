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

#ifndef IMUSIKSTREAMOUT_H
#define IMUSIKSTREAMOUT_H
#include "MUSIKEngine/inc/decoder.h"

class IMUSIKStreamOut
{
public:
	IMUSIKStreamOut()
	{
		m_pMUSIKDecoder = NULL;
	}

	virtual ~IMUSIKStreamOut()
	{
	}
	virtual bool Create(MUSIKDecoder * pYou,int buffersize)
	{
		m_pMUSIKDecoder = pYou;
		return DoCreate(buffersize);
	}
	virtual bool Start()=0;
	virtual void SetVolume(float v)=0;// range is 0.0 to 1.0
	virtual float GetVolume()=0;
	virtual bool SetPlayState( MUSIKEngine::PlayState state)=0;
	virtual MUSIKEngine::PlayState GetPlayState()=0;
	virtual bool Close()=0;

	// ugly HACK
	virtual void * STREAM() {return NULL;}

protected:
	virtual	bool FillBuffer(unsigned char * buff,int len)
	{
		return Decoder()->DoFillBuffer(buff,len);
	}
	virtual bool DoCreate(int buffersize)=0;

	MUSIKDecoder * Decoder()
	{
		return m_pMUSIKDecoder;
	}
private:
	MUSIKDecoder * m_pMUSIKDecoder; 
};

class IMUSIKStreamOutDefault  : public IMUSIKStreamOut
{
public:
	// default methods
	virtual bool Open(const char *FileName)=0;
	virtual bool CanSeek()=0;
	virtual bool SetTime( int nTimeMS)=0;
	virtual int GetTime()=0;
	virtual int GetLengthMs()=0;
	virtual int GetLength()=0;
	virtual const char * Type()=0;
};
#endif
