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

#ifndef MUSIKENGINE_NO_APE_SUPPORT
#include "apedecoder.h"
#endif
#ifndef MUSIKENGINE_NO_MPC_SUPPORT
#include "mpcdecoder.h"
#endif
#ifndef MUSIKENGINE_NO_FLAC_SUPPORT
#include "flacdecoder.h"
#endif
#ifndef _WIN32
#define stricmp strcasecmp
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

		m_Info.LengthMS = m_pIMUSIKStreamOutDefault->GetLengthMs();
		unsigned int lenbytes = m_pIMUSIKStreamOutDefault->GetLength();
		if(m_Info.LengthMS)// can be 0 for netstreams 
			m_Info.bitrate = lenbytes / m_Info.LengthMS  * 8;

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


int MUSIKDefaultDecoder::GetTime()
{
	return m_pIMUSIKStreamOutDefault->GetTime();
}


int MUSIKDefaultDecoder::GetLengthMs()
{
	return m_pIMUSIKStreamOutDefault->GetLengthMs();

}

bool MUSIKDefaultDecoder::SetTime( int nTimeMS)
{
	return m_pIMUSIKStreamOutDefault->SetTime(nTimeMS);

}



bool MUSIKDefaultDecoder::Close()
{
	return m_pIMUSIKStreamOutDefault->Close();
}




MUSIKStream* MUSIKEngine::OpenMedia(const char *FileName)
{
	char *FileExt = strrchr(FileName, '.');
    if(FileExt == NULL)
        return NULL;
	MUSIKDecoder *d = NULL;
	IMUSIKStreamOut *so = NULL;
#ifndef MUSIKENGINE_NO_APE_SUPPORT	
	if (stricmp(FileExt,".ape") == 0)
	{
		so = CreateStreamOut(); 
		d = new MUSIKAPEDecoder(so);

	}
	else 
#endif
#ifndef MUSIKENGINE_NO_FLAC_SUPPORT	
	if (stricmp(FileExt,".flac") == 0)
	{
		so = CreateStreamOut(); 
		d = new MUSIKFLACDecoder(so);
	}
	else 
#endif	
#ifndef MUSIKENGINE_NO_MPC_SUPPORT
	if ( (stricmp(FileExt,".mpc")==0) || (stricmp(FileExt,".mp+")==0) || (stricmp(FileExt,".mpp")==0))
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
