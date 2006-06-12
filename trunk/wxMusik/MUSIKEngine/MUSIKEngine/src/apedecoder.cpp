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

#include "apedecoder.h"
#ifndef MUSIKENGINE_NO_APE_SUPPORT 

#include "MUSIKEngine/inc/imusikstreamout.h"


#ifdef _WIN32
#include "Shared/All.h"							/* Monkey's Audio include file */
#include "MACLib/MACLib.h"						/* Monkey's Audio include file */
#include "Shared/CharacterHelper.h"
#ifdef __VISUALC__
#pragma comment(lib,"MACLib")
#endif

#else
#define BUILD_CROSS_PLATFORM
#include "mac/All.h"							/* Monkey's Audio include file */
#include "mac/MACLib.h"						/* Monkey's Audio include file */
#include "mac/CharacterHelper.h"


#endif

const float APEDecodeBufferSec = 0.5f;			/* APE:  Buffersize in seconds (changeable) */


MUSIKAPEDecoder::MUSIKAPEDecoder(IMUSIKStreamOut * pIMUSIKStreamOut)
:MUSIKDecoder(pIMUSIKStreamOut)
{
	memset(&m_ApeInfo,0,sizeof(m_ApeInfo));
}

MUSIKAPEDecoder::~MUSIKAPEDecoder()
{
	Close();
}

int  MUSIKAPEDecoder::DecodeBlocks(unsigned char  *buff,int len)
{
	int nBlocksOut = 0;
	int bytesPerBlock = m_Info.channels * (m_Info.bits_per_sample >> 3);
	m_ApeInfo.pAPEDecompress->GetData((char *) buff,len / bytesPerBlock,&nBlocksOut); /* Decode some blocks */
	return nBlocksOut * bytesPerBlock ;
}
bool MUSIKAPEDecoder::OpenMedia(const char *FileName)
{

	int nRetVal=0;
#ifdef __APPLE__
	IAPEDecompress * pAPEDecompress = CreateIAPEDecompress(FileName, &nRetVal);
#else
	CSmartPtr<wchar_t> wsFileName;
	wsFileName.Assign(GetUTF16FromANSI(FileName),TRUE);
	IAPEDecompress * pAPEDecompress = CreateIAPEDecompress(wsFileName, &nRetVal);
#endif
	if (pAPEDecompress != NULL)
	{
		m_ApeInfo.pAPEDecompress = pAPEDecompress;
		m_Info.bitrate = pAPEDecompress->GetInfo(APE_INFO_AVERAGE_BITRATE);
		m_Info.bits_per_sample = pAPEDecompress->GetInfo(APE_INFO_BITS_PER_SAMPLE);
		m_Info.channels = pAPEDecompress->GetInfo(APE_INFO_CHANNELS);
		m_Info.frequency = pAPEDecompress->GetInfo(APE_INFO_SAMPLE_RATE);
		m_Info.SampleCount = pAPEDecompress->GetInfo(APE_DECOMPRESS_TOTAL_BLOCKS);
		int bytesPerBlock = m_Info.channels * (m_Info.bits_per_sample >> 3);
		int decoder_buffer_size = bytesPerBlock * (int)((APEDecodeBufferSec * pAPEDecompress->GetInfo(APE_INFO_SAMPLE_RATE)) + 0.5);
		m_Info.FileSize = pAPEDecompress->GetInfo(APE_INFO_APE_TOTAL_BYTES);
		return CreateBuffer(decoder_buffer_size);
	}
	Close();
	return false;
}

int64_t MUSIKAPEDecoder::GetTime()
{
	return m_ApeInfo.pAPEDecompress->GetInfo(APE_DECOMPRESS_CURRENT_MS);
}


bool MUSIKAPEDecoder::DoSeek(int64_t samplepos)
{
	if(m_ApeInfo.pAPEDecompress->Seek((int)samplepos))
    {
        SetDecodeSamplePos(samplepos);
        return true;// do not stop
    }
    return false;
}

MUSIKDecoder::INFO * MUSIKAPEDecoder::GetInfo()
{
	m_Info.bitrate = m_ApeInfo.pAPEDecompress->GetInfo(APE_DECOMPRESS_CURRENT_BITRATE);
	return &m_Info;
}



bool MUSIKAPEDecoder::Close()
{
	MUSIKDecoder::Close();
	delete m_ApeInfo.pAPEDecompress; /* Delete IAPEDecompress interface */
	m_ApeInfo.pAPEDecompress = NULL;
	return true;
}

#endif //MUSIKENGINE_NO_APE_SUPPORT
