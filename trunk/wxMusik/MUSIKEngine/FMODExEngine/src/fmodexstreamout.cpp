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

#include "fmodexstreamout.h"

#define FMOD_STREAM_FORMAT_FLAGS (FSOUND_SIGNED | FSOUND_16BITS | FSOUND_STEREO|FSOUND_HW2D)

FMOD_RESULT F_CALLBACK FMODExStreamOut::pcmreadcallback(FMOD_SOUND *sound, void *data, unsigned int datalen)
{
    FMODExStreamOut  *pThis = NULL;
    FMOD_Sound_GetUserData(sound,(void**)&pThis);
    if(pThis)
        pThis->FillBuffer((unsigned char*)data,datalen);    
    return FMOD_OK;
}

FMODExStreamOut::FMODExStreamOut(FMODExEngine &engine)
:IMUSIKStreamOutDefault(engine)
,m_Engine(engine)
{
 	m_pSound = NULL;
    m_pChannel = NULL;
	bNetStream = false;
    m_Volume = 0.0f;
}
FMODExStreamOut::~FMODExStreamOut()
{
	Close();
}
bool FMODExStreamOut::DoCreate(int buffersize_ms)
{
    if(!Decoder())
        return false;
	if(m_pSound)
	{
		Close();
	}
    FMOD_CREATESOUNDEXINFO  createsoundexinfo;
    memset(&createsoundexinfo, 0, sizeof(FMOD_CREATESOUNDEXINFO));
    createsoundexinfo.cbsize            = sizeof(FMOD_CREATESOUNDEXINFO);              /* required. */
    createsoundexinfo.decodebuffersize  = (Decoder()->GetInfo()->frequency * buffersize_ms) / 1000; /* Chunk size of stream update in samples.  This will be the amount of data passed to the user callback. */
    createsoundexinfo.length            = (int)((Decoder()->GetInfo()->bits_per_sample / 8) * Decoder()->GetInfo()->SampleCount * Decoder()->GetInfo()->channels); /* Length of PCM data in bytes of whole song (for Sound::getLength) */
    createsoundexinfo.numchannels       = Decoder()->GetInfo()->channels;                                    /* Number of channels in the sound. */
    createsoundexinfo.defaultfrequency  = Decoder()->GetInfo()->frequency;       /* Default playback rate of sound. */
    switch(Decoder()->GetInfo()->bits_per_sample)
    {
    case 8:
        createsoundexinfo.format            = FMOD_SOUND_FORMAT_PCM8;                     /* Data format of sound. */
        break;
    case 16:
        createsoundexinfo.format            = FMOD_SOUND_FORMAT_PCM16;                     /* Data format of sound. */
        break;
    case 24:
        createsoundexinfo.format            = FMOD_SOUND_FORMAT_PCM24;                     /* Data format of sound. */
        break;
    case 32:
        createsoundexinfo.format            = FMOD_SOUND_FORMAT_PCM32;                     /* Data format of sound. */
        break;

    }
    createsoundexinfo.pcmreadcallback   = pcmreadcallback;                             /* User callback for reading. */
    createsoundexinfo.userdata = this;
    FMOD_MODE mode = FMOD_SOFTWARE|FMOD_2D | FMOD_OPENUSER | FMOD_LOOP_NORMAL|FMOD_CREATESTREAM; // FMOD_SOFTWARE is important, because else dsp will not work
    m_Engine.System().createSound(0, mode, &createsoundexinfo, &m_pSound);
    if(m_pSound)
    {
        return m_Engine.System().playSound(FMOD_CHANNEL_FREE,m_pSound,true,&m_pChannel) == FMOD_OK; // start paused
    }
    
	return false;
}



bool FMODExStreamOut::Start()
{
    if(!m_pSound)
        return false;
    SetVolume(m_Volume);
    return bNetStream ? m_Engine.System().playSound(FMOD_CHANNEL_FREE,m_pSound,false,&m_pChannel) == FMOD_OK:SetPlayState(MUSIKEngine::Playing);
}
void FMODExStreamOut::SetVolume(float v)// range is 0.0 to 1.0
{
    m_Volume = v;// cache volume, so we can set it right before play starts
    if(m_pChannel)
        m_pChannel->setVolume(v);
}
float FMODExStreamOut::GetVolume()
{
    float v = 0.0;
    if(m_pChannel)
        m_pChannel->getVolume(&v);
	return v;
}

bool FMODExStreamOut::SetPlayState( MUSIKEngine::PlayState state)
{
    if(!m_pChannel)
        return false;
	switch (state )
	{
	case MUSIKEngine::Paused:
		return m_pChannel->setPaused(true) == FMOD_OK;
	case MUSIKEngine::Playing:
		return m_pChannel->setPaused(false) == FMOD_OK;
	case MUSIKEngine::Stopped:
		return m_pChannel->setPaused(true) == FMOD_OK;
	default:
		break;		
	}
	return false;
}

MUSIKEngine::PlayState FMODExStreamOut::GetPlayState()
{
    bool bPaused = false,bIsPlaying = false;
    if(m_pChannel)
    {
        m_pChannel->getPaused(&bPaused);
        m_pChannel->isPlaying(&bIsPlaying);
        if(  bPaused )
            return MUSIKEngine::Paused;
        else if( bIsPlaying )
            return MUSIKEngine::Playing;
        else
            return MUSIKEngine::Stopped;
    }
    return MUSIKEngine::Invalid;
}

bool FMODExStreamOut::Close()
{
	bool bRes = true;
	if(m_pSound)
        bRes = m_pSound->release() == FMOD_OK;
    m_pSound = NULL;
	m_pChannel = NULL;
	return bRes;
}

bool FMODExStreamOut::Open(const char *FileName)
{
	static const char * szhttp = "http://";
	bNetStream = (strncmp(FileName,szhttp,strlen(szhttp)) == 0);
    FMODExEngine::eOpenMode openmode = m_Engine.GetOpenMode();
	int nFlags = FMOD_SOFTWARE | FMOD_2D | FMOD_CREATESTREAM|FMOD_MPEGSEARCH|(openmode == FMODExEngine::OpenMode_MPEGACCURATE ? FMOD_ACCURATETIME:0); // FMOD_SOFTWARE is important, because else dsp will not work
#ifdef WIN32
    if(bNetStream) nFlags |= FMOD_NONBLOCKING;
#else
	if(bNetStream) nFlags = FMOD_CREATESTREAM|FMOD_MPEGSEARCH|FMOD_NONBLOCKING;// on linux and mac, HW2D does not work on net streams.
#endif
    if(bNetStream)
        m_Engine.System().setStreamBufferSize(m_Engine.GetNetBufferSize(), FMOD_TIMEUNIT_RAWBYTES);
    else
        m_Engine.System().setStreamBufferSize(m_Engine.GetBufferMs(), FMOD_TIMEUNIT_MS);
    

    if(FMOD_OK != m_Engine.System().createSound(FileName,  nFlags ,  0, &m_pSound))
        return false;
    bool bRes =  bNetStream ? true: m_Engine.System().playSound(FMOD_CHANNEL_FREE,m_pSound,true,&m_pChannel) == FMOD_OK;//start paused
    if(bRes)
    {
        return IMUSIKStreamOutDefault::Create(NULL);
    }
    return bRes;
}
bool FMODExStreamOut::SetSamplePos( int64_t samplepos)
{
    return m_pChannel && m_pChannel->setPosition((int)samplepos,FMOD_TIMEUNIT_PCM);
}

bool FMODExStreamOut::SetTime( int64_t nTimeMS)
{
	return m_pChannel && m_pChannel->setPosition((int)nTimeMS,FMOD_TIMEUNIT_MS);
}
int64_t FMODExStreamOut::GetSamplePos()
{
    unsigned int pos = 0;
    if(m_pChannel) 
        m_pChannel->getPosition(&pos,FMOD_TIMEUNIT_PCM);
    return (int64_t)pos;
}

int64_t FMODExStreamOut::GetTime()
{
	unsigned int nTimeMS = 0;
    if(m_pChannel) 
        m_pChannel->getPosition(&nTimeMS,FMOD_TIMEUNIT_MS);
    return (int64_t)nTimeMS;
}
int64_t FMODExStreamOut::GetLengthMs()
{
    unsigned int nLengthMS = 0;
    if(m_pSound)
        m_pSound->getLength(&nLengthMS,FMOD_TIMEUNIT_MS);
    return (int64_t) nLengthMS;

}

int64_t FMODExStreamOut::GetSampleCount()
{
    unsigned int nLength = 0;
    if(m_pSound)
        m_pSound->getLength(&nLength,FMOD_TIMEUNIT_PCM);
    return (int64_t)nLength;
}
int64_t FMODExStreamOut::GetFilesize()
{
    unsigned int nLength = 0;
    if(m_pSound)
        m_pSound->getLength(&nLength,FMOD_TIMEUNIT_RAWBYTES);
    return (int64_t)nLength;
}
const char * FMODExStreamOut::Type()
{
	return "FMODEx";
}
bool FMODExStreamOut::CanSeek()
{
	return !bNetStream;
}


MUSIKEngine::Error FMODExStreamOut::GetOpenStatus(MUSIKEngine::OpenStatus *pStatus)
{
    if(!m_pSound)
    {
        return MUSIKEngine::errUnknown;
    }
    FMOD_OPENSTATE openstate = FMOD_OPENSTATE_ERROR;
    unsigned int percentbuffered = 0;
    bool bStarving = false;
    m_pSound->getOpenState(&openstate,&percentbuffered,&bStarving);
    m_pChannel->setMute(bStarving);

    switch(openstate)
    {
    case FMOD_OPENSTATE_READY:
        *pStatus = MUSIKEngine::OPENSTATUS_READY;
        break;
    case FMOD_OPENSTATE_ERROR:
        *pStatus = MUSIKEngine::OPENSTATUS_OPENFAILED;
        break;
    case FMOD_OPENSTATE_LOADING:
    case FMOD_OPENSTATE_CONNECTING:
    case FMOD_OPENSTATE_BUFFERING:
        *pStatus = MUSIKEngine::OPENSTATUS_OPENINPROGRESS;
    default:
        ;
    }
    return MUSIKEngine::errSuccess;
}

MUSIKEngine::Error FMODExStreamOut::GetNetStatus(MUSIKEngine::NetStatus *pStatus,int * pnPercentRead,int * pnBitrate)
{
    if(!m_pSound)
    {
        return MUSIKEngine::errUnknown;
    }
    FMOD_OPENSTATE openstate = FMOD_OPENSTATE_ERROR;
    unsigned int percentbuffered = 0;
    bool bStarving = false;
    if(m_pSound->getOpenState(&openstate,&percentbuffered,&bStarving) != FMOD_OK)
    {
        return MUSIKEngine::errUnknown;
    }
    *pnPercentRead = (int)percentbuffered;
    //pnBitrate
    m_Engine.System().update();
    if(m_pChannel)
    {
        m_pChannel->setMute(bStarving);
    }
        int tagsupdated = 0;
        m_pSound->getNumTags(0, &tagsupdated);
        if (tagsupdated)
        {
            for (;;)
            {
                FMOD_TAG tag;

                if (m_pSound->getTag(0, -1, &tag) != FMOD_OK)
                {
                    break;
                }

                if (tag.datatype == FMOD_TAGDATATYPE_STRING)
                {
                    IMUSIKStreamOutDefault::IMetadataCallback *pMyIMetadataCallback = m_pIMetadataCallback;  // copy to temp variable in case another thread sets pIMetadataCallback to NULL.
                    if(pMyIMetadataCallback)
                        pMyIMetadataCallback->MetadataCallback(tag.name,(const char *)tag.data);
                }
            }
        }
    
    switch(openstate)
    {
    case FMOD_OPENSTATE_READY:
        *pStatus = MUSIKEngine::NETSTATUS_READY;
        break;
    case FMOD_OPENSTATE_CONNECTING:
        *pStatus = MUSIKEngine::NETSTATUS_CONNECTING;
        break;
    case FMOD_OPENSTATE_LOADING:
    case FMOD_OPENSTATE_BUFFERING:
        *pStatus = MUSIKEngine::NETSTATUS_BUFFERING;
        break;
    case FMOD_OPENSTATE_ERROR:
    default:
        *pStatus = MUSIKEngine::NETSTATUS_ERROR;
        break;
    }
    return MUSIKEngine::errSuccess;
}
