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


#include "MUSIKEngine/MUSIKEngine/inc/engine.h"
#include "MUSIKEngine/MUSIKEngine/inc/stream.h"
#include "MUSIKEngine/MUSIKEngine/inc/imusikstreamout.h"
MUSIKStream::MUSIKStream(MUSIKDecoder *pDecoder)
	:m_pDecoder(pDecoder)
{

}

MUSIKStream::~MUSIKStream()
{
	if(m_pDecoder->StreamOut())
		m_pDecoder->StreamOut()->Close(); 
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

int MUSIKStream::GetTime()
{
	return m_pDecoder->GetTime();
}

int MUSIKStream::GetLengthMs()
{
	return m_pDecoder->GetLengthMs();
}

int64_t MUSIKStream::GetLength()
{
	return m_pDecoder->GetLength();
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
