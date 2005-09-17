//Copyright (c) 2005, Gunnar Roth
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

#include "MUSIKEngine/XineEngine/inc/xineengine.h"
#include "MUSIKEngine/MUSIKEngine/inc/defaultdecoder.h"
#include "xinestreamout.h"


#ifdef _WIN32
#define snprintf _snprintf
#endif
#include <stdio.h>


XineEngine::XineEngine()
{
    m_pXine = NULL;
    int major, minor, sub;
    xine_get_version (&major, &minor, &sub);
    snprintf(m_szVersion,sizeof(m_szVersion)/sizeof(m_szVersion[0]) - 1,"%x.%x.%x",major,minor,sub);
    m_bValid = false;
}

MUSIKEngine::Error XineEngine::_Init(int idOutput ,int idDevice,int nMixRate,int nMaxChannels)
{
    m_pXine = xine_new();
    if(!m_pXine)
        return errUnknown;
    xine_init( m_pXine );

    m_pAudioPort = xine_open_audio_driver( m_pXine, NULL, NULL );
    if(!m_pAudioPort)
        return errOutputInitFailed;

//    if(idDevice > 0)
//    {
//        //---------------------//
//        //--- setup device	---//
//        //---------------------//
//        if (  m_pSystem->setDriver( idDevice )!= FMOD_OK )
//            return errDeviceInitFailed;
//    }

    return errSuccess;
}
void XineEngine::SetBufferMs(int nSndBufferMs)
{
    MUSIKEngine::SetBufferMs(nSndBufferMs);
    //m_pSystem->setStreamBufferSize(nSndBufferMs, FMOD_TIMEUNIT_MS);
}
MUSIKEngine::Error XineEngine::Init(int idOutput ,int idDevice ,int nMixRate ,int nMaxChannels)
{
    if( m_pXine )
    {
        if( m_pAudioPort )  
            xine_close_audio_driver( m_pXine, m_pAudioPort );
        xine_exit( m_pXine );
    }
    Error e = _Init(idOutput,idDevice,nMixRate,nMaxChannels);
    m_bValid = (e == errSuccess);
    return e;
}
MUSIKEngine::Error XineEngine::EnumDevices(MUSIKEngine::IEnumNames * pen) const
{
    if(!m_bValid)
        return errUnknown;
    int numDrivers = 1;
    char *name ="default";
    for ( int i = 0; i < numDrivers ; i++ )
    {
        pen->EnumNamesCallback(name,i);
    }
    return errSuccess;
}
MUSIKEngine::Error XineEngine::EnumOutputs(IEnumNames * pen) const
{
    if(!m_bValid)
        return errUnknown;
    static const char * szData[] =
    {
#if defined(WIN32)
        "Direct Sound"
#elif defined(__linux)
        "OSS",
        "ALSA",
#elif defined( __apple__)
        "CoreAudio",
        "SoundManager"
#else 
    #error System not supported
#endif
    };
    for ( size_t i = 0; i < sizeof(szData)/sizeof(szData[0]); i++ )
    {
        pen->EnumNamesCallback(szData[i],(int)i);
    }
    return errSuccess;

}

MUSIKEngine::Error XineEngine::EnumFrequencies(IEnumNames * pen) const
{
    static const char * szData[] =
    {
        "Default"
//        "48000",
//        "44100",
//        "22050",
//        "11025",
//        "8000"
    };   
    for ( size_t i = 0; i < sizeof(szData)/sizeof(szData[0]); i++ )
    {
        pen->EnumNamesCallback(szData[i],(int)i);
    }
    return errSuccess;

}

MUSIKEngine::Error XineEngine::SetNetworkProxy(const char * s)
{
  return /*m_pSystem->setProxy(s) == FMOD_OK ?*/ errSuccess/*:errUnknown*/;
}

MUSIKEngine::Error XineEngine::SetNetBuffer(int nBufferSize,int nPreBufferPercent,int nReBufferPercent)
{
    MUSIKEngine::SetNetBuffer(nBufferSize,nPreBufferPercent,nReBufferPercent);
    return errSuccess;
}


MUSIKDefaultDecoder *XineEngine::CreateDefaultDecoder()
{
	return  new	MUSIKDefaultDecoder(new XineStreamOut(*this));
}
const char *XineEngine::ErrorString()
{
   // int errcode = FSOUND_GetError();
	return "unkown error";
}

void XineEngine::SetVolume(float v)
{ 
    MUSIKEngine::SetVolume(v);
    std::set<IMUSIKStreamOut *> ::iterator it;
    for(it = m_setRegisteredStreamOuts.begin();it != m_setRegisteredStreamOuts.end();it++)
    { 
        float curvol = (*it)->GetVolume();
        (*it)->SetVolume(curvol * GetVolume());
    }
}


bool XineEngine::SetPlayState( MUSIKEngine::PlayState state)
{
    std::set<IMUSIKStreamOut *> ::iterator it;
    for(it = m_setRegisteredStreamOuts.begin();it != m_setRegisteredStreamOuts.end();it++)
    {
        (*it)->SetPlayState(state);
    }
	return true;
}

const char * XineEngine::Version() const
{
    return m_szVersion;
}

xine_stream_t *XineEngine::CreateXineStream()
{
    return m_pXine ? xine_stream_new( m_pXine, m_pAudioPort, NULL ):NULL;
}

XineEngine::~XineEngine(void)
{
    if( m_pXine )
    {
        if( m_pAudioPort )  
            xine_close_audio_driver( m_pXine, m_pAudioPort );
        xine_exit( m_pXine );
    }
}
