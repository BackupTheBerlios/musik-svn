//--- For compilers that support precompilation, includes "wx/wx.h". ---//
#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif 
#include "CFMODInfo.h"

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
	MetaData.nBitrate		= MetaData.nFilesize * 8 / MetaData.nDuration_ms  ;
	FSOUND_Stream_Close( pStream );
	return true;
}

