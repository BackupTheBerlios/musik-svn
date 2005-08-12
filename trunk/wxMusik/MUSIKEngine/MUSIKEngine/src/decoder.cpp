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

#include "MUSIKEngine/inc/decoder.h"
#include "MUSIKEngine/inc/imusikstreamout.h"

MUSIKDecoder::~MUSIKDecoder()
{
	Close();
	delete m_pIMUSIKStreamOut;
	free(m_sample_buffer);

}


bool MUSIKDecoder::CreateBuffer(int decoderbuffersize)
{
	if(m_sample_buffer)
		free(m_sample_buffer);
	m_sample_buffer = (unsigned char*) malloc(decoderbuffersize);
	m_sample_buffer_size = decoderbuffersize;
	if(m_sample_buffer == NULL)
		return false;
	m_sample_buffer_filled = 0;

	return true;
}

bool MUSIKDecoder::DoFillBuffer(unsigned char * buff,int len)
{
    if(!SeekIfNeeded())
    {
        memset(buff,0,len);
        return false;
    }
	int buff_filled = 0;
	if(len >= m_sample_buffer_filled)
	{

		while (len >= m_sample_buffer_filled)
		{
            if(!SeekIfNeeded() )
            {
                memset(buff,0,len);
                return false;
            }
            if(m_sample_buffer_filled > 0)
            {
                memcpy(buff + buff_filled, m_sample_buffer, m_sample_buffer_filled); /* copy samples to buff */
                buff_filled += m_sample_buffer_filled;
                len -= m_sample_buffer_filled;
            }
			m_sample_buffer_filled = DecodeBlocks(m_sample_buffer,m_sample_buffer_size);
			if(m_sample_buffer_filled <= 0)
			{
                m_sample_buffer_filled = 0;
				if(len)
					memset(buff + buff_filled,0,len);// add silence, to the rest of the buffer
				SetDecodeSamplePos(GetSampleCount());
				return false;// we are at the end
			}
		}
	}
	if(len > 0)
	{
		memcpy(buff + buff_filled, m_sample_buffer, len); /* copy samples to buff */
		m_sample_buffer_filled -=len;
		if(m_sample_buffer_filled > 0)
        {
			memmove( m_sample_buffer,((unsigned char*)m_sample_buffer) + len, m_sample_buffer_filled );
        }
	}
	return true;
}
bool MUSIKDecoder::SeekIfNeeded()
{
	if ( m_seek_needed != -1 ) 
	{                          // SEEK
		bool bRes = DoSeek(m_seek_needed);
		m_seek_needed = -1;
		if(bRes == false)
		{
			SetDecodeSamplePos(GetSampleCount());
			return false;// stop
		}
		m_sample_buffer_filled = 0; // reset buffer, because we did a seek. so buffer will be filled with the new data
	}
    return true;
}

bool MUSIKDecoder::Start()
{	
	if(m_decode_pos > 0)
	{
		if(!DoSeek(0))
			return false;
	}
	m_sample_buffer_filled = DecodeBlocks(m_sample_buffer,m_sample_buffer_size); //pre-buffer
	if(!m_pIMUSIKStreamOut->Create(this))
		return false;

	return m_pIMUSIKStreamOut->Start();
}


bool MUSIKDecoder::Close()
{
	return m_pIMUSIKStreamOut->Close();
}

