#define DECODER
#define WINAMP
#define SYNTH_DELAY     481
#define FRAMELEN        (36 * 32)                       // samples per frame

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <malloc.h>
#include "minimax.h"
#include "mpc_dec.h"
#include "requant.h"
#include "huffsv46.h"
#include "huffsv7.h"
#include "synth_filter.h"
#include "bitstream.h"
#include "in_mpc.h"
#include "equalizer.h"

// Klemm settings
#define OTHER_SEEK

/* F U N C T I O N S */


void
MPC_decoder::Reset_Globals ( void )
{
    Reset_BitstreamDecode ();

    DecodedFrames = 0;
    StreamVersion = 0;
    MS_used       = 0;
    eof = 0;

    memset ( SAVE            , 0, sizeof SAVE             );
    memset ( SCF_Index       , 0, sizeof SCF_Index        );
    memset ( Res             , 0, sizeof Res              );
    memset ( SCFI            , 0, sizeof SCFI             );
    memset ( DSCF_Flag       , 0, sizeof DSCF_Flag        );
    memset ( DSCF_Reference  , 0, sizeof DSCF_Reference   );
    memset ( Q               , 0, sizeof Q                );
    memset ( MS_Flag         , 0, sizeof MS_Flag          );
}

#if 0
void
MPC_decoder::perform_EQ ( void )
{
#if 0
    static float  SAVE_L     [DELAY] [32];                      // buffer for ...
    static float  SAVE_R     [DELAY] [32];                      // ... upper subbands
    static float  FirSave_L  [FIR_BANDS] [EQ_TAP];              // buffer for ...
    static float  FirSave_R  [FIR_BANDS] [EQ_TAP];              // ... lowest subbands
#endif
    float         lowestSB_L [FIR_BANDS] [36];
    float         lowestSB_R [FIR_BANDS] [36];
    float         SWAP       [DELAY] [32];
    int           n;
    int           k;
    int           i;
    float         tmp;

    for ( i = 0; i < FIR_BANDS; i++ )                           // L: delay subbands (synchronize FIR-filtered and gained subbands-samples)
        for ( k = 0; k < 36; k++ )
            lowestSB_L [i] [k] = Y_L [k] [i];

    memcpy  ( SWAP,       SAVE_L,       DELAY        * 32 * sizeof(float) );
    memcpy  ( SAVE_L,     Y_L+36-DELAY, DELAY        * 32 * sizeof(float) );
    memmove ( Y_L+DELAY,  Y_L,          (36 - DELAY) * 32 * sizeof(float) );
    memcpy  ( Y_L,        SWAP,         DELAY        * 32 * sizeof(float) );

    for ( i = 0; i < FIR_BANDS; i++ )                           // R: delay subbands (synchronize FIR-filtered and gained subbands-samples)
        for ( k = 0; k < 36; k++ )
            lowestSB_R [i] [k] = Y_R [k] [i];

    memcpy  ( SWAP,       SAVE_R,       DELAY        * 32 * sizeof(float) );
    memcpy  ( SAVE_R,     Y_R+36-DELAY, DELAY        * 32 * sizeof(float) );
    memmove ( Y_R+DELAY,  Y_R,          (36 - DELAY) * 32 * sizeof(float) );
    memcpy  ( Y_R,        SWAP,         DELAY        * 32 * sizeof(float) );

    for ( k = 0; k < 36; k++ ) {                                // apply global EQ to upper subbands
        for ( n = FIR_BANDS; n <= Max_Band; n++ ) {
            Y_L [k] [n] *= EQ_gain [n-FIR_BANDS];
            Y_R [k] [n] *= EQ_gain [n-FIR_BANDS];
        }
    }

    for ( i = 0; i < FIR_BANDS; i++ ) {                         // apply FIR to lower subbands for each channel
        for ( k = 0; k < EQ_TAP; k++ ) {                        // L: perform filter for lowest subbands
            tmp = 0.;
            for ( n = 0; n < EQ_TAP - k; n++ )
                tmp += FirSave_L  [i] [k+n]        * EQ_Filter [i] [n];
            for ( ; n < EQ_TAP; n++ )
                tmp += lowestSB_L [i] [k+n-EQ_TAP] * EQ_Filter [i] [n];
            Y_L [k] [i] = tmp;
        }
        for ( ; k < 36; k++ ) {                                 // L: perform filter for lowest subbands
            tmp = 0.;
            for ( n = 0; n < EQ_TAP; n++ )
                tmp += lowestSB_L [i] [k+n-EQ_TAP] * EQ_Filter [i] [n];
            Y_L [k] [i] = tmp;
        }
        for ( n = 0; n < EQ_TAP; n++ )
            FirSave_L [i] [n] = lowestSB_L [i] [36-EQ_TAP+n];

        for ( k = 0; k < EQ_TAP; k++ ) {                        // R: perform filter for lowest subbands
            tmp = 0.;
            for ( n = 0; n < EQ_TAP - k; n++ )
                tmp += FirSave_R  [i] [k+n]        * EQ_Filter [i] [n];
            for ( ; n < EQ_TAP; n++ )
                tmp += lowestSB_R [i] [k+n-EQ_TAP] * EQ_Filter [i] [n];
            Y_R [k] [i] = tmp;
        }
        for ( ; k < 36; k++ ) {                                 // R: perform filter for lowest subbands
            tmp = 0.;
            for ( n = 0; n < EQ_TAP; n++ )
                tmp += lowestSB_R [i] [k+n-EQ_TAP] * EQ_Filter [i] [n];
            Y_R [k] [i] = tmp;
        }
        for ( n = 0; n < EQ_TAP; n++ )
            FirSave_R [i] [n] = lowestSB_R [i] [36-EQ_TAP+n];
    }

    return;
}
#endif

int
MPC_decoder::Read_Bitstream ( int SV )
{
    long  EncodedBlockLength; // in bits
    unsigned long  CurrentDecoderPosition; // in bits
    
    CurrentDecoderPosition = BitsRead ();
    RecommendedResyncPos   = ( Zaehler << 5 ) + pos;
    
    if ( SV & 0x08 )
        EncodedBlockLength = 16 + Bitstream_read (16) * 8;
    else
        EncodedBlockLength = 20 + Bitstream_read (20);
    
    RecommendedResyncPos += EncodedBlockLength;     // ActDecodePos + FwdJmpInfo: New decoder pos after minor Sync errors with intact framing but defect frames

    switch ( SV ) {
    case 0x04:
    case 0x05:
    case 0x06:
        Read_Bitstream_SV6 ();
        break;
    case 0x07:
    case 0x17:
    case 0x27:
        Read_Bitstream_SV7 ();
        break;
    case 0x08:
        //Read_Bitstream_SV8 ();
        break;
    }        

    // printf ( "%5u / %5u\n", BitsRead () - CurrentDecoderPosition, EncodedBlockLength );
    
    return BitsRead () - CurrentDecoderPosition == EncodedBlockLength  ?  +EncodedBlockLength  :  -EncodedBlockLength;
}

int
MPC_decoder::Read_Bitstream_Jumper ( int SV )
{
    if ( SV & 0x08 )
        return 16 + Bitstream_read (16) * 8;
    else
        return 20 + Bitstream_read (20);
}

int
MPC_decoder::DecodeFrame ( char* buffer )
{
    unsigned int  FrameBitCnt = 0;
    int           tmp;
    int           last_samples;
    unsigned int  EQ    = PluginSettings.EQbyMPC && is_EQ_Activated ();
    unsigned int  frame = DecodedFrames;

    FrameWasValid = 1;

    if ( frame >= OverallFrames )
        return -1;              	// EOF: return -1

    // decode data and check for validity of frame
    tmp = Read_Bitstream (StreamVersion);
    SeekTable [DecodedFrames++] = (unsigned short)tmp == tmp  ?  tmp  :  0;
    FrameWasValid &= tmp >= 0;

    // synthesize signal
    Requantisierung     ( Max_Band );
    if ( PluginSettings.EQbyMPC )
        Do_Perform_Equalizer (synthFilter.YY, SAVE, Max_Band, 2); //perform_EQ ();
    if(PluginSettings.DitherUsed)
		synthFilter.Synthese_Filter_dithered( (short*)buffer + 0*2*1152 );
	else
		synthFilter.Synthese_Filter_opt( (short*)buffer + 0*2*1152 );

    // cut off first SYNTH_DELAY zero-samples
    if ( frame <  EQ )  							// frame 0 + EQ on
        return 0;

    if ( frame == EQ ) {							// frame 0 + EQ off  OR  frame 1 + EQ on
        memmove ( buffer, buffer + 2*2*SYNTH_DELAY, 2*2*(1152-SYNTH_DELAY) );
        return 1152 - SYNTH_DELAY;
    }

    if ( frame == OverallFrames-1  &&  StreamVersion >= 7 ) {               	// last frame, only exactly handled with SV 7
        last_samples = Bitstream_read (11);
        if ( last_samples == 0 )  
	    last_samples = 1152;	// encoder bugfix
        last_samples += SYNTH_DELAY;

        if ( TrueGaplessPresent  &&  last_samples > 1152 ) {
            tmp = Read_Bitstream (StreamVersion);
            SeekTable [DecodedFrames++] = (unsigned short)tmp == tmp  ?  tmp  :  0;
            FrameWasValid &= tmp >= 0;
        }
        else {
            synthFilter.Reset_Y ();     // better ???
        }

        Requantisierung     ( Max_Band );
        if ( PluginSettings.EQbyMPC )
            Do_Perform_Equalizer (synthFilter.YY, SAVE, Max_Band, 2);
		if(PluginSettings.DitherUsed)
			synthFilter.Synthese_Filter_dithered( (short*)buffer + 1*2*1152 );
		else
			synthFilter.Synthese_Filter_opt( (short*)buffer + 1*2*1152 );

        if ( EQ ) {
            synthFilter.Reset_Y ();     // better ???
            Requantisierung     ( Max_Band );
            Do_Perform_Equalizer (synthFilter.YY, SAVE, Max_Band, 2);
			if(PluginSettings.DitherUsed)
				synthFilter.Synthese_Filter_dithered( (short*)buffer + 2*2*1152 );
			else
				synthFilter.Synthese_Filter_opt( (short*)buffer + 2*2*1152 );

            last_samples += 1152;
        }
        return last_samples;
    }

    return 1152;
}

void
MPC_decoder::Requantisierung ( const int Last_Band )
{
    int     Band;
    int     n;
    float   facL;
    float   facR;
    float   templ;
    float   tempr;
    float*  YL;
    float*  YR;
    int*    L;
    int*    R;

    // requantization and scaling of subband-samples
    for ( Band = 0; Band <= Last_Band; Band++ ) {   // setting pointers
        YL = synthFilter.YY [0] [0] + Band;
        YR = synthFilter.YY [1] [0] + Band;
        L  = Q [Band].L;
        R  = Q [Band].R;
        /************************** MS-coded **************************/
        if ( MS_Flag [Band] ) {
            if ( Res[0] [Band] ) {
                if ( Res[1] [Band] ) {    // M!=0, S!=0
                    facL = Cc7[Res[0][Band]] * SCF[(unsigned char)SCF_Index[0][Band][0]];
                    facR = Cc7[Res[1][Band]] * SCF[(unsigned char)SCF_Index[1][Band][0]];
                    for ( n = 0; n < 12; n++, YL += 32, YR += 32 ) {
                        *YL   = (templ = *L++ * facL)+(tempr = *R++ * facR);
                        *YR   = templ - tempr;
                    }
                    facL = Cc7[Res[0][Band]] * SCF[(unsigned char)SCF_Index[0][Band][1]];
                    facR = Cc7[Res[1][Band]] * SCF[(unsigned char)SCF_Index[1][Band][1]];
                    for ( ; n < 24; n++, YL += 32, YR += 32 ) {
                        *YL   = (templ = *L++ * facL)+(tempr = *R++ * facR);
                        *YR   = templ - tempr;
                    }
                    facL = Cc7[Res[0][Band]] * SCF[(unsigned char)SCF_Index[0][Band][2]];
                    facR = Cc7[Res[1][Band]] * SCF[(unsigned char)SCF_Index[1][Band][2]];
                    for ( ; n < 36; n++, YL += 32, YR += 32 ) {
                        *YL   = (templ = *L++ * facL)+(tempr = *R++ * facR);
                        *YR   = templ - tempr;
                    }
                } else {    // M!=0, S==0
                    facL = Cc7[Res[0][Band]] * SCF[(unsigned char)SCF_Index[0][Band][0]];
                    for ( n = 0; n < 12; n++, YL += 32, YR += 32 ) {
                        *YR = *YL = *L++ * facL;
                    }
                    facL = Cc7[Res[0][Band]] * SCF[(unsigned char)SCF_Index[0][Band][1]];
                    for ( ; n < 24; n++, YL += 32, YR += 32 ) {
                        *YR = *YL = *L++ * facL;
                    }
                    facL = Cc7[Res[0][Band]] * SCF[(unsigned char)SCF_Index[0][Band][2]];
                    for ( ; n < 36; n++, YL += 32, YR += 32 ) {
                        *YR = *YL = *L++ * facL;
                    }
                }
            } else {
                if (Res[1][Band])    // M==0, S!=0
                {
                    facR = Cc7[Res[1][Band]] * SCF[(unsigned char)SCF_Index[1][Band][0]];
                    for ( n = 0; n < 12; n++, YL += 32, YR += 32 ) {
                        *YR = - (*YL = *(R++) * facR);
                    }
                    facR = Cc7[Res[1][Band]] * SCF[(unsigned char)SCF_Index[1][Band][1]];
                    for ( ; n < 24; n++, YL += 32, YR += 32 ) {
                        *YR = - (*YL = *(R++) * facR);
                    }
                    facR = Cc7[Res[1][Band]] * SCF[(unsigned char)SCF_Index[1][Band][2]];
                    for ( ; n < 36; n++, YL += 32, YR += 32 ) {
                        *YR = - (*YL = *(R++) * facR);
                    }
                } else {    // M==0, S==0
                    for ( n = 0; n < 36; n++, YL += 32, YR += 32 ) {
                        *YR = *YL = 0.f;
                    }
                }
            }
        }
        /************************** LR-coded **************************/
        else {
            if ( Res[0] [Band] ) {
                if ( Res[1] [Band] ) {    // L!=0, R!=0
                    facL = Cc7[Res[0][Band]] * SCF[(unsigned char)SCF_Index[0][Band][0]];
                    facR = Cc7[Res[1][Band]] * SCF[(unsigned char)SCF_Index[1][Band][0]];
                    for (n = 0; n < 12; n++, YL += 32, YR += 32 ) {
                        *YL = *L++ * facL;
                        *YR = *R++ * facR;
                    }
                    facL = Cc7[Res[0][Band]] * SCF[(unsigned char)SCF_Index[0][Band][1]];
                    facR = Cc7[Res[1][Band]] * SCF[(unsigned char)SCF_Index[1][Band][1]];
                    for (; n < 24; n++, YL += 32, YR += 32 ) {
                        *YL = *L++ * facL;
                        *YR = *R++ * facR;
                    }
                    facL = Cc7[Res[0][Band]] * SCF[(unsigned char)SCF_Index[0][Band][2]];
                    facR = Cc7[Res[1][Band]] * SCF[(unsigned char)SCF_Index[1][Band][2]];
                    for (; n < 36; n++, YL += 32, YR += 32 ) {
                        *YL = *L++ * facL;
                        *YR = *R++ * facR;
                    }
                } else {     // L!=0, R==0
                    facL = Cc7[Res[0][Band]] * SCF[(unsigned char)SCF_Index[0][Band][0]];
                    for ( n = 0; n < 12; n++, YL += 32, YR += 32 ) {
                        *YL = *L++ * facL;
                        *YR = 0.f;
                    }
                    facL = Cc7[Res[0][Band]] * SCF[(unsigned char)SCF_Index[0][Band][1]];
                    for ( ; n < 24; n++, YL += 32, YR += 32 ) {
                        *YL = *L++ * facL;
                        *YR = 0.f;
                    }
                    facL = Cc7[Res[0][Band]] * SCF[(unsigned char)SCF_Index[0][Band][2]];
                    for ( ; n < 36; n++, YL += 32, YR += 32 ) {
                        *YL = *L++ * facL;
                        *YR = 0.f;
                    }
                }
            }
            else {
                if ( Res[1] [Band] ) {    // L==0, R!=0
                    facR = Cc7[Res[1][Band]] * SCF[(unsigned char)SCF_Index[1][Band][0]];
                    for ( n = 0; n < 12; n++, YL += 32, YR += 32 ) {
                        *YL = 0.f;
                        *YR = *R++ * facR;
                    }
                    facR = Cc7[Res[1][Band]] * SCF[(unsigned char)SCF_Index[1][Band][1]];
                    for ( ; n < 24; n++, YL += 32, YR += 32 ) {
                        *YL = 0.f;
                        *YR = *R++ * facR;
                    }
                    facR = Cc7[Res[1][Band]] * SCF[(unsigned char)SCF_Index[1][Band][2]];
                    for ( ; n < 36; n++, YL += 32, YR += 32 ) {
                        *YL = 0.f;
                        *YR = *R++ * facR;
                    }
                } else {    // L==0, R==0
                    for ( n = 0; n < 36; n++, YL += 32, YR += 32 ) {
                        *YR = *YL = 0.f;
                    }
                }
            }
        }
    }
}


/****************************************** SV 6 ******************************************/
void
MPC_decoder::Read_Bitstream_SV6 ( void )
{
    int n,k;
    int Max_used_Band=0;
    HuffmanTyp *Table;
    const HuffmanTyp *x1;
    const HuffmanTyp *x2;
    int *L;
    int *R;
    int *ResL = Res[0];
    int *ResR = Res[1];

    /************************ HEADER **************************/
    ResL = Res[0];
    ResR = Res[1];
    for (n=0; n<=Max_Band; ++n, ++ResL, ++ResR)
    {
        if      (n<11)           Table = Region_A;
        else if (n>=11 && n<=22) Table = Region_B;
        else /*if (n>=23)*/      Table = Region_C;

        *ResL = Q_res[n][Huffman_Decode(Table)];
        if (MS_used)      MS_Flag[n] = Bitstream_read(1);
        *ResR = Q_res[n][Huffman_Decode(Table)];

        // only perform the following procedure up to the maximum non-zero subband
        if (*ResL || *ResR) Max_used_Band = n;
    }

    /************************* SCFI-Bundle *****************************/
    ResL = Res[0];
    ResR = Res[1];
    for (n=0; n<=Max_used_Band; ++n, ++ResL, ++ResR) {
        if (*ResL) SCFI_Bundle_read(SCFI_Bundle, &SCFI[0][n], &DSCF_Flag[0][n]);
        if (*ResR) SCFI_Bundle_read(SCFI_Bundle, &SCFI[1][n], &DSCF_Flag[1][n]);
    }

    /***************************** SCFI ********************************/
    ResL = Res[0];
    ResR = Res[1];
    L    = SCF_Index[0][0];
    R    = SCF_Index[1][0];
    for (n=0; n<=Max_used_Band; ++n, ++ResL, ++ResR, L+=3, R+=3)
    {
        if (*ResL)
        {
            /*********** DSCF ************/
            if (DSCF_Flag[0][n]==1)
            {
                L[2] = DSCF_Reference[0][n];
                switch (SCFI[0][n])
                {
                case 3:
                    L[0] = L[2] + Huffman_Decode_fast(DSCF_Entropie);
                    L[1] = L[0];
                    L[2] = L[1];
                    break;
                case 1:
                    L[0] = L[2] + Huffman_Decode_fast(DSCF_Entropie);
                    L[1] = L[0] + Huffman_Decode_fast(DSCF_Entropie);
                    L[2] = L[1];
                    break;
                case 2:
                    L[0] = L[2] + Huffman_Decode_fast(DSCF_Entropie);
                    L[1] = L[0];
                    L[2] = L[1] + Huffman_Decode_fast(DSCF_Entropie);
                    break;
                case 0:
                    L[0] = L[2] + Huffman_Decode_fast(DSCF_Entropie);
                    L[1] = L[0] + Huffman_Decode_fast(DSCF_Entropie);
                    L[2] = L[1] + Huffman_Decode_fast(DSCF_Entropie);
                    break;
                default:
                    return;
                    break;
                }
            }
            /************ SCF ************/
            else
            {
                switch (SCFI[0][n])
                {
                case 3:
                    L[0] = Bitstream_read(6);
                    L[1] = L[0];
                    L[2] = L[1];
                    break;
                case 1:
                    L[0] = Bitstream_read(6);
                    L[1] = Bitstream_read(6);
                    L[2] = L[1];
                    break;
                case 2:
                    L[0] = Bitstream_read(6);
                    L[1] = L[0];
                    L[2] = Bitstream_read(6);
                    break;
                case 0:
                    L[0] = Bitstream_read(6);
                    L[1] = Bitstream_read(6);
                    L[2] = Bitstream_read(6);
                    break;
                default:
                    return;
                    break;
                }
            }
            // update Reference for DSCF
            DSCF_Reference[0][n] = L[2];
        }
        if (*ResR)
        {
            R[2] = DSCF_Reference[1][n];
            /*********** DSCF ************/
            if (DSCF_Flag[1][n]==1)
            {
                switch (SCFI[1][n])
                {
                case 3:
                    R[0] = R[2] + Huffman_Decode_fast(DSCF_Entropie);
                    R[1] = R[0];
                    R[2] = R[1];
                    break;
                case 1:
                    R[0] = R[2] + Huffman_Decode_fast(DSCF_Entropie);
                    R[1] = R[0] + Huffman_Decode_fast(DSCF_Entropie);
                    R[2] = R[1];
                    break;
                case 2:
                    R[0] = R[2] + Huffman_Decode_fast(DSCF_Entropie);
                    R[1] = R[0];
                    R[2] = R[1] + Huffman_Decode_fast(DSCF_Entropie);
                    break;
                case 0:
                    R[0] = R[2] + Huffman_Decode_fast(DSCF_Entropie);
                    R[1] = R[0] + Huffman_Decode_fast(DSCF_Entropie);
                    R[2] = R[1] + Huffman_Decode_fast(DSCF_Entropie);
                    break;
                default:
                    return;
                    break;
                }
            }
            /************ SCF ************/
            else
            {
                switch (SCFI[1][n])
                {
                case 3:
                    R[0] = Bitstream_read(6);
                    R[1] = R[0];
                    R[2] = R[1];
                    break;
                case 1:
                    R[0] = Bitstream_read(6);
                    R[1] = Bitstream_read(6);
                    R[2] = R[1];
                    break;
                case 2:
                    R[0] = Bitstream_read(6);
                    R[1] = R[0];
                    R[2] = Bitstream_read(6);
                    break;
                case 0:
                    R[0] = Bitstream_read(6);
                    R[1] = Bitstream_read(6);
                    R[2] = Bitstream_read(6);
                    break;
                default:
                    return;
                    break;
                }
            }
            // update Reference for DSCF
            DSCF_Reference[1][n] = R[2];
        }
    }

    /**************************** Samples ****************************/
    ResL = Res[0];
    ResR = Res[1];
    for (n=0; n<=Max_used_Band; ++n, ++ResL, ++ResR)
    {
        // setting pointers
        x1 = SampleHuff[*ResL];
        x2 = SampleHuff[*ResR];
        L = Q[n].L;
        R = Q[n].R;

        if (x1!=NULL || x2!=NULL)
            for (k=0; k<36; ++k)
            {
                if (x1 != NULL) *L++ = Huffman_Decode_fast (x1);
                if (x2 != NULL) *R++ = Huffman_Decode_fast (x2);
            }

        if (*ResL>7 || *ResR>7)
            for (k=0; k<36; ++k)
            {
                if (*ResL>7) *L++ = (int)Bitstream_read(*ResL-1) - Dc7[*ResL];
                if (*ResR>7) *R++ = (int)Bitstream_read(*ResR-1) - Dc7[*ResR];

                //if (*ResL>7) *L++ = (int)Bitstream_read(Res_bit[*ResL]) - Dc7[*ResL];
                //if (*ResR>7) *R++ = (int)Bitstream_read(Res_bit[*ResR]) - Dc7[*ResR];
            }
    }
}


/****************************************** SV 7 ******************************************/
void
MPC_decoder::Read_Bitstream_SV7 ( void )
{
    // these arrays hold decoding results for bundled quantizers (3- and 5-step)
    static int idx30[] = { -1, 0, 1,-1, 0, 1,-1, 0, 1,-1, 0, 1,-1, 0, 1,-1, 0, 1,-1, 0, 1,-1, 0, 1,-1, 0, 1};
    static int idx31[] = { -1,-1,-1, 0, 0, 0, 1, 1, 1,-1,-1,-1, 0, 0, 0, 1, 1, 1,-1,-1,-1, 0, 0, 0, 1, 1, 1};
    static int idx32[] = { -1,-1,-1,-1,-1,-1,-1,-1,-1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1};
    static int idx50[] = { -2,-1, 0, 1, 2,-2,-1, 0, 1, 2,-2,-1, 0, 1, 2,-2,-1, 0, 1, 2,-2,-1, 0, 1, 2};
    static int idx51[] = { -2,-2,-2,-2,-2,-1,-1,-1,-1,-1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2};

    int n,k;
    int Max_used_Band=0;
    const HuffmanTyp *Table;
    int idx;
    int *L   ,*R;
    int *ResL,*ResR;
    unsigned int tmp;

    /***************************** Header *****************************/
    ResL  = Res[0];
    ResR  = Res[1];

    // first subband
    *ResL = Bitstream_read(4);
    *ResR = Bitstream_read(4);
    if (MS_used && !(*ResL==0 && *ResR==0)) MS_Flag[0] = Bitstream_read(1);

    // consecutive subbands
    ++ResL; ++ResR; // increase pointers
    for (n=1; n<=Max_Band; ++n, ++ResL, ++ResR)
    {
        idx   = Huffman_Decode_fast(HuffHdr);
        *ResL = (idx!=4) ? *(ResL-1) + idx : Bitstream_read(4);

        idx   = Huffman_Decode_fast(HuffHdr);
        *ResR = (idx!=4) ? *(ResR-1) + idx : Bitstream_read(4);

        if (MS_used && !(*ResL==0 && *ResR==0)) MS_Flag[n] = Bitstream_read(1);

        // only perform following procedures up to the maximum non-zero subband
        if (*ResL!=0 || *ResR!=0) Max_used_Band = n;
    }
    /****************************** SCFI ******************************/
    L     = SCFI[0];
    R     = SCFI[1];
    ResL  = Res[0];
    ResR  = Res[1];
    for (n=0; n<=Max_used_Band; ++n, ++L, ++R, ++ResL, ++ResR) {
        if (*ResL) *L = Huffman_Decode_faster(HuffSCFI);
        if (*ResR) *R = Huffman_Decode_faster(HuffSCFI);
    }

    /**************************** SCF/DSCF ****************************/
    ResL  = Res[0];
    ResR  = Res[1];
    L     = SCF_Index[0][0];
    R     = SCF_Index[1][0];
    for (n=0; n<=Max_used_Band; ++n, ++ResL, ++ResR, L+=3, R+=3) {
        if (*ResL)
        {
            L[2] = DSCF_Reference[0][n];
            switch (SCFI[0][n])
            {
                case 1:
                    idx  = Huffman_Decode_fast(HuffDSCF);
                    L[0] = (idx!=8) ? L[2] + idx : Bitstream_read(6);
                    idx  = Huffman_Decode_fast(HuffDSCF);
                    L[1] = (idx!=8) ? L[0] + idx : Bitstream_read(6);
                    L[2] = L[1];
                    break;
                case 3:
                    idx  = Huffman_Decode_fast(HuffDSCF);
                    L[0] = (idx!=8) ? L[2] + idx : Bitstream_read(6);
                    L[1] = L[0];
                    L[2] = L[1];
                    break;
                case 2:
                    idx  = Huffman_Decode_fast(HuffDSCF);
                    L[0] = (idx!=8) ? L[2] + idx : Bitstream_read(6);
                    L[1] = L[0];
                    idx  = Huffman_Decode_fast(HuffDSCF);
                    L[2] = (idx!=8) ? L[1] + idx : Bitstream_read(6);
                    break;
                case 0:
                    idx  = Huffman_Decode_fast(HuffDSCF);
                    L[0] = (idx!=8) ? L[2] + idx : Bitstream_read(6);
                    idx  = Huffman_Decode_fast(HuffDSCF);
                    L[1] = (idx!=8) ? L[0] + idx : Bitstream_read(6);
                    idx  = Huffman_Decode_fast(HuffDSCF);
                    L[2] = (idx!=8) ? L[1] + idx : Bitstream_read(6);
                    break;
                default:
                    return;
                    break;
            }
            // update Reference for DSCF
            DSCF_Reference[0][n] = L[2];
        }
        if (*ResR)
        {
            R[2] = DSCF_Reference[1][n];
            switch (SCFI[1][n])
            {
                case 1:
                    idx  = Huffman_Decode_fast(HuffDSCF);
                    R[0] = (idx!=8) ? R[2] + idx : Bitstream_read(6);
                    idx  = Huffman_Decode_fast(HuffDSCF);
                    R[1] = (idx!=8) ? R[0] + idx : Bitstream_read(6);
                    R[2] = R[1];
                    break;
                case 3:
                    idx  = Huffman_Decode_fast(HuffDSCF);
                    R[0] = (idx!=8) ? R[2] + idx : Bitstream_read(6);
                    R[1] = R[0];
                    R[2] = R[1];
                    break;
                case 2:
                    idx  = Huffman_Decode_fast(HuffDSCF);
                    R[0] = (idx!=8) ? R[2] + idx : Bitstream_read(6);
                    R[1] = R[0];
                    idx  = Huffman_Decode_fast(HuffDSCF);
                    R[2] = (idx!=8) ? R[1] + idx : Bitstream_read(6);
                    break;
                case 0:
                    idx  = Huffman_Decode_fast(HuffDSCF);
                    R[0] = (idx!=8) ? R[2] + idx : Bitstream_read(6);
                    idx  = Huffman_Decode_fast(HuffDSCF);
                    R[1] = (idx!=8) ? R[0] + idx : Bitstream_read(6);
                    idx  = Huffman_Decode_fast(HuffDSCF);
                    R[2] = (idx!=8) ? R[1] + idx : Bitstream_read(6);
                    break;
                default:
                    return;
                    break;
            }
            // update Reference for DSCF
            DSCF_Reference[1][n] = R[2];
        }
    }
    /***************************** Samples ****************************/
    ResL = Res[0];
    ResR = Res[1];
    L    = Q[0].L;
    R    = Q[0].R;
    for (n=0; n<=Max_used_Band; ++n, ++ResL, ++ResR, L+=36, R+=36)
    {
        /************** links **************/
        switch (*ResL)
        {
            /*
            case  -2: case  -3: case  -4: case  -5: case  -6: case  -7: case  -8: case  -9:
            case -10: case -11: case -12: case -13: case -14: case -15: case -16: case -17:
                L += 36;
                break;
            */
            case -1:
                for (k=0; k<36; k++ ) {
                    tmp  = random_int ();
                    *L++ = ((tmp >> 24) & 0xFF) + ((tmp >> 16) & 0xFF) + ((tmp >>  8) & 0xFF) + ((tmp >>  0) & 0xFF) - 510;
                }
                break;
            case 0:
                L += 36;// increase pointer
                break;
            case 1:
                Table = HuffQ[Bitstream_read(1)][1];
                for (k=0; k<12; ++k)
                {
                    idx = Huffman_Decode_fast(Table);
                    *L++ = idx30[idx];
                    *L++ = idx31[idx];
                    *L++ = idx32[idx];
                }
                break;
            case 2:
                Table = HuffQ[Bitstream_read(1)][2];
                for (k=0; k<18; ++k)
                {
                    idx = Huffman_Decode_fast(Table);
                    *L++ = idx50[idx];
                    *L++ = idx51[idx];
                }
                break;
            case 3:
            case 4:
                Table = HuffQ[Bitstream_read(1)][*ResL];
                for (k=0; k<36; ++k)
                    *L++ = Huffman_Decode_faster(Table);
                break;
            case 5:
                Table = HuffQ[Bitstream_read(1)][*ResL];
                for (k=0; k<36; ++k)
                    *L++ = Huffman_Decode_fast(Table);
                break;
            case 6:
            case 7:
                Table = HuffQ[Bitstream_read(1)][*ResL];
                for (k=0; k<36; ++k)
                    *L++ = Huffman_Decode(Table);
                break;
            case 8: case 9: case 10: case 11: case 12: case 13: case 14: case 15: case 16: case 17:
                tmp = Dc7[*ResL];
                for (k=0; k<36; ++k)
                    *L++ = (int)Bitstream_read(*ResL-1) - tmp;
                    //*L++ = (int)Bitstream_read(Res_bit[*ResL]) - tmp;
                break;
            default:
                return;
        }
        /************** rechts **************/
        switch (*ResR)
        {
            /*
            case  -2: case  -3: case  -4: case  -5: case  -6: case  -7: case  -8: case  -9:
            case -10: case -11: case -12: case -13: case -14: case -15: case -16: case -17:
                R += 36;
                break;
            */
            case  -2:
                for (k=0; k<36; k++ )
                   *R++ = L [k-36];		// Copy samples from other channel (untested, because there is no Encoder support!!!)
                break;
            case -1:
                for (k=0; k<36; k++ ) {
                    tmp  = random_int ();
                    *R++ = ((tmp >> 24) & 0xFF) + ((tmp >> 16) & 0xFF) + ((tmp >>  8) & 0xFF) + ((tmp >>  0) & 0xFF) - 510;
                }
                break;
            case 0:
                R += 36;// increase pointer
                break;
            case 1:
                Table = HuffQ[Bitstream_read(1)][1];
                for (k=0; k<12; ++k)
                {
                    idx = Huffman_Decode_fast(Table);
                    *R++ = idx30[idx];
                    *R++ = idx31[idx];
                    *R++ = idx32[idx];
                }
                break;
            case 2:
                Table = HuffQ[Bitstream_read(1)][2];
                for (k=0; k<18; ++k)
                {
                    idx = Huffman_Decode_fast(Table);
                    *R++ = idx50[idx];
                    *R++ = idx51[idx];
                }
                break;
            case 3:
            case 4:
                Table = HuffQ[Bitstream_read(1)][*ResR];
                for (k=0; k<36; ++k)
                    *R++ = Huffman_Decode_faster(Table);
                break;
            case 5:
                Table = HuffQ[Bitstream_read(1)][*ResR];
                for (k=0; k<36; ++k)
                    *R++ = Huffman_Decode_fast(Table);
                break;
            case 6:
            case 7:
                Table = HuffQ[Bitstream_read(1)][*ResR];
                for (k=0; k<36; ++k)
                    *R++ = Huffman_Decode(Table);
                break;
            case 8: case 9: case 10: case 11: case 12: case 13: case 14: case 15: case 16: case 17:
                tmp = Dc7[*ResR];
                for (k=0; k<36; ++k)
                    *R++ = (int)Bitstream_read(*ResR-1) - tmp;
                    //*R++ = (int)Bitstream_read(Res_bit[*ResR]) - tmp;
                break;
            default:
                return;
        }
    }
}


MPC_decoder::~MPC_decoder()
{
     if (m_reader) delete m_reader;
	 if (SeekTable) free ( SeekTable );
}

MPC_decoder::MPC_decoder(Reader * r)
{
    m_reader=r;
    eof=0;
    HuffQ[0][0]=0;
    HuffQ[1][0]=0;
    HuffQ[0][1]=HuffQ1[0];
    HuffQ[1][1]=HuffQ1[1];
    HuffQ[0][2]=HuffQ2[0];
    HuffQ[1][2]=HuffQ2[1];
    HuffQ[0][3]=HuffQ3[0];
    HuffQ[1][3]=HuffQ3[1];
    HuffQ[0][4]=HuffQ4[0];
    HuffQ[1][4]=HuffQ4[1];
    HuffQ[0][5]=HuffQ5[0];
    HuffQ[1][5]=HuffQ5[1];
    HuffQ[0][6]=HuffQ6[0];
    HuffQ[1][6]=HuffQ6[1];
    HuffQ[0][7]=HuffQ7[0];
    HuffQ[1][7]=HuffQ7[1];

    SampleHuff[0]=NULL;
    SampleHuff[1]=Entropie_1;
    SampleHuff[2]=Entropie_2;
    SampleHuff[3]=Entropie_3;
    SampleHuff[4]=Entropie_4;
    SampleHuff[5]=Entropie_5;
    SampleHuff[6]=Entropie_6;
    SampleHuff[7]=Entropie_7;
    SampleHuff[8]=NULL;
    SampleHuff[9]=NULL;
    SampleHuff[10]=NULL;
    SampleHuff[11]=NULL;
    SampleHuff[12]=NULL;
    SampleHuff[13]=NULL;
    SampleHuff[14]=NULL;
    SampleHuff[15]=NULL;
    SampleHuff[16]=NULL;
    SampleHuff[17]=NULL;

    MPCHeaderPos=0;
    SeekTable=0;
    sfreq_khz=0;

    StreamVersion=0;
    MS_used=0;
    //FwdJumpInfo=0;
    //ActDecodePos=0;
    RecommendedResyncPos=0;
    FrameWasValid=0;
    OverallFrames=0;
    DecodedFrames=0;
    LastValidSamples=0;
    TrueGaplessPresent=0;

    dword=0;
    pos=0;
    Zaehler=0;
    WordsRead=0;

    //EQ_Activated =   0;
    //EQ_Dezibel   = +20;

    /*
    memset(SAVE_L,0,sizeof(SAVE_L));
    memset(SAVE_R,0,sizeof(SAVE_R));
    memset(FirSave_L,0,sizeof(FirSave_L));
    memset(FirSave_R,0,sizeof(FirSave_R));
    */

    initialisiere_Quantisierungstabellen ();
    Huffman_SV6_Decoder ();
    Huffman_SV7_Decoder ();
}

void MPC_decoder::SetStreamInfo(StreamInfo * si)
{
    StreamVersion      = si->simple.StreamVersion;
    MS_used            = si->simple.MS;
    Max_Band           = si->simple.MaxBand;
    OverallFrames      = si->simple.Frames;
    MPCHeaderPos       = si->simple.HeaderPosition;
    LastValidSamples   = si->simple.LastFrameSamples;
    TrueGaplessPresent = si->simple.IsTrueGapless;
    sfreq_khz          = si->simple.SampleFreq / 1000;

    if (SeekTable != NULL )
        free (SeekTable);
    SeekTable       = (unsigned short *)calloc ( sizeof(unsigned short), OverallFrames+64 );
}

int MPC_decoder::FileInit()
{
    // AB: setting position to the beginning of the data-bitstream
    switch ( StreamVersion ) {
    case 0x04: f_seek ( 4 + MPCHeaderPos, SEEK_SET); pos = 16; break;  // Geht auch über eine der Helperfunktionen
    case 0x05:
    case 0x06: f_seek ( 8 + MPCHeaderPos, SEEK_SET); pos =  0; break;
    case 0x07:
    case 0x17: f_seek ( 24 + MPCHeaderPos, SEEK_SET); pos =  8; break;
    default: return 0;
    }

    // AB: fill buffer and initialize decoder
    f_read ( Speicher, 4*MEMSIZE );
    dword = Speicher [Zaehler = 0];
    return 1;
}

//---------------------------------------------------------------
// will seek from the beginning of the file to the desired
// position in ms (given by seek_needed)
//---------------------------------------------------------------

void
MPC_decoder::Helper1 ( unsigned long bitpos )
{
    f_seek ( (bitpos>>5) * 4 + MPCHeaderPos, SEEK_SET );
    f_read ( Speicher, sizeof(int)*2);
    dword = Speicher [ Zaehler = 0];
    pos   = bitpos & 31;
}

void
MPC_decoder::Helper2 ( unsigned long bitpos )
{
    f_seek ( (bitpos>>5) * 4 + MPCHeaderPos, SEEK_SET );
    f_read ( Speicher, sizeof(int) * MEMSIZE);
    dword = Speicher [ Zaehler = 0];
    pos   = bitpos & 31;
}

void
MPC_decoder::Helper3 ( unsigned long bitpos, long* buffoffs )
{
    pos      = bitpos & 31;
    bitpos >>= 5;
    if ( (unsigned long)(bitpos - *buffoffs) >= MEMSIZE-2 ) {
        *buffoffs = bitpos;
        f_seek(bitpos * 4L + MPCHeaderPos, SEEK_SET );
        f_read(Speicher, sizeof(int)*MEMSIZE);
    }
    dword = Speicher [ Zaehler = bitpos - *buffoffs];
}

int
MPC_decoder::perform_jump ( int* done, int* Frames, int seek_needed )
{
    unsigned long  fpos;
    //unsigned int   FrameBitCnt;
    unsigned int   RING;
    int            fwd;
    unsigned int   decframes;
    unsigned long  buffoffs = 0x80000000;

    switch ( StreamVersion ) {                                                  // setting position to the beginning of the data-bitstream
    case  0x04: fpos =  48; break;
    case  0x05:
    case  0x06: fpos =  64; break;
    case  0x07:
    case  0x17: fpos = 200; break;
    default   : return 0;                                                       // was gibt diese Funktion im Falle eines Fehlers zurück ???
    }

    fwd           = (int) ( seek_needed * sfreq_khz / FRAMELEN + .5f );         // no of frames to skip
    fwd           = fwd < (int)OverallFrames  ?  fwd  :  (int)OverallFrames;    // prevent from desired position out of allowed range
    decframes     = DecodedFrames;
    DecodedFrames = 0;                                                          // reset number of decoded frames

    if ( fwd > 32 && decframes > 384 ) {                                        // only do proceed, if desired position is not in between the first 32 frames
        for ( ; (int)DecodedFrames + 32 < fwd; DecodedFrames++ ) {              // proceed until 32 frames before (!!) desired position (allows to scan the scalefactors)
            if ( SeekTable [DecodedFrames] == 0 ) {
#ifdef OTHER_SEEK
                Helper3 ( fpos, (long*)&buffoffs );
#else
                Helper1 ( fpos );
#endif
                fpos += SeekTable [DecodedFrames] = Read_Bitstream_Jumper(StreamVersion);   // calculate desired pos (in Bits)
            } else {
                fpos += SeekTable [DecodedFrames];
            }
        }
    }
    Helper2 ( fpos );

    for ( ; (int)DecodedFrames < fwd; DecodedFrames++ ) {                            // read the last 32 frames before the desired position to scan the scalefactors (artifactless jumping)
        RING = Zaehler;

	    int tmp = Read_Bitstream (StreamVersion);

        if ( tmp < 0 ) {
//            Box ("Bug in perform_jump");
            return 0;
        }

        /*
        RING         = Zaehler;
        int FwdJumpInfo  = Bitstream_read (20);                                     // read jump-info
        ActDecodePos = (Zaehler << 5) + pos;
        FrameBitCnt  = BitsRead ();                                             // scanning the scalefactors and check for validity of frame
        if (StreamVersion >= 7)  Read_Bitstream_SV7();
        else Read_Bitstream_SV6();
        if ( BitsRead() - FrameBitCnt != FwdJumpInfo ) {
//            Box ("Bug in perform_jump");
            return 0;
        }
        */
        if ( (RING ^ Zaehler) & MEMSIZE2 )                                      // update buffer
            if ( f_read ( Speicher + (RING & MEMSIZE2), 4 * MEMSIZE2) != 4 * MEMSIZE2 ) eof = 1;
    }


    *done = 0;                                                                  // file is not done
    synthFilter.Reset_V ();			                                                         // resetting synthesis filter to avoid "clicks"
    //mod.outMod->Flush ( decode_pos_ms );                                        // flush sound buffer
    *Frames = -1;                                                               // AB: to restart calculation of avg bitrate

    return 1;
}

void MPC_decoder::UpdateBuffer(unsigned int RING)
{
    if ( (RING ^ Zaehler) & MEMSIZE2 )
        if ( f_read(Speicher + (RING & MEMSIZE2), 4 * MEMSIZE2) != 4 * MEMSIZE2 ) eof = 1;      // update buffer
}


// free tagdata from memory
void StreamInfo::Clear()
{
	if (filename) {free(filename);filename=0;}
}

StreamInfo::StreamInfo()
{
	memset(&simple,0,sizeof(simple));
	filename=0;
}

void StreamInfo::SetFilename(const char *fn)
{
	if (filename) free(filename);
	filename=strdup(fn);
}

const char * StreamInfo::GetFilename()
{
	return filename ? filename : "";
}
