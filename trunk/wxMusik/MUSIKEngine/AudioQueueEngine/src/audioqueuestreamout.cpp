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

#include "audioqueuestreamout.h"

	// we only use time here as a guideline
	// we're really trying to get somewhere between 16K and 64K buffers, but not allocate too much if we don't need it
void CalculateBytesForTime (CAStreamBasicDescription & inDesc, UInt32 inMaxPacketSize, Float64 inSeconds, UInt32 *outBufferSize, UInt32 *outNumPackets)
{
	static const int maxBufferSize = 0x10000; // limit size to 64K
	static const int minBufferSize = 0x4000; // limit size to 16K

	if (inDesc.mFramesPerPacket) {
		Float64 numPacketsForTime = inDesc.mSampleRate / inDesc.mFramesPerPacket * inSeconds;
		*outBufferSize = numPacketsForTime * inMaxPacketSize;
	} else {
		// if frames per packet is zero, then the codec has no predictable packet == time
		// so we can't tailor this (we don't know how many Packets represent a time period
		// we'll just return a default buffer size
		*outBufferSize = maxBufferSize > inMaxPacketSize ? maxBufferSize : inMaxPacketSize;
	}
	
		// we're going to limit our size to our default
	if (*outBufferSize > maxBufferSize && *outBufferSize > inMaxPacketSize)
		*outBufferSize = maxBufferSize;
	else {
		// also make sure we're not too small - we don't want to go the disk for too small chunks
		if (*outBufferSize < minBufferSize)
			*outBufferSize = minBufferSize;
	}
	*outNumPackets = *outBufferSize / inMaxPacketSize;
}



void AudioQueueStreamOut::AQBufferCallback(void *					inUserData,
                                            AudioQueueRef			inAQ,
                                            AudioQueueBufferRef		inCompleteAQBuffer)
{
	AudioQueueStreamOut * pThis = (AudioQueueStreamOut *)inUserData;
	if (pThis->mInfo.mDone) return;
    if(pThis->mInfo.m_SeekToPacket != -1)
    {
        pThis->mInfo.mCurrentPacket = pThis->mInfo.m_SeekToPacket;
        pThis->mInfo.m_SeekToPacket = -1; 
    }        
	UInt32 numBytes;
	UInt32 nPackets = pThis->mInfo.mNumPacketsToRead;

	if ((AudioFileReadPackets(pThis->mInfo.mAudioFile, false, &numBytes, pThis->mInfo.mPacketDescs, pThis->mInfo.mCurrentPacket, &nPackets, 
								inCompleteAQBuffer->mAudioData) == 0)
                                
		
	 && (nPackets > 0)) {
		inCompleteAQBuffer->mAudioDataByteSize = numBytes;		
        AudioQueueParameterEvent event;
        event.mID = kAudioQueueParam_Volume;
        event.mValue = pThis->m_Volume; 
		AudioQueueEnqueueBufferWithParameters(inAQ, inCompleteAQBuffer, (pThis->mInfo.mPacketDescs ? nPackets : 0), pThis->mInfo.mPacketDescs,0,0,1, &event,NULL,NULL);
		
		pThis->mInfo.mCurrentPacket += nPackets;
	} 
    else 
    {
		AudioQueueStop(pThis->mInfo.mQueue, false);
			// reading nPackets == 0 is our EOF condition
		pThis->mInfo.mDone = true;
	}
}

AudioQueueStreamOut::AudioQueueStreamOut(AudioQueueEngine &engine)
:IMUSIKStreamOutDefault(engine)
,m_Engine(engine)
{
    mInfo.mQueue = NULL;
    mInfo.mAudioFile = NULL;
    mInfo.mPacketDescs = NULL;
 	bNetStream = false;
    m_Volume = 0.0f;
    m_bPaused = false;
    m_totalFrames = 0;
    mInfo.m_SeekToPacket = -1;
}
AudioQueueStreamOut::~AudioQueueStreamOut()
{
	Close();
}



bool AudioQueueStreamOut::Start()
{
    if(!mInfo.mQueue)
        return false;
    return AudioQueueStart(mInfo.mQueue, NULL) == 0;
}   

void AudioQueueStreamOut::SetVolume(float v)// range is 0.0 to 1.0
{
    m_Volume = v;
 //   if(mInfo.mQueue)
 //       AudioQueueSetParameter(mInfo.mQueue, kAudioQueueParam_Volume, v);
}

float AudioQueueStreamOut::GetVolume()
{

    if(mInfo.mQueue)
          AudioQueueGetParameter(mInfo.mQueue, kAudioQueueParam_Volume,&m_Volume);
            
	return m_Volume;
}

bool AudioQueueStreamOut::SetPlayState( MUSIKEngine::PlayState state)
{
    if(!mInfo.mQueue)
        return false;
       
	switch (state )
	{
	case MUSIKEngine::Paused:
		return m_bPaused = (AudioQueuePause(mInfo.mQueue) == 0);
	case MUSIKEngine::Playing:
        m_bPaused = false; 
		return AudioQueueStart(mInfo.mQueue, NULL) == 0;
	case MUSIKEngine::Stopped:
        m_bPaused = false; 
        mInfo.mCurrentPacket = 0;
		return AudioQueueStop(mInfo.mQueue, true) == 0;
	default:
		break;		
	}
	return false;
}

MUSIKEngine::PlayState AudioQueueStreamOut::GetPlayState()
{
    if(mInfo.mQueue)
    {
        UInt32 IsRunning = 0;
        UInt32 size = sizeof(IsRunning);
        AudioQueueGetProperty (mInfo.mQueue, kAudioQueueProperty_IsRunning, &IsRunning, &size);
        if(  m_bPaused  && IsRunning )
            return MUSIKEngine::Paused;
        else if( !m_bPaused && IsRunning )
            return MUSIKEngine::Playing;
        else
            return MUSIKEngine::Stopped;
    }
    return MUSIKEngine::Invalid;
}

bool AudioQueueStreamOut::Close()
{
	bool bRes = true;
	if(mInfo.mQueue)
    {
        mInfo.mDone = TRUE;
        AudioQueueStop(mInfo.mQueue, true);
        AudioQueueDispose(mInfo.mQueue, true);
        mInfo.mQueue = NULL;
    }
	if(mInfo.mAudioFile)
    {
        AudioFileClose(mInfo.mAudioFile);
        mInfo.mAudioFile = NULL;
    }
    delete [] mInfo.mPacketDescs;
    mInfo.mPacketDescs = NULL;
	return bRes;
}

bool AudioQueueStreamOut::Open(const char *FileName)
{
    delete [] mInfo.mPacketDescs;
    mInfo.mPacketDescs = NULL;
    m_totalFrames = 0;
    mInfo.m_SeekToPacket = -1;
 	try {
   
        CFURLRef sndFile = CFURLCreateFromFileSystemRepresentation (NULL, (const UInt8 *)FileName, strlen(FileName), false);
        if (!sndFile) return false;
            
        OSStatus result = AudioFileOpenURL (sndFile, 0x1/*fsRdPerm*/, 0/*inFileTypeHint*/, &mInfo.mAudioFile);
        CFRelease (sndFile);
                                
        UInt32 size = sizeof(mInfo.mDataFormat);
        XThrowIfError(AudioFileGetProperty(mInfo.mAudioFile, 
                                    kAudioFilePropertyDataFormat, &size, &mInfo.mDataFormat), "couldn't get file's data format");
        
        printf ("File format: "); mInfo.mDataFormat.Print();

        XThrowIfError(AudioQueueNewOutput(&mInfo.mDataFormat, AudioQueueStreamOut::AQBufferCallback, this, 
                                    NULL, kCFRunLoopCommonModes, 0, &mInfo.mQueue), "AudioQueueNew failed");

        UInt32 bufferByteSize;
        
        // we need to calculate how many packets we read at a time, and how big a buffer we need
        // we base this on the size of the packets in the file and an approximate duration for each buffer
        {
            bool isFormatVBR = (mInfo.mDataFormat.mBytesPerPacket == 0 || mInfo.mDataFormat.mFramesPerPacket == 0);
            
            // first check to see what the max size of a packet is - if it is bigger
            // than our allocation default size, that needs to become larger
            UInt32 maxPacketSize;
            size = sizeof(maxPacketSize);
            XThrowIfError(AudioFileGetProperty(mInfo.mAudioFile, 
                                    kAudioFilePropertyPacketSizeUpperBound, &size, &maxPacketSize), "couldn't get file's max packet size");
            
            // adjust buffer size to represent about a half second of audio based on this format
            CalculateBytesForTime (mInfo.mDataFormat, maxPacketSize, 0.5/*seconds*/, &bufferByteSize, &mInfo.mNumPacketsToRead);
            
            if (isFormatVBR)
                mInfo.mPacketDescs = new AudioStreamPacketDescription [mInfo.mNumPacketsToRead];
            else
                mInfo.mPacketDescs = NULL; // we don't provide packet descriptions for constant bit rate formats (like linear PCM)
                
            printf ("Buffer Byte Size: %d, Num Packets to Read: %d\n", (int)bufferByteSize, (int)mInfo.mNumPacketsToRead);
        }

        // (2) If the file has a cookie, we should get it and set it on the AQ
        size = sizeof(UInt32);
        result = AudioFileGetPropertyInfo (mInfo.mAudioFile, kAudioFilePropertyMagicCookieData, &size, NULL);

        if (!result && size) {
            char* cookie = new char [size];		
            XThrowIfError (AudioFileGetProperty (mInfo.mAudioFile, kAudioFilePropertyMagicCookieData, &size, cookie), "get cookie from file");
            XThrowIfError (AudioQueueSetProperty(mInfo.mQueue, kAudioQueueProperty_MagicCookie, cookie, size), "set cookie on queue");
            delete [] cookie;
        }

            // prime the queue with some data before starting
        mInfo.mDone = false;
        mInfo.mCurrentPacket = 0;
        for (UInt32 i = 0; i < sizeof(mInfo.mBuffers)/sizeof(mInfo.mBuffers[0]); ++i) {
            XThrowIfError(AudioQueueAllocateBuffer(mInfo.mQueue, bufferByteSize, &mInfo.mBuffers[i]), "AudioQueueAllocateBuffer failed");

            AQBufferCallback (this, mInfo.mQueue, mInfo.mBuffers[i]);
            
            if (mInfo.mDone) break;
        }	
        return IMUSIKStreamOutDefault::Create(NULL);
	}
	catch (CAXException e) {
		char buf[256];
		fprintf(stderr, "Error: %s (%s)\n", e.mOperation, e.FormatError(buf));
	}
    
    return false;
}
bool AudioQueueStreamOut::SetSamplePos( int64_t samplepos)
{
    if( samplepos > GetSampleCount() ) 
        return false;
    mInfo.m_SeekToPacket = samplepos / mInfo.mDataFormat.mFramesPerPacket;
    return true;
}

bool AudioQueueStreamOut::SetTime( int64_t nTimeMS)
{
	return SetSamplePos(nTimeMS * mInfo.mDataFormat.mSampleRate / 1000);
}
int64_t AudioQueueStreamOut::GetSamplePos()
{
	return (int64_t)mInfo.mCurrentPacket * mInfo.mDataFormat.mFramesPerPacket;
}

int64_t AudioQueueStreamOut::GetTime()
{
   return (int64_t) ((double)GetSamplePos() * 1000.0 / (double)mInfo.mDataFormat.mSampleRate );
}
int64_t AudioQueueStreamOut::GetLengthMs()
{
        
    return (int64_t) ((double)GetSampleCount() * 1000.0 / (double)mInfo.mDataFormat.mSampleRate );

}

int64_t AudioQueueStreamOut::GetSampleCount()
{
    unsigned int nLength = 0;
    if(!mInfo.mAudioFile)
        return 0;
    Float64 TimeSec = 0;   
    UInt32 propertySize = sizeof(TimeSec);
    OSStatus err = AudioFileGetProperty(mInfo.mAudioFile, kAudioFilePropertyEstimatedDuration, &propertySize, &TimeSec);
    if (err) 
    {
        return 0;
    }    
    m_totalFrames = (int64_t)(TimeSec * mInfo.mDataFormat.mSampleRate); 
       
/*
    if(m_totalFrames);
        return m_totalFrames;

    UInt64 dataPacketCount;
    UInt32 propertySize = sizeof(dataPacketCount);
    OSStatus err = AudioFileGetProperty(mInfo.mAudioFile, kAudioFilePropertyAudioDataPacketCount, &propertySize, &dataPacketCount);
    if (err) 
    {
        // try  using AudioFilePacketTableInfo
        AudioFilePacketTableInfo pti;
        propertySize = sizeof(pti);
        err = AudioFileGetProperty(mInfo.mAudioFile, kAudioFilePropertyPacketTableInfo, &propertySize, &pti);
        if (err == noErr)
            m_totalFrames = pti.mNumberValidFrames;
        // else dont know
    } 
    else 
    {
        if (mInfo.mDataFormat.mFramesPerPacket)
            m_totalFrames = mInfo.mDataFormat.mFramesPerPacket * dataPacketCount;
    }
*/    
    return m_totalFrames;
}
        
int64_t AudioQueueStreamOut::GetFilesize()
{
    unsigned int nLength = 100000000;
    return (int64_t)nLength;
}
const char * AudioQueueStreamOut::Type()
{
	return "AudioQueue";
}
bool AudioQueueStreamOut::CanSeek()
{
	return !bNetStream;
}


MUSIKEngine::Error AudioQueueStreamOut::GetOpenStatus(MUSIKEngine::OpenStatus *pStatus)
{
    *pStatus = MUSIKEngine::OPENSTATUS_READY;
    return MUSIKEngine::errSuccess;
}

MUSIKEngine::Error AudioQueueStreamOut::GetNetStatus(MUSIKEngine::NetStatus *pStatus,int * pnPercentRead,int * pnBitrate)
{
    *pStatus = MUSIKEngine::NETSTATUS_ERROR;
    return MUSIKEngine::errSuccess;
}
