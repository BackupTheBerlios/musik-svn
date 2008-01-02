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

#include "MUSIKEngine/FMODExEngine/inc/fmodexengine.h"
#include "MUSIKEngine/MUSIKEngine/inc/defaultdecoder.h"
#include "fmodexstreamout.h"
#ifdef WIN32
#define snprintf _snprintf
#endif
#include <fmodex/fmod.hpp>
#include <fmodex/fmod_errors.h>
#include <stdio.h>


FMODExEngine::FMODExEngine()
{
    m_OpenMode = OpenMode_Default;
    unsigned int version;
    FMOD_RESULT result = FMOD::System_Create(&m_pSystem);
    result = m_pSystem->getVersion(&version);
    snprintf(m_szVersion,sizeof(m_szVersion)/sizeof(m_szVersion[0]) - 1,"%x.%x.%x",version>>16,version&0xff00,version&0xff);
    m_bValid = false;
}

MUSIKEngine::Error FMODExEngine::_Init(int idOutput ,int idDevice,int nMixRate,int nMaxChannels)
{
    if(!m_pSystem)
        return errUnknown;
    //-----------------//
    //--- windows	---//
    //-----------------//
#if defined(WIN32)
    if ( idOutput == 0 )
    {
        if( m_pSystem->setOutput( FMOD_OUTPUTTYPE_DSOUND ) != FMOD_OK )
            return errOutputInitFailed;
    }
    else if ( idOutput == 1 )
    {
        if ( m_pSystem->setOutput( FMOD_OUTPUTTYPE_WINMM ) != FMOD_OK )
            return errOutputInitFailed;
    }
    else if ( idOutput == 2 )
    {
        if ( m_pSystem->setOutput( FMOD_OUTPUTTYPE_ASIO ) != FMOD_OK )
            return errOutputInitFailed;
    }
    //-----------------//
    //--- linux		---//
    //-----------------//
#elif defined (__linux)
    if ( idOutput == 0 )
    {
        if( m_pSystem->setOutput( FMOD_OUTPUTTYPE_OSS ) != FMOD_OK )
            idOutput = 2;
    }
    if ( idOutput == 2 )
    {
        if ( m_pSystem->setOutput( FMOD_OUTPUTTYPE_ALSA ) != FMOD_OK )
            idOutput = 1;
    }
     if ( idOutput == 1 )
    {
        if ( m_pSystem->setOutput( FMOD_OUTPUTTYPE_ESD ) != FMOD_OK )
            return errOutputInitFailed;
    }
#elif defined (APPLE)
#if 0
    if ( idOutput == 0 )
    {
        if( m_pSystem->setOutput( FMOD_OUTPUTTYPE_COREAUDIO ) != FMOD_OK )
            return errOutputInitFailed;
    }
    else if ( idOutput == 1 )
    {
        if( m_pSystem->setOutput( FMOD_OUTPUTTYPE_SOUNDMANAGER ) != FMOD_OK )
            return errOutputInitFailed;
    }
#endif
#else 
    #error System not supported
#endif

#ifndef APPLE
    if(idDevice > 0)
    {
        //---------------------//
        //--- setup device	---//
        //---------------------//
        if (  m_pSystem->setDriver( idDevice )!= FMOD_OK )
            return errDeviceInitFailed;
    }

    // initialize system
    int oldNumOutCh=2,oldNumInCh=8;
    FMOD_SOUND_FORMAT oldFormat = FMOD_SOUND_FORMAT_PCM16;
    FMOD_DSP_RESAMPLER resamplemethod;
    if(nMixRate)
    {
        m_pSystem->getSoftwareFormat(NULL,&oldFormat,&oldNumOutCh,&oldNumInCh,&resamplemethod,NULL);
        if(m_pSystem->setSoftwareFormat(nMixRate,oldFormat,oldNumOutCh,oldNumInCh,resamplemethod) != FMOD_OK)
            return errUnknown;
    }
#endif //!APPLE

	FMOD_SPEAKERMODE speakermode = FMOD_SPEAKERMODE_STEREO; 
	FMOD_CAPS        caps; 
	FMOD_RESULT result = m_pSystem->getDriverCaps(0, &caps, 0, 0, &speakermode); 
	result = m_pSystem->setSpeakerMode(speakermode);       /* Set the user selected speaker mode. */ 

	if (caps & FMOD_CAPS_HARDWARE_EMULATED)             /* The user has the 'Acceleration' slider set to off!  This is really bad for latency!. */ 
	{                                                   /* You might want to warn the user about this. */ 
		result = m_pSystem->setDSPBufferSize(1024, 10);    /* At 48khz, the latency between issuing an fmod command and hearing it will now be about 213ms. */ 
	} 

    result = m_pSystem->init(nMaxChannels, FMOD_INIT_NORMAL, 0);
	if (result == FMOD_ERR_OUTPUT_CREATEBUFFER)         /* Ok, the speaker mode selected isn't supported by this soundcard.  Switch it back to stereo... */ 
	{ 
		result = m_pSystem->setSpeakerMode(FMOD_SPEAKERMODE_STEREO); 

		result = m_pSystem->init(100, FMOD_INIT_NORMAL, 0); /* Replace with whatever channel count and flags you use! */ 
	} 

    if(result != FMOD_OK)
    {
        printf("FMOD error! (%d) %s\n", result, FMOD_ErrorString(result));
        return errUnknown;
    }
    return errSuccess;
}
void FMODExEngine::SetBufferMs(int nSndBufferMs)
{
    MUSIKEngine::SetBufferMs(nSndBufferMs);
    m_pSystem->setStreamBufferSize(nSndBufferMs, FMOD_TIMEUNIT_MS);
}
MUSIKEngine::Error FMODExEngine::Init(int idOutput ,int idDevice ,int nMixRate ,int nMaxChannels)
{
    if(!m_pSystem)
        return errUnknown;
    if(m_bValid)
        m_pSystem->close();
    Error e = _Init(idOutput,idDevice,nMixRate,nMaxChannels);
    m_bValid = (e == errSuccess);
    return e;
}
MUSIKEngine::Error FMODExEngine::EnumDevices(MUSIKEngine::IEnumNames * pen) const
{
    int numDrivers = 0;
    char name[100];
    m_pSystem->getNumDrivers(&numDrivers);
    for ( int i = 0; i < numDrivers ; i++ )
    {
#ifdef APPLE
	strcpy(name,"Default");
#else
        m_pSystem->getDriverName(i,name,sizeof(name) - 1);
#endif
        pen->EnumNamesCallback(name,i);
    }
    return errSuccess;
}
MUSIKEngine::Error FMODExEngine::EnumOutputs(IEnumNames * pen) const
{
    static const char * szData[] =
    {
#if defined(WIN32)
        "Direct Sound",
        "Windows Multimedia",
        "ASIO"
#elif defined(__linux)
        "OSS",
        "ESD",
        "ALSA 0.9"
#elif defined(APPLE)
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

MUSIKEngine::Error FMODExEngine::EnumFrequencies(IEnumNames * pen) const
{
    static const char * szData[] =
    {
        "Default",
        "48000",
        "44100",
        "22050",
        "11025",
        "8000"
    };   
    for ( size_t i = 0; i < sizeof(szData)/sizeof(szData[0]); i++ )
    {
        pen->EnumNamesCallback(szData[i],(int)i);
    }
    return errSuccess;

}

MUSIKEngine::Error FMODExEngine::SetNetworkProxy(const char * s)
{
  return m_pSystem->setNetworkProxy(s) == FMOD_OK ? errSuccess:errUnknown;
}

MUSIKEngine::Error FMODExEngine::SetNetBuffer(int nBufferSize,int nPreBufferPercent,int nReBufferPercent)
{
    MUSIKEngine::SetNetBuffer(nBufferSize,nPreBufferPercent,nReBufferPercent);
    return errSuccess;
}

IMUSIKStreamOut *FMODExEngine::CreateStreamOut()
{
  return new FMODExStreamOut(*this);
}
MUSIKDefaultDecoder *FMODExEngine::CreateDefaultDecoder()
{
	return  new	MUSIKDefaultDecoder(new FMODExStreamOut(*this));
}
const char *FMODExEngine::ErrorString()
{
   // int errcode = FSOUND_GetError();
	return "unkown error";
}

void FMODExEngine::SetVolume(float v)
{ 
    FMOD::ChannelGroup * chgroup = NULL;
    m_pSystem->getMasterChannelGroup(&chgroup);
    chgroup->setVolume(v);
}

float FMODExEngine::GetVolume() const
{ 
    FMOD::ChannelGroup * chgroup = NULL;
    float v = 0.0f;
    m_pSystem->getMasterChannelGroup(&chgroup);
    chgroup->getVolume(&v);
    return v;
}

bool FMODExEngine::SetPlayState( MUSIKEngine::PlayState state)
{
    FMOD::ChannelGroup * chgroup = NULL;
    m_pSystem->getMasterChannelGroup(&chgroup);
	switch (state )
	{
	case MUSIKEngine::Paused:
		return chgroup->setPaused(true) == FMOD_OK;
	case MUSIKEngine::Playing:
		return chgroup->setPaused(false) == FMOD_OK;
	case MUSIKEngine::Stopped:
		return chgroup->setPaused(true) == FMOD_OK;
	case MUSIKEngine::Invalid:
		return false;
	}
	return false;
}

const char * FMODExEngine::Version() const
{
    return m_szVersion;
}

FMODExEngine::~FMODExEngine(void)
{
    if(m_pSystem)
    {
        if(m_bValid)
            m_pSystem->close();
        m_pSystem->release();
    }
    
}
