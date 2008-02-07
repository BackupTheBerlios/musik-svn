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

#include "AudioQueueEngine/inc/audioqueueengine.h"
#include "MUSIKEngine/inc/defaultdecoder.h"
#include "audioqueuestreamout.h"
#include <stdio.h>
#include <AudioToolbox/AudioQueue.h>
#include <AudioToolbox/AudioFile.h>

// helpers
#include "CAXException.h"
#include "CAStreamBasicDescription.h"

static const int kNumberBuffers = 3;


AudioQueueEngine::AudioQueueEngine()
{
    m_OpenMode = OpenMode_Default;
    m_bValid = false;
}

MUSIKEngine::Error AudioQueueEngine::_Init(int idOutput ,int idDevice,int nMixRate,int nMaxChannels)
{
    return errSuccess;
}
void AudioQueueEngine::SetBufferMs(int nSndBufferMs)
{
    MUSIKEngine::SetBufferMs(nSndBufferMs);
}
MUSIKEngine::Error AudioQueueEngine::Init(int idOutput ,int idDevice ,int nMixRate ,int nMaxChannels)
{
    Error e = _Init(idOutput,idDevice,nMixRate,nMaxChannels);
    m_bValid = (e == errSuccess);
    return e;
}
MUSIKEngine::Error AudioQueueEngine::EnumDevices(MUSIKEngine::IEnumNames * pen) const
{
    char name[100];
	strcpy(name,"Default");
    pen->EnumNamesCallback(name,0);
    return errSuccess;
}
MUSIKEngine::Error AudioQueueEngine::EnumOutputs(IEnumNames * pen) const
{
    static const char * szData[] =
    {
        "CoreAudio"
    };
    for ( size_t i = 0; i < sizeof(szData)/sizeof(szData[0]); i++ )
    {
        pen->EnumNamesCallback(szData[i],(int)i);
    }
    return errSuccess;

}

MUSIKEngine::Error AudioQueueEngine::EnumFrequencies(IEnumNames * pen) const
{
    static const char * szData[] =
    {
        "Default"
/*        ,
        "48000",
        "44100",
        "22050",
        "11025",
        "8000"
*/    };   
    for ( size_t i = 0; i < sizeof(szData)/sizeof(szData[0]); i++ )
    {
        pen->EnumNamesCallback(szData[i],(int)i);
    }
    return errSuccess;

}

MUSIKEngine::Error AudioQueueEngine::SetNetworkProxy(const char * s)
{
  return  errSuccess;
}

MUSIKEngine::Error AudioQueueEngine::SetNetBuffer(int nBufferSize,int nPreBufferPercent,int nReBufferPercent)
{
    MUSIKEngine::SetNetBuffer(nBufferSize,nPreBufferPercent,nReBufferPercent);
    return errSuccess;
}

IMUSIKStreamOut *AudioQueueEngine::CreateStreamOut()
{
  return new AudioQueueStreamOut(*this);
}
MUSIKDefaultDecoder *AudioQueueEngine::CreateDefaultDecoder()
{
	return  new	MUSIKDefaultDecoder(new AudioQueueStreamOut(*this));
}
const char *AudioQueueEngine::ErrorString()
{
	return "unkown error";
}

void AudioQueueEngine::SetVolume(float v)
{ 
    MUSIKEngine::SetVolume(v);
    std::set<IMUSIKStreamOut *> ::iterator it;
    for(it = m_setRegisteredStreamOuts.begin();it != m_setRegisteredStreamOuts.end();it++)
    { 
        float curvol = (*it)->GetVolume();
        (*it)->SetVolume(curvol * GetVolume());
    }
    
}


bool AudioQueueEngine::SetPlayState( MUSIKEngine::PlayState state)
{
    std::set<IMUSIKStreamOut *> ::iterator it;
    for(it = m_setRegisteredStreamOuts.begin();it != m_setRegisteredStreamOuts.end();it++)
    {
        (*it)->SetPlayState(state);
    }
	return true;
}

const char * AudioQueueEngine::Version() const
{
    return "1.0";
}

AudioQueueEngine::~AudioQueueEngine(void)
{
}
