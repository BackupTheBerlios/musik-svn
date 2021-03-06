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

#ifndef MPCDECODER_H
#define MPCDECODER_H
#include <stdio.h>
#include "MUSIKEngine/inc/decoder.h"

#ifdef MUSIKENGINE_DO_NOT_USE_OWN_DECODERS
#define MUSIKENGINE_NO_MPC_SUPPORT
#endif

#ifndef MUSIKENGINE_NO_MPC_SUPPORT


#include "mpcdec/mpcdec.h"


class MUSIKMPCDecoder: public MUSIKDecoder
{

	struct MPCStream
	{
        /*
          The data bundle we pass around with our reader to store file
          position and size etc. 
        */
        typedef struct reader_data_t {
            FILE *file;
            long size;
            mpc_bool_t seekable;
        } reader_data;

        MPCStream();
        bool Init(FILE * input);
        bool CanSeek() 
        {
            return reader.canseek(&data) != 0;
        }
        unsigned Decode()
        {
            return mpc_decoder_decode(&decoder,sample_buffer, 0, 0);
        }
        bool SeekSample(mpc_int64_t samplenr)
        {
            return  mpc_decoder_seek_sample(&decoder,samplenr);
        }
        void Close();
        
		MPC_SAMPLE_FORMAT sample_buffer[MPC_DECODER_BUFFER_LENGTH];
		mpc_streaminfo info;
    private:    
        mpc_decoder decoder;
        mpc_reader reader;
        reader_data data;
    private:
        static mpc_int32_t read_impl(void *data, void *ptr, mpc_int32_t size);
        static mpc_bool_t seek_impl(void *data, mpc_int32_t offset);
        static mpc_int32_t tell_impl(void *data);
        static mpc_int32_t get_size_impl(void *data);
        static mpc_bool_t canseek_impl(void *data);

    };

	MUSIKMPCDecoder(IMUSIKStreamOut * pIMUSIKStreamOut);
public:

	~MUSIKMPCDecoder();
	virtual bool CanSeek()
	{
		return m_MPCStream.CanSeek();
	}
	bool Close();
	virtual const char * Type()
	{
		return "Musepack";
	}
protected:
	virtual bool OpenMedia(const char *FileName);

	virtual int DecodeBlocks(unsigned char *buff,int len);
	virtual bool DoSeek(int64_t samplepos);

	int CopySamplesToBuffer(const MPC_SAMPLE_FORMAT * p_buffer,unsigned p_size,unsigned char *pDestbuf);


private:
	MPCStream m_MPCStream;

	friend class MUSIKEngine;
};

#endif //!MUSIKENGINE_NO_MPC_SUPPORT
#endif
