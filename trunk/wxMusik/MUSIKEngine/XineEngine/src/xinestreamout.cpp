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

#include "xinestreamout.h"
extern "C"
{
#define XINE_COMPILE
#include <sys/types.h>
#include <xine.h>
}


XineStreamOut::XineStreamOut(XineEngine &engine)
:IMUSIKStreamOutDefault(engine)
,m_Engine(engine)
{
 	m_pStream = NULL;
    m_pEventQueue = NULL;
	bNetStream = false;
}
XineStreamOut::~XineStreamOut()
{
	Close();
}



bool XineStreamOut::Start()
{
    if(!m_pStream)
        return false;
    if( xine_play( m_pStream, 0, 0 ) )
    {
        return true;
    }
    int error = xine_get_error( m_pStream );
    return false;
}
void XineStreamOut::SetVolume(float v)// range is 0.0 to 1.0
{
    if(m_pStream)
        xine_set_param( m_pStream, XINE_PARAM_AUDIO_AMP_LEVEL,(int)v * 100);
}
float XineStreamOut::GetVolume()
{
    float v = 0.0;
    if(m_pStream)
        v = ((float)xine_get_param( m_pStream, XINE_PARAM_AUDIO_AMP_LEVEL ))/100.0f;
	return v;
}

bool XineStreamOut::SetPlayState( MUSIKEngine::PlayState state)
{
    if(!m_pStream)
        return false;
	switch (state )
	{
	case MUSIKEngine::Paused:
        xine_set_param( m_pStream, XINE_PARAM_SPEED, XINE_SPEED_PAUSE );
        xine_set_param( m_pStream, XINE_PARAM_AUDIO_CLOSE_DEVICE, 1);
        break;
    case MUSIKEngine::Stopped:
        xine_stop( m_pStream );
        xine_set_param( m_pStream, XINE_PARAM_AUDIO_CLOSE_DEVICE, 1);
        break;
	case MUSIKEngine::Playing:
        xine_set_param( m_pStream, XINE_PARAM_SPEED, XINE_SPEED_NORMAL );
	default:
		break;		
	}
	return false;
}

MUSIKEngine::PlayState XineStreamOut::GetPlayState()
{
    bool bPaused = false,bIsPlaying = false;
    if(m_pStream)
    {
        switch( xine_get_status( m_pStream ) )
        {
        case XINE_STATUS_PLAY: return xine_get_param( m_pStream, XINE_PARAM_SPEED ) ? MUSIKEngine::Playing : MUSIKEngine::Paused;
        case XINE_STATUS_STOP:
        default:     return MUSIKEngine::Stopped;
        }
    }
    return MUSIKEngine::Invalid;
}

bool XineStreamOut::Close()
{
    if(m_pStream)
    {
        SetPlayState(MUSIKEngine::Stopped);
        xine_close( m_pStream );
        if( m_pEventQueue ) 
            xine_event_dispose_queue( m_pEventQueue );
        xine_dispose( m_pStream );
        m_pStream = NULL;
    }
	return true;
}

bool XineStreamOut::Open(const char *FileName)
{
    m_pStream = m_Engine.CreateXineStream();
    if(!m_pStream)
        return false;
	static const char * szhttp = "http://";
	bNetStream = (strncmp(FileName,szhttp,strlen(szhttp)) == 0);

    XineEngine::eOpenMode openmode = m_Engine.GetOpenMode();
//	int nFlags = FMOD_HARDWARE | FMOD_2D | FMOD_CREATESTREAM|FMOD_MPEGSEARCH|(openmode == XineEngine::OpenMode_MPEGACCURATE ? FMOD_ACCURATETIME:0);
//#ifdef WIN32
//    if(bNetStream) nFlags |= FMOD_NONBLOCKING;
//#else
//	if(bNetStream) nFlags = FMOD_CREATESTREAM|FMOD_MPEGSEARCH|FMOD_NONBLOCKING;// on linux and mac, HW2D does not work on net streams.
//#endif
//    if(bNetStream)
//        m_Engine.System().setStreamBufferSize(m_Engine.GetNetBufferSize(), FMOD_TIMEUNIT_RAWBYTES);
//    else
//        m_Engine.System().setStreamBufferSize(m_Engine.GetBufferMs(), FMOD_TIMEUNIT_MS);
    
    m_pEventQueue = xine_event_new_queue( m_pStream );
    if(!m_pEventQueue)
        return false;
    
    xine_set_param( m_pStream, XINE_PARAM_METRONOM_PREBUFFER, 5000 );
    xine_set_param( m_pStream, XINE_PARAM_IGNORE_VIDEO, 1 );
    xine_set_param( m_pStream, XINE_PARAM_AUDIO_AMP_LEVEL, 0 );
    if( xine_open( m_pStream,FileName))
        return IMUSIKStreamOutDefault::Create(NULL);
    return false;
}
bool XineStreamOut::SetSamplePos( int64_t samplepos)
{
   return false; //return m_pChannel && m_pChannel->setPosition((int)samplepos,FMOD_TIMEUNIT_PCM);
}

bool XineStreamOut::SetTime( int64_t nTimeMS)
{
    if(!m_pStream)
        return false;
    if( xine_get_param( m_pStream, XINE_PARAM_SPEED ) == XINE_SPEED_PAUSE ) 
    {
        xine_play( m_pStream, 0, (int)nTimeMS );
        xine_set_param( m_pStream, XINE_PARAM_SPEED, XINE_SPEED_PAUSE );
    }
    else
        xine_play( m_pStream, 0, (int)nTimeMS );
    return true;
}
int64_t XineStreamOut::GetSamplePos()
{
    return 0;
}

int64_t XineStreamOut::GetTime()
{
    int time = 0;
    if(m_pStream)
        xine_get_pos_length( m_pStream, NULL, &time, NULL );

    return time;
}
int64_t XineStreamOut::GetLengthMs()
{
    int nLengthMS = 0;
    if(m_pStream)
        xine_get_pos_length( m_pStream, NULL, NULL, &nLengthMS );

    return (int64_t) nLengthMS;

}

int64_t XineStreamOut::GetSampleCount()
{
    return 0;
}
int64_t XineStreamOut::GetFilesize()
{
    return 0;
}
const char * XineStreamOut::Type()
{
	return "Xine";
}
bool XineStreamOut::CanSeek()
{
	return !bNetStream;
}


MUSIKEngine::Error XineStreamOut::GetOpenStatus(MUSIKEngine::OpenStatus *pStatus)
{
//    if(!m_pSound)
//    {
//        return MUSIKEngine::errUnknown;
//    }
//    FMOD_OPENSTATE openstate = FMOD_OPENSTATE_ERROR;
//    unsigned int percentbuffered = 0;
//    bool bStarving = false;
//    m_pSound->getOpenState(&openstate,&percentbuffered,&bStarving);
//    m_pChannel->setMute(bStarving);
//
//    switch(openstate)
//    {
//    case FMOD_OPENSTATE_READY:
//        *pStatus = MUSIKEngine::OPENSTATUS_READY;
//        break;
//    case FMOD_OPENSTATE_ERROR:
//        *pStatus = MUSIKEngine::OPENSTATUS_OPENFAILED;
//        break;
//    case FMOD_OPENSTATE_LOADING:
//    case FMOD_OPENSTATE_CONNECTING:
//    case FMOD_OPENSTATE_BUFFERING:
//        *pStatus = MUSIKEngine::OPENSTATUS_OPENINPROGRESS;
//    }
    return MUSIKEngine::errSuccess;
}

MUSIKEngine::Error XineStreamOut::GetNetStatus(MUSIKEngine::NetStatus *pStatus,int * pnPercentRead,int * pnBitrate)
{
//    if(!m_pSound)
//    {
//        return MUSIKEngine::errUnknown;
//    }
//    FMOD_OPENSTATE openstate = FMOD_OPENSTATE_ERROR;
//    unsigned int percentbuffered = 0;
//    bool bStarving = false;
//    if(m_pSound->getOpenState(&openstate,&percentbuffered,&bStarving) != FMOD_OK)
//    {
//        return MUSIKEngine::errUnknown;
//    }
//    *pnPercentRead = (int)percentbuffered;
//    //pnBitrate
//    m_Engine.System().update();
//    if(m_pChannel)
//    {
//        m_pChannel->setMute(bStarving);
//    }
//        int tagsupdated = 0;
//        m_pSound->getNumTags(0, &tagsupdated);
//        if (tagsupdated)
//        {
//            for (;;)
//            {
//                FMOD_TAG tag;
//
//                if (m_pSound->getTag(0, -1, &tag) != FMOD_OK)
//                {
//                    break;
//                }
//
//                if (tag.datatype == FMOD_TAGDATATYPE_STRING)
//                {
//                    IMUSIKStreamOutDefault::IMetadataCallback *pMyIMetadataCallback = m_pIMetadataCallback;  // copy to temp variable in case another thread sets pIMetadataCallback to NULL.
//                    if(pMyIMetadataCallback)
//                        pMyIMetadataCallback->MetadataCallback(tag.name,(const char *)tag.data);
//                }
//            }
//        }
//    
//    switch(openstate)
//    {
//    case FMOD_OPENSTATE_READY:
//        *pStatus = MUSIKEngine::NETSTATUS_READY;
//        break;
//    case FMOD_OPENSTATE_CONNECTING:
//        *pStatus = MUSIKEngine::NETSTATUS_CONNECTING;
//        break;
//    case FMOD_OPENSTATE_LOADING:
//    case FMOD_OPENSTATE_BUFFERING:
//        *pStatus = MUSIKEngine::NETSTATUS_BUFFERING;
//        break;
//    case FMOD_OPENSTATE_ERROR:
//    default:
//        *pStatus = MUSIKEngine::NETSTATUS_ERROR;
//        break;
//    }
    return MUSIKEngine::errSuccess;
}
