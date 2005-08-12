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

#ifndef MUSIKENGINE_H
#define MUSIKENGINE_H

#include <string.h>

class MUSIKStream;
class MUSIKDefaultDecoder;
class IMUSIKStreamOut;

class MUSIKEngine
{
public:
	enum PlayState
	{
		Invalid = 0,
		Paused = 1,
		Stopped = 2,
		Playing=3
	};
    enum Error
    {
        errSuccess = 0,
        errUnknown,
        errNotSupported,
        errOutputInitFailed,
        errDeviceInitFailed,
        errInvalidArg
    };
    enum NetStatus
    {
        NETSTATUS_INVALID = -1,
        NETSTATUS_NOTCONNECTED = 0,
        NETSTATUS_CONNECTING,
        NETSTATUS_BUFFERING,
        NETSTATUS_READY,
        NETSTATUS_ERROR
    };
    enum OpenStatus
    {
        OPENSTATUS_READY = 0,
        OPENSTATUS_OPENFAILED,
        OPENSTATUS_OPENINPROGRESS
    };
    struct IEnumNames
    {
        virtual bool EnumNamesCallback(const char * szName,int id)=0;
    };
public: 
	MUSIKEngine()
        :m_nSndBufferMs(400)
        ,m_NetBufferSize(64*1024)
	{
	}
    virtual Error Init(int idOutput = -1 ,int idDevice = -1,int nMixRate = 48000,int nMaxChannels = 4) = 0;
    virtual Error SetProxy(const char * s){s; return errNotSupported;}
    virtual Error SetNetBuffer(int nBufferSize,int nPreBufferPercent,int nReBufferPercent) 
        {m_NetBufferSize = nBufferSize;nPreBufferPercent,nReBufferPercent; return errNotSupported;}
    virtual Error EnumDevices(IEnumNames * pen) const = 0;
    virtual Error EnumOutputs(IEnumNames * pen) const = 0;
    virtual Error EnumFrequencies(IEnumNames * pen) const = 0;

    MUSIKStream * OpenMedia(const char *FileName);
	virtual void SetVolume(float){}
	virtual float GetVolume() const {return 1.0;}
	virtual bool SetPlayState( MUSIKEngine::PlayState state)=0;
    virtual const char * Version() const =0;
    virtual bool IsValid() const {return true;}
    virtual void SetBufferMs(int nSndBufferMs = 400)
    {
       m_nSndBufferMs = nSndBufferMs;
    }
    virtual int GetBufferMs() const
    {
        return m_nSndBufferMs;
    }
    virtual int GetNetBufferSize() const
    {
        return m_NetBufferSize;
    }

	virtual ~MUSIKEngine()
	{
	}
protected:
	virtual IMUSIKStreamOut *CreateStreamOut() = 0;
	virtual MUSIKDefaultDecoder *CreateDefaultDecoder()
	{
		return NULL;
	}
private:
    int m_nSndBufferMs;
    int m_NetBufferSize;
};

#endif
