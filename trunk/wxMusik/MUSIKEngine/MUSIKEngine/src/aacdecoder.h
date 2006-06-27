//Copyright (c) 2004, Gunnar Roth
//All rights reserved.
//This code is derived from :

/* in_flac - Winamp2 AAC input plugin
* Copyright (C) 2000,2001,2002,2003,2004  Josh Coalson
* and from
* libxmms-flac - XMMS AAC input plugin
* Copyright (C) 2000,2001,2002,2003,2004  Josh Coalson
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef AACDECODER_H
#define AACDECODER_H


#ifdef MUSIKENGINE_DO_NOT_USE_OWN_DECODERS
#define MUSIKENGINE_NO_FAAD2_SUPPORT
#endif

#ifndef MUSIKENGINE_NO_FAAD2_SUPPORT
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "MUSIKEngine/inc/decoder.h"

#include "faad2/include/neaacdec.h"
#include "faad2/common/mp4ff/mp4ff.h"


#define AAC_SAMPLES_PER_WRITE 576

class MUSIKAACDecoder :public MUSIKDecoder
{
	class Reader
	{
	public:
		Reader(FILE * p_file,bool p_seekable) : m_file(p_file), m_seekable(p_seekable)
		{
			fseek(m_file,0,SEEK_END);
			m_size = ftell(m_file);
			fseek(m_file,0,SEEK_SET);
		}
		virtual ~Reader()
		{
			fclose(m_file);
		}
	    int32_t read ( void *ptr, int32_t size ) {return fread(ptr,1,size,m_file);}
		bool seek ( int32_t offset ) {return m_seekable ? !fseek(m_file,offset,SEEK_SET) : false;}
		int32_t tell () {return ftell(m_file);}
		int32_t get_size () {return m_size;}
		bool canseek() {return m_seekable;}
	private:
		FILE * m_file;
		long m_size;
		bool m_seekable;
	};

	struct AACStreamInfo
	{
        Reader *m_pReader;
        NeAACDecHandle decoder;
        
                
        uint8_t m_Channels;
        int64_t m_Milliseconds;
        int64_t m_Length;
        uint32_t m_SampleRate;
        uint32_t *m_pSeektable;
        uint32_t m_BitRate;
        uint32_t m_NumSamples;

//    private:    
        unsigned char fbuf[4608];
        unsigned long fbuflen;
        uint32_t m_initbytes;

        //MP4 stuff
        mp4ff_callback_t mp4cb;
        mp4ff_t * m_pMP4File;
        int mp4track;
        int32_t sampleid,timescale;
        
        char *tags; // iTunes/MP4 metadata
     public:       
		AACStreamInfo()
		{
			m_Channels = m_Milliseconds=m_Length=m_SampleRate=m_initbytes=0;
            m_pSeektable=NULL;
            m_pMP4File = NULL;
            tags = NULL;	
            m_pReader = NULL;
            decoder = NULL;
            fbuflen = 0;
		}
        
        ~AACStreamInfo()
        {
            Close();
        }
        bool Open(Reader *pReader)
        {
            decoder=NeAACDecOpen();
            m_pReader = pReader;
            fbuflen = 0;
            return true;
        }
        void Close()
        {
            if (m_pMP4File) 
                mp4ff_close(m_pMP4File);
            m_pMP4File = NULL;
            free(m_pSeektable);
            m_pSeektable = NULL;
            free(tags);
            tags = NULL;
            delete m_pReader;
            m_pReader = NULL;
            if(decoder)
                NeAACDecClose(decoder);
            decoder = NULL;    
        }
    protected:
        int FindAACIndex();
        uint32_t GetSampleRate(int index) const;
        int get_AAC_format();    
        int read_ADIF_header();
        int read_ADTS_header();
    friend class MUSIKAACDecoder;    
	};

	MUSIKAACDecoder(IMUSIKStreamOut * pIMUSIKStreamOut);

public:
	~MUSIKAACDecoder();

	virtual bool CanSeek()
	{
		return true;
	}

	bool Close();
	virtual const char * Type()
	{
		return "AAC/MP4 Audio Codec (AAC,MP4) File";
	}

protected:
	virtual bool OpenMedia(const char *FileName);

	int DecodeBlocks(unsigned char *buff,int len);
	bool DoSeek(int64_t samplepos);


private:
    bool CreateMP4Stream();
    bool CreateAACStream();

static uint32_t read_callback(void* user_data,void* buffer,uint32_t length)
{
    Reader * pReader = (Reader *)user_data;
    return pReader->read(buffer,length);
}

static uint32_t seek_callback(void* user_data,uint64_t position)
{
    Reader * pReader = (Reader *)user_data;
    return pReader->seek(position);
}



	AACStreamInfo m_AacInfo;

	friend class MUSIKEngine;
};
#endif //!MUSIKENGINE_NO_FAAD2_SUPPORT

#endif
