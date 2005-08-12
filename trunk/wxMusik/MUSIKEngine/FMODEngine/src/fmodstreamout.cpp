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

#include "fmodstreamout.h"

#define FMOD_STREAM_FORMAT_FLAGS (FSOUND_SIGNED | FSOUND_16BITS | FSOUND_STEREO|FSOUND_HW2D)

signed char F_CALLBACKAPI FMODStreamOut::StreamCallback(FSOUND_STREAM *stream, void *buff,
														int len, void *userdata)
{
	FMODStreamOut  *pThis = (FMODStreamOut *)userdata;
	return pThis->FillBuffer((unsigned char*)buff,len);
}

FMODStreamOut::FMODStreamOut(FMODEngine::eOpenMode m)
:m_OpenMode(m)
{
    
	StreamPointer = NULL;
	FMODChannel = -1;
	bNetStream = false;
}
FMODStreamOut::~FMODStreamOut()
{
	Close();
}
bool FMODStreamOut::DoCreate(int buffersize)
{
	if(StreamPointer)
	{                             
		Close();
	}
	StreamPointer = FSOUND_Stream_Create(StreamCallback, buffersize, FMOD_STREAM_FORMAT_FLAGS,(int) Decoder()->GetInfo()->frequency, (void *) this);
	return StreamPointer != NULL;
}



bool FMODStreamOut::Start()
{
    FMODChannel = FSOUND_Stream_PlayEx(FSOUND_FREE, StreamPointer, NULL, 1);
    FSOUND_SetPaused(FMODChannel, 0);
	return FMODChannel != -1;

}
void FMODStreamOut::SetVolume(float v)// range is 0.0 to 1.0
{
	FSOUND_SetVolume(FMODChannel, (int) (v * 255.0));
}
float FMODStreamOut::GetVolume()
{
	return (float)(((double)FSOUND_GetVolume(FMODChannel)+0.5)/255.0);
}

bool FMODStreamOut::SetPlayState( MUSIKEngine::PlayState state)
{
	switch (state )
	{
	case MUSIKEngine::Paused:
		return FSOUND_SetPaused(FMODChannel, 1) != 0;
	case MUSIKEngine::Playing:
		return FSOUND_SetPaused(FMODChannel, 0) != 0;
	case MUSIKEngine::Stopped:
		return FSOUND_SetPaused(FMODChannel, 1) != 0;
	default:
		break;		
	}
	return false;
}

MUSIKEngine::PlayState FMODStreamOut::GetPlayState()
{
	if( FSOUND_GetPaused(FMODChannel) != 0)
		return MUSIKEngine::Paused;
	else if(FSOUND_IsPlaying(FMODChannel))
		return MUSIKEngine::Playing;
	else
		return MUSIKEngine::Stopped;
}

bool FMODStreamOut::Close()
{
	bool bRes = true;
	if(StreamPointer)
		bRes = FSOUND_Stream_Close(StreamPointer) > 0;
	StreamPointer = NULL;

	return bRes;
}

bool FMODStreamOut::Open(const char *FileName)
{
	static const char * szhttp = "http://";
	bNetStream = (strncmp(FileName,szhttp,strlen(szhttp)) == 0);
	int nFlags = FMOD_STREAM_FORMAT_FLAGS|(m_OpenMode == FMODEngine::OpenMode_MPEGACCURATE ? FSOUND_MPEGACCURATE:0);
#ifdef WIN32
    if(bNetStream) nFlags |= FSOUND_NONBLOCKING;
#else
	if(bNetStream) nFlags = FSOUND_NORMAL|FSOUND_NONBLOCKING;// on linux and mac, HW2D does not work on net streams.
#endif
	StreamPointer = FSOUND_Stream_Open(FileName,nFlags,0,0);
	return StreamPointer != NULL;
}

bool FMODStreamOut::SetTime( int nTimeMS)
{
	return StreamPointer && FSOUND_Stream_SetTime(StreamPointer,nTimeMS);
}

int FMODStreamOut::GetTime()
{
	
	return  StreamPointer ? FSOUND_Stream_GetTime(StreamPointer):0;
}
int FMODStreamOut::GetLengthMs()
{
	return StreamPointer ? FSOUND_Stream_GetLengthMs(StreamPointer):0;
}

int FMODStreamOut::GetLength()
{
	return StreamPointer ? FSOUND_Stream_GetLength(StreamPointer):0;
}
const char * FMODStreamOut::Type()
{
	return "FMOD";
}
bool FMODStreamOut::CanSeek()
{
	return !bNetStream;
}
MUSIKEngine::Error  FMODStreamOut::SetMetadataCallback(IMUSIKStreamOutDefault::IMetadataCallback *pCB)
{
    pIMetadataCallback = pCB;
    return StreamPointer && FSOUND_Stream_Net_SetMetadataCallback(StreamPointer, MetadataCallback, this) ? MUSIKEngine::errSuccess: MUSIKEngine::errUnknown;
}
signed char F_CALLBACKAPI FMODStreamOut::MetadataCallback(char *name, char *value, void * userdata)
{
    FMODStreamOut  *pThis = (FMODStreamOut *)userdata;
    IMUSIKStreamOutDefault::IMetadataCallback *pMyIMetadataCallback = pThis->pIMetadataCallback;  // copy to temp variable in case another thread sets pIMetadataCallback to NULL.
    if(pMyIMetadataCallback)
        pMyIMetadataCallback->MetadataCallback(name,value);
    return 1;
}

MUSIKEngine::Error FMODStreamOut::GetOpenStatus(MUSIKEngine::OpenStatus *pStatus)
{
    if(!StreamPointer)
    {
        return MUSIKEngine::errUnknown;
    }

    int status = FSOUND_Stream_GetOpenState(StreamPointer);
    switch(status)
    {
    case 0:
        *pStatus = MUSIKEngine::OPENSTATUS_READY;
        break;
    case -1:
        return MUSIKEngine::errUnknown;
        break;
    case -3:
        *pStatus = MUSIKEngine::OPENSTATUS_OPENFAILED;
        break;
    case -2:
    case -4:
    case -5:
        *pStatus = MUSIKEngine::OPENSTATUS_OPENINPROGRESS;
    }
    return MUSIKEngine::errSuccess;
}

MUSIKEngine::Error FMODStreamOut::GetNetStatus(MUSIKEngine::NetStatus *pStatus,int * pnPercentRead,int * pnBitrate)
{
    unsigned int flags = 0;
    int status = 0;
    MUSIKEngine::Error err =  StreamPointer && FSOUND_Stream_Net_GetStatus(StreamPointer,&status,pnPercentRead,pnBitrate,&flags) ? MUSIKEngine::errSuccess: MUSIKEngine::errUnknown;
    switch(status)
    {
    case FSOUND_STREAM_NET_NOTCONNECTED:
        *pStatus = MUSIKEngine::NETSTATUS_NOTCONNECTED;
        break;
    case FSOUND_STREAM_NET_CONNECTING:
        *pStatus = MUSIKEngine::NETSTATUS_CONNECTING;
        break;
    case FSOUND_STREAM_NET_BUFFERING:
        *pStatus = MUSIKEngine::NETSTATUS_BUFFERING;
        break;
    case FSOUND_STREAM_NET_READY:
        *pStatus = MUSIKEngine::NETSTATUS_READY;
        break;
    case FSOUND_STREAM_NET_ERROR:
    default:
        *pStatus = MUSIKEngine::NETSTATUS_ERROR;
        break;
    }
    return err;
}