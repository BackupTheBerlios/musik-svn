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

#ifndef MUSIKAPEDECODER_H
#define MUSIKAPEDECODER_H

#include "MUSIKEngine/inc/decoder.h"
class IAPEDecompress;

class MUSIKAPEDecoder : public MUSIKDecoder
{
	struct APEStreamInfo
	{
		IAPEDecompress *					pAPEDecompress;
	};

	MUSIKAPEDecoder(IMUSIKStreamOut * pIMUSIKStreamOut);
	~MUSIKAPEDecoder();
public:
	virtual bool CanSeek()
	{
		return true;
	}
	virtual int GetTime();
	virtual bool Close();
	INFO * GetInfo();
	virtual const char * Type()
	{
		return "Monkey's Audio File";
	}
protected:
	virtual bool OpenMedia(const char *FileName);
	virtual bool DoSeek(int nTimeMS);
	virtual int DecodeBlocks(unsigned char *buff,int len);
private:
	APEStreamInfo m_ApeInfo;

	friend class MUSIKEngine;

};

#endif //MUSIKAPEDECODER_H
