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

#ifndef MUSIKSTREAM_H
#define MUSIKSTREAM_H
#include "MUSIKEngine/inc/decoder.h"
class MUSIKStream 
{

	MUSIKStream(MUSIKDecoder *pDecoder);
public:

	virtual ~MUSIKStream();

	virtual bool Play();
	virtual void SetVolume(float v);// range is 0.0 to 1.0
	virtual float GetVolume();
	virtual bool CanSeek();
	virtual bool SetTime( int nTimeMS);
	virtual int GetTime();
	virtual int GetLengthMs();
	virtual int64_t GetLength();
	virtual const char * Type();
	virtual bool SetPlayState( MUSIKEngine::PlayState state);
	virtual MUSIKEngine::PlayState GetPlayState();
	MUSIKDecoder::INFO * GetDecoderInfo();

	// thisis a HACK
	IMUSIKStreamOut *	GetStreamOut() {return  m_pDecoder->StreamOut();}

protected:
	MUSIKDecoder *m_pDecoder;	

friend class MUSIKEngine;
};

#endif