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


#include "MUSIKEngine/inc/engine.h"
#include "MUSIKEngine/inc/stream.h"
#include "MUSIKEngine/inc/defaultdecoder.h"
#include "MUSIKEngine/inc/imusikstreamout.h"

#include "apedecoder.h"
#include "mpcdecoder.h"
#include "flacdecoder.h"
#include "aacdecoder.h"

#ifndef _WIN32
#define _stricmp strcasecmp
#endif

#ifdef __VISUALC__
#ifdef _DEBUG
#pragma comment(lib,"MUSIKEngined")
#else
#pragma comment(lib,"MUSIKEngine")
#endif
#endif

MUSIKDefaultDecoder::MUSIKDefaultDecoder(IMUSIKStreamOutDefault * pIMUSIKStreamOutDefault)
:MUSIKDecoder(pIMUSIKStreamOutDefault)
{
	m_pIMUSIKStreamOutDefault = pIMUSIKStreamOutDefault;
}
bool MUSIKDefaultDecoder::OpenMedia(const char *FileName)
{
	if(m_pIMUSIKStreamOutDefault->Open(FileName))
	{
		m_Info.bits_per_sample = 16;

		m_Info.SampleCount = m_pIMUSIKStreamOutDefault->GetSampleCount();
		int64_t lenbytes = m_pIMUSIKStreamOutDefault->GetFilesize();
		if(GetLengthMs())// can be 0 for netstreams 
			m_Info.bitrate = (int)(lenbytes * 8 / GetLengthMs());

		m_Info.frequency = 44100;
		m_Info.channels = 2;
		return true;
	}
	return false;
}

bool MUSIKDefaultDecoder::Start()
{	
	return m_pIMUSIKStreamOutDefault->Start();
}

int64_t MUSIKDefaultDecoder::GetSamplePos()
{
    return m_pIMUSIKStreamOutDefault->GetSamplePos();
}


int64_t MUSIKDefaultDecoder::GetTime()
{
	return m_pIMUSIKStreamOutDefault->GetTime();
}


int64_t MUSIKDefaultDecoder::GetLengthMs()
{
	return m_pIMUSIKStreamOutDefault->GetLengthMs();

}
bool MUSIKDefaultDecoder::SetSamplePos( int64_t samplepos)
{
    return m_pIMUSIKStreamOutDefault->SetSamplePos(samplepos);
}

bool MUSIKDefaultDecoder::SetTime( int64_t nTimeMS)
{
	return m_pIMUSIKStreamOutDefault->SetTime(nTimeMS);
}



bool MUSIKDefaultDecoder::Close()
{
	return m_pIMUSIKStreamOutDefault->Close();
}




MUSIKStream* MUSIKEngine::OpenMedia(const char *FileName)
{
    if(FileName == NULL)
        return NULL;
	const char *FileExt = strrchr(FileName, '.');
    if(FileExt == NULL)
        return NULL;
	MUSIKDecoder *d = NULL;
	IMUSIKStreamOut *so = NULL;
#ifndef MUSIKENGINE_NO_APE_SUPPORT	
	if (_stricmp(FileExt,".ape") == 0)
	{
		so = CreateStreamOut(); 
		d = new MUSIKAPEDecoder(so);

	}
	else 
#endif
#ifndef MUSIKENGINE_NO_FLAC_SUPPORT	
	if (_stricmp(FileExt,".flac") == 0)
	{
		so = CreateStreamOut(); 
		d = new MUSIKFLACDecoder(so);
	}
	else 
#endif	
#ifndef MUSIKENGINE_NO_FAAD2_SUPPORT	
	if ((_stricmp(FileExt,".aac") == 0) || (_stricmp(FileExt,".mp4")==0) || (_stricmp(FileExt,".mp4a")==0))

	{
		so = CreateStreamOut(); 
		d = new MUSIKAACDecoder(so);
	}
	else 
#endif	
#ifndef MUSIKENGINE_NO_MPC_SUPPORT
	if ( (_stricmp(FileExt,".mpc")==0) || (_stricmp(FileExt,".mp+")==0) || (_stricmp(FileExt,".mpp")==0))
	{
		so = CreateStreamOut(); 
		d = new MUSIKMPCDecoder(so);
	}
	else
#endif	
	{
		d = CreateDefaultDecoder();
	}

	if(d)
	{
		if(d->OpenMedia(FileName))
		{
			return new MUSIKStream(d);
		}
		delete d; 
		return NULL;
	}
	delete so;
	return NULL;
}
