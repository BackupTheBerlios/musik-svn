// Modified for my own use from:
//
/* Winamp plugin for ".mpc Musepack", based on miniSDK by Nullsoft */
/* by Andree Buschmann, contact: Andree.Buschmann@web.de           */
/* --------- ".mpc Musepack" is a registered trademark  ---------- */


/************************ DEFINES ************************/
#define NCH_MAX         2                               // number of channels (only stereo!)
#define BPS             16                              // bits per sample (only 16 bps!)
#define BYTES           4                               // = (NCH_MAX*(BPS/8)) -> number of bytes needed for one stereo/16bit-sample
#define FRAMELEN        (36 * 32)                       // samples per frame


/*********************** INCLUDES ************************/
#include <windows.h>

#include <math.h>
#include <stdio.h>

#include <io.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/utime.h>
#include "in_mpc.h"


#include "requant.h"
#include "huffsv46.h"
#include "huffsv7.h"
#include "bitstream.h"
#include "mpc_dec.h"
#include "minimax.h"
#include <malloc.h>
#include "equalizer.h"

void Reader::idle() { Sleep (10); } // avoid windows.h in mpc_dec.h



/************************ VARIABLES *************************/
SettingsMPC      PluginSettings;                         // AB: PluginSettings holds the parameters for the plugin-configuration
BOOL             bUseUnicode;                            // UNICODE enabled OS


/********************** DECLARING FUNCTIONS **********************/
int           ReadFileHeader    ( const char*, StreamInfo* );




void GetPluginSettings ( void );


//---------------------------------------------------------------
// converts profile-index to string
//---------------------------------------------------------------
static const char*
Stringify ( unsigned int profile )            // profile is 0...15, where 7...13 is used
{
    static const char   na    [] = "INFO_NOT_AVAILABLE";
    static const char*  Names [] = {
        na, "'Unstable/Experimental'", na, na,
        na, "below 'Telephone'", "below 'Telephone'", "'Telephone'",
        "'Thumb'", "'Radio'", "'Standard'", "'Xtreme'",
        "'Insane'", "'BrainDead'", "above 'BrainDead'", "above 'BrainDead'"
    };

    return profile >= sizeof(Names)/sizeof(*Names)  ?  na  :  Names [profile];
}

// read information from SV8 header
int StreamInfo::ReadHeaderSV8 ( Reader* fp )
{
    
    return 0;
}

// read information from SV7 header
int StreamInfo::ReadHeaderSV7 ( Reader* fp )
{
    const long samplefreqs [4] = { 44100, 48000, 37800, 32000 };

    unsigned int    HeaderData [8];
    unsigned short  EstimatedPeakTitle = 0;

    if (simple.StreamVersion > 0x71 ) {
 
        return 0;
    }

    if ( fp->seek ( simple.HeaderPosition, SEEK_SET ) != 0 )         // seek to header start
        return ERROR_CODE_FILE;
    if ( fp->read ( HeaderData, sizeof HeaderData) != sizeof HeaderData )
        return ERROR_CODE_FILE;

    simple.Channels         = 2;
    simple.Bitrate          = 0;
    simple.Frames           =  HeaderData[1];
    simple.IS               = 0;
    simple.MS               = (HeaderData[2] >> 30) & 0x0001;
    simple.MaxBand          = (HeaderData[2] >> 24) & 0x003F;
    simple.BlockSize        = 1;
    simple.Profile          = (HeaderData[2] <<  8) >> 28;
    simple.ProfileName      = Stringify ( simple.Profile );

    simple.SampleFreq       = samplefreqs [(HeaderData[2]>>16) & 0x0003];

    EstimatedPeakTitle      =  HeaderData[2]        & 0xFFFF;         // read the ReplayGain data
    simple.GainTitle        = (HeaderData[3] >> 16) & 0xFFFF;
    simple.PeakTitle        =  HeaderData[3]        & 0xFFFF;
    simple.GainAlbum        = (HeaderData[4] >> 16) & 0xFFFF;
    simple.PeakAlbum        =  HeaderData[4]        & 0xFFFF;

    simple.IsTrueGapless    = (HeaderData[5] >> 31) & 0x0001;         // true gapless: used?
    simple.LastFrameSamples = (HeaderData[5] >> 20) & 0x07FF;         // true gapless: valid samples for last frame

    simple.EncoderVersion   = (HeaderData[6] >> 24) & 0x00FF;
    if ( simple.EncoderVersion == 0 ) {
        sprintf ( simple.Encoder, "Buschmann 1.7.0...9, Klemm 0.90...1.05" );
    } else {
        switch ( simple.EncoderVersion % 10 ) {
        case 0:
            sprintf ( simple.Encoder, "Release %u.%u", simple.EncoderVersion/100, simple.EncoderVersion/10%10 );
            break;
        case 2: case 4: case 6: case 8:
            sprintf ( simple.Encoder, "Beta %u.%02u", simple.EncoderVersion/100, simple.EncoderVersion%100 );
            break;
        default:
            sprintf ( simple.Encoder, "--Alpha-- %u.%02u", simple.EncoderVersion/100, simple.EncoderVersion%100 );
            break;
        }
    }

    if ( simple.PeakTitle == 0 )                                      // there is no correct PeakTitle contained within header
        simple.PeakTitle = (unsigned short)(EstimatedPeakTitle * 1.18);
    if ( simple.PeakAlbum == 0 )
        simple.PeakAlbum = simple.PeakTitle;                          // no correct PeakAlbum, use PeakTitle

    return ERROR_CODE_OK;
}

// read information from SV4-SV6 header
int StreamInfo::ReadHeaderSV6 ( Reader* fp )
{
    unsigned int    HeaderData [8];

    if ( fp->seek (  simple.HeaderPosition, SEEK_SET ) != 0 )         // seek to header start
        return ERROR_CODE_FILE;
    if ( fp->read ( HeaderData, sizeof HeaderData ) != sizeof HeaderData )
        return ERROR_CODE_FILE;

    simple.Bitrate          = (HeaderData[0] >> 23) & 0x01FF;         // read the file-header (SV6 and below)
    simple.IS               = (HeaderData[0] >> 22) & 0x0001;
    simple.MS               = (HeaderData[0] >> 21) & 0x0001;
    simple.StreamVersion    = (HeaderData[0] >> 11) & 0x03FF;
    simple.MaxBand          = (HeaderData[0] >>  6) & 0x001F;
    simple.BlockSize        = (HeaderData[0]      ) & 0x003F;
    simple.Profile          = 0;
    simple.ProfileName      = "INFO_NOT_AVAILABLE";
    if ( simple.StreamVersion >= 5 )
        simple.Frames       =  HeaderData[1];                         // 32 bit
    else
        simple.Frames       = (HeaderData[1]>>16);                    // 16 bit

    simple.GainTitle        = 0;                                      // not supported
    simple.PeakTitle        = 0;
    simple.GainAlbum        = 0;
    simple.PeakAlbum        = 0;

    simple.LastFrameSamples = 0;
    simple.IsTrueGapless    = 0;

    simple.EncoderVersion   = 0;
    simple.Encoder[0]       = '\0';

    if ( simple.StreamVersion == 7 ) return ERROR_CODE_SV7BETA;       // are there any unsupported parameters used?
    if ( simple.Bitrate       != 0 ) return ERROR_CODE_CBR;
    if ( simple.IS            != 0 ) return ERROR_CODE_IS;
    if ( simple.BlockSize     != 1 ) return ERROR_CODE_BLOCKSIZE;

    if ( simple.StreamVersion < 6 )                                   // Bugfix: last frame was invalid for up to SV5
        simple.Frames -= 1;

    simple.SampleFreq    = 44100;                                     // AB: used by all files up to SV7
    simple.Channels      = 2;

    if ( simple.StreamVersion < 4  ||  simple.StreamVersion > 7 )
        return ERROR_CODE_INVALIDSV;

    return ERROR_CODE_OK;
}
// searches for a ID3v2-tag and reads the length (in bytes) of it
// -1 on errors of any kind

static long JumpID3v2 ( Reader* fp )
{
	unsigned char  tmp [10];
	unsigned int   Unsynchronisation;   // ID3v2.4-flag
	unsigned int   ExtHeaderPresent;    // ID3v2.4-flag
	unsigned int   ExperimentalFlag;    // ID3v2.4-flag
	unsigned int   FooterPresent;       // ID3v2.4-flag
	long           ret;

	fp->read  ( tmp, sizeof(tmp) );

	// check id3-tag
	if ( 0 != memcmp ( tmp, "ID3", 3) )
		return 0;

	// read flags
	Unsynchronisation = tmp[5] & 0x80;
	ExtHeaderPresent  = tmp[5] & 0x40;
	ExperimentalFlag  = tmp[5] & 0x20;
	FooterPresent     = tmp[5] & 0x10;

	if ( tmp[5] & 0x0F )
		return -1;              // not (yet???) allowed
	if ( (tmp[6] | tmp[7] | tmp[8] | tmp[9]) & 0x80 )
		return -1;              // not allowed

	// read HeaderSize (syncsave: 4 * $0xxxxxxx = 28 significant bits)
	ret  = tmp[6] << 21;
	ret += tmp[7] << 14;
	ret += tmp[8] <<  7;
	ret += tmp[9]      ;
	ret += 10;
	if ( FooterPresent )
		ret += 10;

	return ret;
}

// reads file header and tags
int StreamInfo::ReadStreamInfo ( Reader* fp)
{
    unsigned int    HeaderData[1];
    int             Error = 0;

    memset ( &simple, 0, sizeof (BasicData) );                          // Reset Info-Data

    if ( (simple.HeaderPosition = JumpID3v2 (fp)) < 0 )                 // get header position
        return ERROR_CODE_FILE;

    if ( fp->seek ( simple.HeaderPosition, SEEK_SET ) != 0 )            // seek to first byte of mpc data
        return ERROR_CODE_FILE;
    if ( fp->read ( HeaderData, sizeof HeaderData ) != sizeof HeaderData )
        return ERROR_CODE_FILE;
    if ( fp->seek ( 0L, SEEK_END ) != 0 )                               // get filelength
        return ERROR_CODE_FILE;
    simple.TotalFileLength = fp->tell ();
    simple.TagOffset = simple.TotalFileLength;

    if ( memcmp ( HeaderData, "MP+", 3 ) == 0 ) {                       // check version
        simple.StreamVersion = HeaderData[0] >> 24;

        if ( (simple.StreamVersion & 15) >= 8 )                         // StreamVersion 8
            Error = ReadHeaderSV8 ( fp );
        else if ( (simple.StreamVersion & 15) == 7 )                    // StreamVersion 7
            Error = ReadHeaderSV7 ( fp );
    } else {                                                            // StreamVersion 4-6
        Error = ReadHeaderSV6 ( fp );
    }
    simple.PCMSamples = 1152 * simple.Frames - 576;                     // estimation, exact value needs too much time
    if ( simple.PCMSamples > 0 )
        simple.AverageBitrate = (simple.TagOffset - simple.HeaderPosition) * 8. * simple.SampleFreq / simple.PCMSamples;
    else
        simple.AverageBitrate = 0;

    return Error;
}

//---------------------------------------------------------------
// used for detecting URL streams.. unused here.
// strncmp(fn,"http://",7) to detect HTTP streams, etc
//---------------------------------------------------------------
int
isourfile ( char* fn )
{
    int    ret = 0;
#if 0
    FILE*  fp = fopen ( fn, "rb" );
    char   buff [4];

    if ( fp != NULL ) {
        if ( fread ( buff, 1, 4, fp ) == 4 )
            if ( memcmp ( buff, "MP+\007", 4) == 0 )
                ret++;
        fclose (fp);
    }
#endif

    return ret;
}

//---------------------------------------------------------------
// calculates the factor that must be applied to match the
// chosen ReplayGain-mode
//---------------------------------------------------------------
float
ProcessReplayGain ( int mode, StreamInfo *info )
{
    static unsigned char  modetab [7] = { 0,4,5,6,7,2,3 };
    static char           oldmessage [800];
    char                  message    [800];
    float                 factor_gain;
    float                 factor_clip;
    float                 factor_preamp;
    int                   Gain     = 0;
    int                   Peak     = 0;
    int                   Headroom = 0;
    char*                 p        = message;

    mode = modetab [mode];
  
    if ( info->simple.PeakAlbum == 0 )
        info->simple.PeakAlbum = info->simple.PeakTitle;      // Solche Sachen sollten einheitlich an einer Stelle gehandelt werden, und da ist der Headerparser wesentlich besser geeignet.
                                                // Wenn man defensiv versucht, Fehler an falschen Stellen zu kaschieren, schießt man sich in fortgeschrittenen Projekten mehrfach am Tag die Füße, Knie und die Eier weg.
                                                // Ausnahmen sind einzig Projekte, wo man Fehler nicht nachträglich mehr beheben kann (Flugzeuge, Auto, Raumfahrt, Medizin)

    if ( (mode & 1) == 0 )
        Gain = info->simple.GainTitle, Peak = info->simple.PeakTitle;
    else
        Gain = info->simple.GainAlbum, Peak = info->simple.PeakAlbum;

    if ( PluginSettings.DebugUsed ) {
        p += sprintf ( p, "UsedPeak: %u     UsedGain: %+5.2f\n", Peak, 0.01*Gain );
    }

    if ( (mode & 2) == 0 )
        Gain = 0;

    if ( (mode & 4) == 0 )
        Peak = 0;

    if ( mode == 0 )
        Headroom = 0;
    else
        Headroom = PluginSettings.ReplayGainHeadroom - 14;   // K-2...26, 2 => -12, 26 => +12

    // calculate multipliers for ReplayGain and ClippingPrevention
    factor_preamp = (float) pow (10., -0.05 * Headroom);
    factor_gain   = (float) pow (10., 0.0005 * Gain);
    factor_clip   = (float) (32767./( Peak + 1 ) );

    if ( factor_preamp * factor_gain < factor_clip )
        factor_clip  = 1.f;
    else
        factor_clip /= factor_preamp * factor_gain;


    return factor_preamp * factor_gain * factor_clip;
}


