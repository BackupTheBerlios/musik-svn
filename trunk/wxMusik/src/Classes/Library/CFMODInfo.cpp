//--- For compilers that support precompilation, includes "wx/wx.h". ---//
#include "myprec.h"
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif 
#include "MusikDefines.h"

#include "CFMODInfo.h"


#ifdef USE_FMOD3
#include <fmod/fmod.h>


CFMODInfo::CFMODInfo()
{

}
bool CFMODInfo::ReadMetaData(CSongMetaData & MetaData) const
{
	FSOUND_STREAM *pStream = FSOUND_Stream_Open( ( const char* )ConvFn2A( MetaData.Filename.GetFullPath() ), FSOUND_2D, 0, 0 );

	if ( pStream  == NULL)
		return false;
	int numinfo = 0 ;
	MetaData.bVBR			=  false;

	FSOUND_Stream_GetNumTagFields(pStream, &numinfo);
	for (int i=0;i < numinfo;i++)
	{
		char *name, *value;
		int  type, length;

		if (FSOUND_Stream_GetTagField(pStream, i, &type, &name, (void**)&value, &length))
		{
			if(strcmp(name,"TITLE")==0)
				MetaData.Title = ConvFromISO8859_1ToUTF8(value);
			else if(strcmp(name,"AUTHOR")==0)
				MetaData.Artist = ConvFromISO8859_1ToUTF8(value);
			else if(strcmp(name,"DESCRIPTION")==0)
				MetaData.Notes = ConvFromISO8859_1ToUTF8(value);
			else if(strcmp(name,"WM/AlbumTitle")==0)
				MetaData.Album = ConvFromISO8859_1ToUTF8(value);
			else if(strcmp(name,"WM/Year")==0)
				MetaData.Year = ConvFromISO8859_1ToUTF8(value);
			else if(strcmp(name,"WM/Genre")==0)
				MetaData.Genre = ConvFromISO8859_1ToUTF8(value);
			else if(strcmp(name,"WM/TrackNumber")==0)
				MetaData.nTracknum = atoi(value);
			else if(strcmp(name,"IsVbr")==0 && length == sizeof(int))
				MetaData.bVBR	 = (*(int*)(value)) != 0;

		}
	}

	MetaData.nDuration_ms	=  FSOUND_Stream_GetLengthMs( pStream );
	MetaData.nFilesize		= FSOUND_Stream_GetLength( pStream );
	MetaData.nBitrate		= MetaData.nDuration_ms ? MetaData.nFilesize * 8 / MetaData.nDuration_ms : 0;
	FSOUND_Stream_Close( pStream );
	return true;
}

#else
// FMODEx
#ifdef WIN32
#pragma comment(lib,"fmodex_vc")
#endif
#include <fmodex/fmod.hpp>

CFMODInfo::CFMODInfo()
{
    m_pSystem = NULL;
    FMOD::System_Create(&m_pSystem);
    m_pSystem->init(100, FMOD_INIT_NORMAL, 0);

}
CFMODInfo::~CFMODInfo()
{
    if(m_pSystem)
    {
        m_pSystem->close();
        m_pSystem->release();
    }

}
bool CFMODInfo::ReadMetaData(CSongMetaData & MetaData) const
{
    if(!m_pSystem)
        return false;
    FMOD::Sound     *pSound = NULL;

    m_pSystem->createSound(( const char* )ConvFn2A( MetaData.Filename.GetFullPath()) , FMOD_SOFTWARE | FMOD_2D | FMOD_CREATESTREAM | FMOD_OPENONLY, 0, &pSound);   

    if ( pSound  == NULL)
        return false;
    MetaData.bVBR			=  false;

    for (;;)
    {
        FMOD_TAG         tag;
        if (pSound->getTag(0, -1, &tag) != FMOD_OK)
        {
            break;
        }
        if (tag.datatype == FMOD_TAGDATATYPE_STRING)
        {
            if(strcmp(tag.name,"TITLE")==0)
                MetaData.Title = ConvFromISO8859_1ToUTF8((const char *)tag.data);
            else if(strcmp(tag.name,"AUTHOR")==0 || strcmp(tag.name,"WM/AlbumArtist") == 0)
                MetaData.Artist = ConvFromISO8859_1ToUTF8((const char *)tag.data);
            else if(strcmp(tag.name,"DESCRIPTION")==0)
                MetaData.Notes = ConvFromISO8859_1ToUTF8((const char *)tag.data);
            else if(strcmp(tag.name,"WM/AlbumTitle")==0)
                MetaData.Album = ConvFromISO8859_1ToUTF8((const char *)tag.data);
            else if(strcmp(tag.name,"WM/Year")==0)
                MetaData.Year = ConvFromISO8859_1ToUTF8((const char *)tag.data);
            else if(strcmp(tag.name,"WM/Genre")==0)
                MetaData.Genre = ConvFromISO8859_1ToUTF8((const char *)tag.data);
        }
        else if(tag.datatype == FMOD_TAGDATATYPE_INT)
        {
            if(strcmp(tag.name,"WM/Track")==0 || strcmp(tag.name,"WM/TrackNumber")==0)
                MetaData.nTracknum = (*(int*)(tag.data));
            else if(strcmp(tag.name,"IsVbr")==0 )
                MetaData.bVBR	 = (*(int*)(tag.data)) != 0;
        }
    }
    pSound->getLength((unsigned int *)&MetaData.nDuration_ms,FMOD_TIMEUNIT_MS);
    pSound->release();
    pSound = NULL;
    wxStructStat fstatus;
    memset(&fstatus,0,sizeof(wxStructStat));
    wxStat(MetaData.Filename.GetFullPath(),&fstatus);
    MetaData.nFilesize		= fstatus.st_size;
    MetaData.nBitrate		= MetaData.nDuration_ms ? MetaData.nFilesize * 8 / MetaData.nDuration_ms : 0;

    return true;
}

#endif

