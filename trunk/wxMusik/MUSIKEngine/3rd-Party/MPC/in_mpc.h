#ifndef _in_mpc_h_
#define _in_mpc_h_

#ifndef _mpc_dec_h_
class Reader;
#endif
/************************ DEFINES *************************/
#define MPCDEC_VERSION             "0.98"

// error codes
#define ERROR_CODE_OK            0
#define ERROR_CODE_FILE         -1
#define ERROR_CODE_SV7BETA       1
#define ERROR_CODE_CBR           2
#define ERROR_CODE_IS            3
#define ERROR_CODE_BLOCKSIZE     4
#define ERROR_CODE_INVALIDSV     5


/************************ TYPEDEF *************************/
typedef long off_t;

typedef struct {
    double              SampleFreq;
    unsigned int        Channels;
    off_t               HeaderPosition;     // byte position of header
    unsigned int        StreamVersion;      // Streamversion of current file
    unsigned int        Bitrate;            // bitrate of current file (bps)
    double              AverageBitrate;     // Average bitrate in bits/sec
    unsigned int        Frames;             // number of frames contained
    __int64             PCMSamples;
    unsigned int        MaxBand;            // maximum band-index used (0...31)
    unsigned int        IS;                 // Intensity Stereo (0: off, 1: on)
    unsigned int        MS;                 // Mid/Side Stereo (0: off, 1: on)
    unsigned int        BlockSize;          // only needed for SV4...SV6 -> not supported
    unsigned int        Profile;            // quality profile
    const char*         ProfileName;

    // ReplayGain related data
    short               GainTitle;
    short               GainAlbum;
    unsigned short      PeakAlbum;
    unsigned short      PeakTitle;

    // true gapless stuff
    unsigned int        IsTrueGapless;      // is true gapless used?
    unsigned int        LastFrameSamples;   // number of valid samples within last frame

    unsigned int        EncoderVersion;     // version of encoder used
    char                Encoder[256];

    off_t TagOffset;
    off_t TotalFileLength;
} BasicData;


class StreamInfo
{
private:
    char * filename;
public://ahem fixme
    BasicData           simple;
public:
    StreamInfo();
    ~StreamInfo() {Clear();}
    void SetFilename(const char *);
    const char * GetFilename();
    void Clear();
    int ReadStreamInfo(Reader * fp);
private:
    //fixme: add writetag here too
    int ReadHeaderSV6 ( Reader* fp );
    int ReadHeaderSV7 ( Reader* fp );
    int ReadHeaderSV8 ( Reader* fp );
};


typedef struct {
    char                TitleFormat [1024];
    int                 EQbyMPC;
    int                 EQdB;
    int                 DisplayID3Names;
    unsigned int        UpdateBitrate;      // 0 = average, >0 average over such a number of frames
    int                 MaxBrokenFrames;
    int                 SkipTrackOnError;
    int                 ReplayGainMode;
    int                 ReplayGainHeadroom;
    int                 DitherUsed;
    int                 DebugUsed;
    char                HTTP_Proxy[256];
    int                 HTTP_ProxyMode;
    int                 HTTP_Prebuffer_Start;
    int                 HTTP_Prebuffer_Underrun;
    int                 HTTP_Buffer_Size;
} SettingsMPC;

/************************ GLOBALS *************************/
extern SettingsMPC PluginSettings;

#endif
