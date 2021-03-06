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

#ifndef FMODENGINE_H
#define FMODENGINE_H


#include "MUSIKEngine/inc/engine.h"
#include "MUSIKEngine/inc/stream.h"
#include "MUSIKEngine/inc/decoder.h"
class MUSIKDefaultDecoder;
class FMODExStreamOut;
namespace FMOD
{
    class System;
};

class FMODExEngine :public MUSIKEngine
{

public: 
	enum eOpenMode
	{ 
		OpenMode_Default = 0,
		OpenMode_MPEGACCURATE = 1
	};
	FMODExEngine();
    MUSIKEngine::Error Init(int idOutput = -1 ,int idDevice = -1,int nMixRate = 44100,int nMaxChannels = 4);
    virtual Error SetNetworkProxy(const char * s);
    virtual Error SetNetBuffer(int nBufferSize,int nPreBufferPercent,int nReBufferPercent);
    virtual Error EnumDevices(IEnumNames * pen) const;
    virtual Error EnumOutputs(IEnumNames * pen) const;
    virtual Error EnumFrequencies(IEnumNames * pen) const;

	void SetOpenMode(FMODExEngine::eOpenMode m){m_OpenMode = m;}
    eOpenMode GetOpenMode() const {return m_OpenMode;}
	virtual void SetVolume(float v);
	virtual float GetVolume() const;
	virtual bool SetPlayState( MUSIKEngine::PlayState state);
    virtual const char * Version() const;
    virtual bool IsValid() const{return m_bValid;}
	static const char *ErrorString();
    virtual void SetBufferMs(int nSndBufferMs = 400);
	virtual MUSIKEqualizer * Equalizer()
	{
		return  m_pEq;
	}
    FMOD::System & System()
    {
        return *m_pSystem;
    }
	~FMODExEngine();
protected:
    MUSIKEngine::Error _Init(int idOutput = -1 ,int idDevice = -1,int nMixRate = 44100,int nMaxChannels = 4);
	virtual MUSIKDefaultDecoder *CreateDefaultDecoder();
	virtual IMUSIKStreamOut *CreateStreamOut();
	eOpenMode m_OpenMode;
private:
	MUSIKEqualizer * m_pEq;
    bool m_bValid;
    char m_szVersion[20];
    FMOD::System *m_pSystem;

};
#ifdef __VISUALC__
#if defined _AMD64_
#pragma comment(lib,"fmodex64_vc")
#else
#pragma comment(lib,"fmodex_vc")
#endif
#pragma comment(lib,"fmodexengine")
#endif //__VISUALC__
#endif



