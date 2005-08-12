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
    struct IMetadataCallback
    {
        virtual void MetadataCallback(const char *name, const char *value) = 0;
    };

	IMUSIKStreamOut(MUSIKEngine & e)
        :m_Engine(e)
        ,m_pMUSIKDecoder(NULL)
        ,m_pIMetadataCallback(NULL)
	{
	}

	virtual ~IMUSIKStreamOut()
	{
        delete m_pIMetadataCallback;
	}
	virtual bool Create(MUSIKDecoder * pYou)
	{
		m_pMUSIKDecoder = pYou;
		return DoCreate(m_Engine.GetBufferMs());
	}
	virtual bool Start()=0;
	virtual void SetVolume(float v)=0;// range is 0.0 to 1.0
	virtual float GetVolume()=0;
	virtual bool SetPlayState( MUSIKEngine::PlayState state)=0;
	virtual MUSIKEngine::PlayState GetPlayState()=0;
	virtual bool Close()=0;
    virtual MUSIKEngine::Error GetOpenStatus(MUSIKEngine::OpenStatus *pStatus)
        {pStatus; return MUSIKEngine::errNotSupported;}
    virtual MUSIKEngine::Error SetMetadataCallback(IMUSIKStreamOut::IMetadataCallback *pCb)
    {
        IMetadataCallback *pOldCb = m_pIMetadataCallback;
        m_pIMetadataCallback = pCb;
        delete pOldCb;
        return MUSIKEngine::errSuccess;
    }
    virtual MUSIKEngine::Error GetNetStatus(MUSIKEngine::NetStatus *pStatus,int * pnPercentRead,int * pnBitrate)
        {pStatus,pnPercentRead,pnBitrate; return MUSIKEngine::errNotSupported;}

protected:
	virtual	bool FillBuffer(unsigned char * buff,int len)
	{
		return Decoder()->DoFillBuffer(buff,len);
	}
	virtual bool DoCreate(int buffersize_ms)=0;

	MUSIKDecoder * Decoder()
	{
		return m_pMUSIKDecoder;
	}
    IMetadataCallback *m_pIMetadataCallback;
private:
    MUSIKEngine & m_Engine;
	MUSIKDecoder * m_pMUSIKDecoder; 

};

class IMUSIKStreamOutDefault  : public IMUSIKStreamOut
{
public:
    IMUSIKStreamOutDefault(MUSIKEngine & e)
        :IMUSIKStreamOut(e)
    {}
    // default methods
	virtual bool Open(const char *FileName)=0;
	virtual bool CanSeek()=0;
    virtual bool SetSamplePos( int64_t samplepos)=0;
	virtual bool SetTime( int64_t nTimeMS)=0;
    virtual int64_t GetSamplePos()=0;
	virtual int64_t GetTime()=0;
	virtual int64_t GetLengthMs()=0;
	virtual int64_t GetSampleCount()=0;
    virtual int64_t GetFilesize()=0;
	virtual const char * Type()=0;
};
#endif
