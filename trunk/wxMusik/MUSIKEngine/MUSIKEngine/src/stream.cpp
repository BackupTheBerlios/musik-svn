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

#include "stdafx.h"
#include "MUSIKEngine/inc/engine.h"
#include "MUSIKEngine/inc/stream.h"
#include "MUSIKEngine/inc/imusikstreamout.h"


class MetadataCallbackConnector : public IMUSIKStreamOut::IMetadataCallback
{
public:
    MetadataCallbackConnector(MUSIKStream *pStream,MUSIKStream::IMetadataCallback *pCB)
        :m_pMetaCb(pCB)
        ,m_pStream(pStream)
    {}
    virtual void MetadataCallback(const char *name, const char *value)
    {
        if(m_pMetaCb)
            m_pMetaCb->MetadataCallback(m_pStream,name,value);
    }

private:
    MUSIKStream::IMetadataCallback *m_pMetaCb;
    MUSIKStream *m_pStream;
};


MUSIKStream::MUSIKStream(MUSIKDecoder *pDecoder)
	:m_pDecoder(pDecoder)
{

}

MUSIKStream::~MUSIKStream()
{
	if(m_pDecoder->StreamOut())
    {
        m_pDecoder->StreamOut()->Close(); 
        m_pDecoder->StreamOut()->SetMetadataCallback(NULL); 

    }
	delete m_pDecoder;
}

bool MUSIKStream::Play()
{
	return m_pDecoder->Start();
}
void MUSIKStream::SetVolume(float v)
{
	m_pDecoder->StreamOut()->SetVolume(v);
}
float MUSIKStream::GetVolume()
{
	return m_pDecoder->StreamOut()->GetVolume();
}
bool MUSIKStream::CanSeek()
{
	return m_pDecoder->CanSeek();
}
bool MUSIKStream::SetTime( int nTimeMS)
{
	return m_pDecoder->SetTime(nTimeMS);
}

int64_t MUSIKStream::GetTime()
{
	return m_pDecoder->GetTime();
}

int64_t MUSIKStream::GetLengthMs()
{
	return m_pDecoder->GetLengthMs();
}

int64_t MUSIKStream::GetSampleCount()
{
	return m_pDecoder->GetSampleCount();
}

int64_t MUSIKStream::GetFilesize()
{
    return m_pDecoder->GetFilesize();
}

const char * MUSIKStream::Type()
{
	return m_pDecoder->Type();
}

bool MUSIKStream::SetPlayState( MUSIKEngine::PlayState state)
{
	if(m_pDecoder->StreamOut())
		return m_pDecoder->StreamOut()->SetPlayState(state); 
	return false;
}

MUSIKEngine::PlayState MUSIKStream::GetPlayState()
{
	if(m_pDecoder->StreamOut())
		return m_pDecoder->StreamOut()->GetPlayState(); 
	return MUSIKEngine::Invalid;
}
MUSIKDecoder::INFO * MUSIKStream::GetDecoderInfo()
{
	return m_pDecoder->GetInfo();
}


MUSIKEngine::Error MUSIKStream::SetMetadataCallback(IMetadataCallback *pCb)
{
    m_pDecoder->StreamOut()->SetMetadataCallback(pCb ? new MetadataCallbackConnector(this,pCb) : NULL) ;
    return MUSIKEngine::errNotSupported;
}

MUSIKEngine::Error MUSIKStream::GetOpenStatus(MUSIKEngine::OpenStatus *pStatus)
{
    return m_pDecoder->StreamOut()->GetOpenStatus(pStatus); 
}

MUSIKEngine::Error MUSIKStream::GetNetStatus(MUSIKEngine::NetStatus *pStatus,int * pnPercentRead,int * pnBitrate)
{
   return m_pDecoder->StreamOut()->GetNetStatus(pStatus,pnPercentRead,pnBitrate); 
}
