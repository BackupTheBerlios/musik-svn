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
#include <fmod_errors.h>
FMODEngine::FMODEngine(int nSampleRate, int nChannels, int nFlags)
{
	FSOUND_Init(nSampleRate,nChannels,nFlags);			
	FSOUND_SetBufferSize( 500 );
	m_OpenMode = OpenMode_Default;
	
}
IMUSIKStreamOut *FMODEngine::CreateStreamOut()
{
  return new FMODStreamOut(m_OpenMode);
}
MUSIKDefaultDecoder *FMODEngine::CreateDefaultDecoder()
{
	return  new	MUSIKDefaultDecoder(new FMODStreamOut(m_OpenMode));
}
char *FMODEngine::ErrorString(int errcode)
{
	return FMOD_ErrorString(errcode);
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
FMODEngine::~FMODEngine(void)
{	
	FSOUND_Close();
}
