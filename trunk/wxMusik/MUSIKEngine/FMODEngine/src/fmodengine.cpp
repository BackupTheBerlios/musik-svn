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

#include "MUSIKEngine/FMODEngine/inc/fmodengine.h"
#include "MUSIKEngine/MUSIKEngine/inc/defaultdecoder.h"
#include "fmodstreamout.h"
#include <fmod.h>
#include <fmod_errors.h>
#include <stdio.h>
#ifdef WIN32
#define snprintf _snprintf
#endif

FMODEngine::FMODEngine()
{
    m_OpenMode = OpenMode_Default;
    snprintf(m_szVersion,sizeof(m_szVersion)/sizeof(m_szVersion[0]) - 1,"%.2f",FSOUND_GetVersion());
    m_bValid = false;
}

MUSIKEngine::Error FMODEngine::_Init(int idOutput ,int idDevice,int nMixRate,int nMaxChannels,int nSndBufferMs)
{
    //-----------------//
    //--- windows	---//
    //-----------------//
#if defined(__WXMSW__)
    if ( idOutput == 0 )
    {
        if( FSOUND_SetOutput( FSOUND_OUTPUT_DSOUND ) == 0 )
            return errOutputInitFailed;
    }
    else if ( idOutput == 1 )
    {
        if ( FSOUND_SetOutput( FSOUND_OUTPUT_WINMM ) == 0 )
            return errOutputInitFailed;
    }
    else if ( idOutput == 2 )
    {
        if ( FSOUND_SetOutput( FSOUND_OUTPUT_ASIO ) == 0 )
            return errOutputInitFailed;
    }
    //-----------------//
    //--- linux		---//
    //-----------------//
#elif defined (__WXGTK__)
    if ( idOutput == 0 )
    {
        if( FSOUND_SetOutput( FSOUND_OUTPUT_OSS ) == 0 )
            return errOutputInitFailed;
    }
    else if ( idOutput == 1 )
    {
        if ( FSOUND_SetOutput( FSOUND_OUTPUT_ESD ) == 0 )
            return errOutputInitFailed;
    }
    else if ( idOutput == 2 )
    {
        if ( FSOUND_SetOutput( FSOUND_OUTPUT_ALSA ) == 0 )
            return errOutputInitFailed;
    }
#elif defined (__WXMAC__)
    if( FSOUND_SetOutput( FSOUND_OUTPUT_MAC ) == 0 )
        return errOutputInitFailed;
#endif
    if(idDevice > 0)
    {
        //---------------------//
        //--- setup device	---//
        //---------------------//
        if (  FSOUND_SetDriver( idDevice ) == 0 )
            return errDeviceInitFailed;
    }

    // initialize system
   
    if(!FSOUND_Init(nMixRate,nMaxChannels,0))
        return errUnknown;
#ifdef __WXMAC__
    FSOUND_SetBufferSize( 1000 );
#else        
    FSOUND_SetBufferSize( 500 );
#endif   
    FSOUND_Stream_SetBufferSize( nSndBufferMs );
    return errSuccess;
}
MUSIKEngine::Error FMODEngine::Init(int idOutput ,int idDevice ,int nMixRate ,int nMaxChannels)
{
    if(m_bValid)
        FSOUND_Close();
    int nSndBufferMs = 400;
    Error e = _Init(idOutput,idDevice,nMixRate,nMaxChannels,nSndBufferMs);
    m_bValid = (e == errSuccess);
    return e;
}
MUSIKEngine::Error FMODEngine::EnumDevices(MUSIKEngine::IEnumNames * pen) const
{
    if(!m_bValid)
        return errUnknown;
    for ( int i = 0; i < FSOUND_GetNumDrivers(); i++ )
    {
        pen->EnumNamesCallback(FSOUND_GetDriverName( i ),i);
    }
    return errSuccess;
}
MUSIKEngine::Error FMODEngine::EnumOutputs(IEnumNames * pen) const
{
    if(!m_bValid)
        return errUnknown;
    static const char * szData[] =
    {
#if defined(_WIN32)
        "Direct Sound",
        "Windows Multimedia",
        "ASIO"
#elif defined(__linux)
        "OSS",
        "ESD",
        "ALSA 0.9"
#elif defined( __apple__)
        "MAC"
#else 
    #error System not supported
#endif
    };
    for ( int i = 0; i < sizeof(szData)/sizeof(szData[0]); i++ )
    {
        pen->EnumNamesCallback(szData[i],i);
    }
    return errSuccess;

}

MUSIKEngine::Error FMODEngine::EnumFrequencies(IEnumNames * pen) const
{
    static const char * szData[] =
    {
        "48000",
        "44100",
        "22050",
        "11025",
        "8000"
    };   
    for ( int i = 0; i < sizeof(szData)/sizeof(szData[0]); i++ )
    {
        pen->EnumNamesCallback(szData[i],i);
    }
    return errSuccess;

}

MUSIKEngine::Error FMODEngine::SetNetworkProxy(const char * s)
{
  return FSOUND_Stream_Net_SetProxy(s)? errSuccess:errUnknown;
}

MUSIKEngine::Error FMODEngine::SetNetBuffer(int nBufferSize,int nPreBufferPercent,int nReBufferPercent)
{
    return FSOUND_Stream_Net_SetBufferProperties(nBufferSize , nPreBufferPercent, nReBufferPercent) ? errSuccess:errUnknown;
}

IMUSIKStreamOut *FMODEngine::CreateStreamOut()
{
  return new FMODStreamOut(*this);
}
MUSIKDefaultDecoder *FMODEngine::CreateDefaultDecoder()
{
	return  new	MUSIKDefaultDecoder(new FMODStreamOut(*this));
}
char *FMODEngine::ErrorString()
{
    int errcode = FSOUND_GetError();
	return FMOD_ErrorString(errcode);
}

void FMODEngine::SetVolume(float v)
{ 
    FSOUND_SetSFXMasterVolume((int) (v * 255.0)); 
}

float FMODEngine::GetVolume()
{ 
    return (float)(((double)FSOUND_GetSFXMasterVolume()+0.5)/255.0); 
}

bool FMODEngine::SetPlayState( MUSIKEngine::PlayState state)
{
	switch (state )
	{
	case MUSIKEngine::Paused:
		return FSOUND_SetPaused(FSOUND_ALL, 1) != 0;
	case MUSIKEngine::Playing:
		return FSOUND_SetPaused(FSOUND_ALL, 0) != 0;
	case MUSIKEngine::Stopped:
		return FSOUND_SetPaused(FSOUND_ALL, 1) != 0;
	case MUSIKEngine::Invalid:
		return false;
	}
	return false;
}

const char * FMODEngine::Version() const
{
    return m_szVersion;
}

FMODEngine::~FMODEngine(void)
{	
    if(m_bValid)
	    FSOUND_Close();
}
