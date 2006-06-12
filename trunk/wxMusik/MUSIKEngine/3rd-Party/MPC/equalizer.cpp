#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "equalizer.h"
#include "equal.h"
#include "requant.h"
#include "synth_filter.h"

//#define DEBUG                           // Debugging off or on?
//#define FORMAT  "%10.6f "               // Print out filter coefficients
//#define MAIN                            // have own main() for debug purpose


// Function of the Equalizer off/on switch
//                                      // mode = 0          mode = 1
#ifndef EQ_TYPE                         // ---------------------------------------------------------
# define EQ_TYPE          0             // EQ off            Graphic EQ on
//# define EQ_TYPE        1             // EQ off            Graphic EQ on + Equalization
//# define EQ_TYPE        2             // Graphic EQ on     Graphic EQ on + Equalization
#endif


// Frequencies of the graphic 10-band equalizer
#define EQ_freq_0           32.         // original    60.
#define EQ_freq_1           63.         // original   170.
#define EQ_freq_2          125.         // original   310.
#define EQ_freq_3          250.         // original   600.
#define EQ_freq_4          500.         // original  1000.
#define EQ_freq_5         1000.         // original  3000.
#define EQ_freq_6         2000.         // original  6000.
#define EQ_freq_7         4000.         // original 12000.
#define EQ_freq_8         8000.         // original 14000.
#define EQ_freq_9        16000.         // original 16000.

#define SUBBANDS            32
#define TAPS_PER_SUBBAND    97
#ifndef M_PI
# define M_PI                3.141592653589793238462643383276
#endif


// For a given correction table 'table' and a given frequency
// 'freq' calculate the correction in dB
static float
PickTable ( const float freq, const corr_t* table )
{
    const corr_elem_t*  tab = table -> tab;
    const size_t        len = table -> len;
    size_t              i;

    if ( freq <= tab[0].freq )
        return tab[0].level;

    for ( i = 1; i < len; i++ )
        if ( freq <= tab[i].freq )
            return ( tab[i-1].level * (tab[i  ].freq - freq)
                   + tab[i  ].level * (freq - tab[i-1].freq) ) /
                     (tab[i].freq - tab[i-1].freq);

    return tab [len-1].level;
}


// global variables for Equalizer
static float         EQ_Filter  [SUBBANDS] [36 + 1];    // contains FIR filters (of different length for all subbands. Size of the FIR filters can be between 73 (for subband 0) and 1 (for highest subbands)
static unsigned int  EQ_Activated =   0;                // 0 to bypass equalizer, != 0 to do equalization
int                  EQ_Dezibel   = +20;                // range of the graphic EQ: -EQ_Dezibel...+EQ_Dezibel


// compute FIR filter for a single subband
static void
Do_Equalizer_Setup_Subband ( const float*  resp,        // resp [0...TAPS_PER_SUBBAND-1] frequency response
                             float*        EQ,          // storage to store delay*2+1 filter coefficients
                             int           delay,       // delay*2+1 tap FIR filter with a delay of delay
                             int           odd )        // 0 for even, 1 for odd subbands
{
    double  tmp;
    float   win;
    int     i;
    int     n;

#ifdef FORMAT
    printf ( ":" );
#endif
    if ( delay == 0 ) {                                                 // "1 tap FIR" = global gain for a subband
        tmp = 0.;
        for ( i = 0; i < TAPS_PER_SUBBAND; i++ )
            tmp += resp[i] * resp[i];
        EQ [0] = (float) sqrt ( tmp / TAPS_PER_SUBBAND );
#ifdef FORMAT
        printf ( FORMAT, EQ [0] );
#endif
    }
    else {                                                              // compute 2*delay+1 tap FIR
        for ( n = 0; n <= delay; n++ ) {                                // calculate impulse response of FIR via IDFT
            tmp    = 0.;
            for ( i = 0; i < TAPS_PER_SUBBAND; i++ )
                tmp += resp [odd  ?  TAPS_PER_SUBBAND-1-i  :  i] *      // frequency inversion of every second subband
                       cos  (M_PI/TAPS_PER_SUBBAND * n*(i+0.5) );
            win    = (float) cos ( M_PI/2 / (delay+1) * n );
            EQ [n] = (float) (win * win * tmp / TAPS_PER_SUBBAND);
#ifdef FORMAT
            printf ( FORMAT, EQ [n] );
#endif
        }
    }
#ifdef FORMAT
    printf ( "\n" );
#endif
}


// subbands 0         ... FIR_BANDS1-1  are filtered using a 2*DELAY1+1 tap FIR filter
// subbands FIRBANDS1 ... FIR_BANDS2-1  are filtered using a 2*DELAY2+1 tap FIR filter
// subbands FIRBANDS2 ... FIR_BANDS3-1  are filtered using a 2*DELAY3+1 tap FIR filter
// subbands FIRBANDS3 ... FIR_BANDS4-1  are filtered using a 2*DELAY4+1 tap FIR filter
// subbands FIRBANDS4 ... MaxBand       are filtered using a          1 tap FIR filter (only Gain)

#define FIR_BANDS1         1    // 0.0 ... 0.7 kHz: 73 tap FIR
#define DELAY1            36
#define FIR_BANDS2         3    // 0.7 ... 2.1 kHz: 23 tap FIR
#define DELAY2            11
#define FIR_BANDS3         6    // 2.1 ... 4.1 kHz: 11 tap FIR
#define DELAY3             5
#define FIR_BANDS4        12    // 4.1 ... 8.3 kHz:  5 tap FIR
#define DELAY4             2
                                // 8.3 ...22.0 kHz:  1 tap FIR

static void
Dump ( const char* title, corr_t* T )
{
#ifdef DEBUG
    int  i;

    printf ( "%s\n", title );
    for ( i = 0; i < (int)(T -> len); i++ )
        printf ( "%2u  %7.1f %6.2f\n", i, T -> tab[i].freq, T -> tab[i].level );
    printf ( "\n\n" );
#else
#endif
}


// complete setup of the equalizer for all subbands
// this function should be called by EQSet () with the right parameters
void
Do_Equalizer_Setup ( const float       fs,            // sample frequency in Hz
                     const int         mode,          // see "EQ_TYPE"
                     const float       preamp_ctrl,   // -20. ... +20. (-EQ_Dezibel dB ... +EQ_Dezibel dB)
                     const float*      eq_ctrl,       // -20. ... +20. (-EQ_Dezibel dB ... +EQ_Dezibel dB)
                     const param_eq_t  equalize,
                     const param_eq_t  colorize )
{
    corr_elem_t   K [] = {
        { 0         }, { (float) 0.5* (0         + EQ_freq_0) },
        { EQ_freq_0 }, { (float) 0.5* (EQ_freq_0 + EQ_freq_1) },
        { EQ_freq_1 }, { (float) sqrt (EQ_freq_1 * EQ_freq_2) },
        { EQ_freq_2 }, { (float) sqrt (EQ_freq_2 * EQ_freq_3) },
        { EQ_freq_3 }, { (float) sqrt (EQ_freq_3 * EQ_freq_4) },
        { EQ_freq_4 }, { (float) sqrt (EQ_freq_4 * EQ_freq_5) },
        { EQ_freq_5 }, { (float) sqrt (EQ_freq_5 * EQ_freq_6) },
        { EQ_freq_6 }, { (float) sqrt (EQ_freq_6 * EQ_freq_7) },
        { EQ_freq_7 }, { (float) sqrt (EQ_freq_7 * EQ_freq_8) },
        { EQ_freq_8 }, { (float) sqrt (EQ_freq_8 * EQ_freq_9) },
        { EQ_freq_9 }
    };
    corr_t        T = { K, sizeof(K)/sizeof(*K) };
    float         resp [TAPS_PER_SUBBAND * SUBBANDS]; // frequency response for 1*fx, 3*fx, 5*fx, 7*fx, 9*fx ... 2*TAPS_PER_SUBBAND* SUBBANDS-1*fx
    float         freq;
    float         dB;
    int           i;

#if EQ_TYPE == 0  ||  EQ_TYPE == 1
    if ( (EQ_Activated = mode) == 0 )
        return;
#else
    EQ_Activated = 1;
#endif

    // compute useful attenuations for the 21 frequencies of K/T
    for ( i = 0; i < 10; i++ )
        K[2*i + 2].level = (eq_ctrl [i] + preamp_ctrl) * (EQ_Dezibel * 0.05f);

    K[ 0].level =  K[ 2].level < K[ 4].level  ?  2.f * K[ 2].level - K[ 4].level  :  2.f * K[ 4].level - K[ 2].level;
    K[ 1].level = (K[ 0].level + K[ 2].level) / 2.f;
    K[19].level = (K[18].level + K[20].level) / 2.f;

    for ( i = 3; i <= 17; i += 2 )
        K[i].level = (K[i-1].level + K[i+1].level) * 9/16 - (K[i-3].level + K[i+3].level) * 1/16;

    Dump ("Basic Amplification table", &T );

    // transform from Decibels to Voltage
    for ( i = 0; i < SUBBANDS * TAPS_PER_SUBBAND; i++ ) {       // bin
        freq = (float) ( (i + 0.5) * (fs / 2.) / (SUBBANDS * TAPS_PER_SUBBAND) );
        dB   = PickTable ( freq, &T );
#ifdef DEBUG
        printf ("%7.1f %6.2f\n", freq, dB );
#endif

#if EQ_TYPE == 1  ||  EQ_TYPE == 2
# if EQ_TYPE == 2
        if ( mode == 1 ) {
# endif
            // on request do some high resolution equalizations
            switch ( equalize ) {
            case linear:                                                             break;
            case akg_k401:         dB += PickTable ( freq, &corr_akg_k401         ); break;
            case akg_k501:         dB += PickTable ( freq, &corr_akg_k501         ); break;
            case sennheiser_hd580: dB += PickTable ( freq, &corr_sennheiser_hd580 ); break;
            case sennheiser_hd600: dB += PickTable ( freq, &corr_sennheiser_hd600 ); break;
            case sf_amati:         dB += PickTable ( freq, &corr_sonusfaber_amati ); break;
            }
            switch ( colorize ) {
            case linear:                                                             break;
            case akg_k401:         dB -= PickTable ( freq, &corr_akg_k401         ); break;
            case akg_k501:         dB -= PickTable ( freq, &corr_akg_k501         ); break;
            case sennheiser_hd580: dB -= PickTable ( freq, &corr_sennheiser_hd580 ); break;
            case sennheiser_hd600: dB -= PickTable ( freq, &corr_sennheiser_hd600 ); break;
            case sf_amati:         dB -= PickTable ( freq, &corr_sonusfaber_amati ); break;
            }
# if EQ_TYPE == 2
        }
# endif
#endif
        resp [i] = (float) pow ( 10.f, 0.05f * dB );
    }

    // calculate FIR filter/Global gains for the subbands
#ifdef DEBUG
    printf ("\nFIR Filter coefficients for the 32 subbands\n" );
#endif
    i = 0;
    for ( ; i < FIR_BANDS1; i++ )
        Do_Equalizer_Setup_Subband ( resp + i * TAPS_PER_SUBBAND, EQ_Filter[i], DELAY1, i & 1 );
    for ( ; i < FIR_BANDS2; i++ )
        Do_Equalizer_Setup_Subband ( resp + i * TAPS_PER_SUBBAND, EQ_Filter[i], DELAY2, i & 1 );
    for ( ; i < FIR_BANDS3; i++ )
        Do_Equalizer_Setup_Subband ( resp + i * TAPS_PER_SUBBAND, EQ_Filter[i], DELAY3, i & 1 );
    for ( ; i < FIR_BANDS4; i++ )
        Do_Equalizer_Setup_Subband ( resp + i * TAPS_PER_SUBBAND, EQ_Filter[i], DELAY4, i & 1 );
    for ( ; i < SUBBANDS  ; i++ )
        Do_Equalizer_Setup_Subband ( resp + i * TAPS_PER_SUBBAND, EQ_Filter[i], 0     , i & 1 );
}


#if defined __GNUC__  || defined __cplusplus  ||  ( __STDC_VERSION__  &&  __STDC_VERSION__ >= 199901L )
inline
#endif
static void
Subband_Equalizer ( float         Y    [  36] [32],  // current subband samples
                    const float   SAVE [3*36] [32],  // storage for the last 2 frames + current frame
                    const float*  EQ,
                    unsigned int  delay )
{
    float  tmp;
    int    k;
    int    n;

    if ( delay == 0 ) {
        tmp = EQ [0];
        for ( k = 0; k < 36; k++ )
            Y [k] [0] = tmp * SAVE [k] [0];
    }
    else {
        for ( k = 0; k < 36; k++ ) {
            tmp = EQ [0] * SAVE [k] [0];
            for ( n = 1; n <= (int)delay; n++ )
                tmp += EQ [n] * (SAVE [k-n] [0] + SAVE [k+n] [0]);
            Y [k] [0] = tmp;
        }
    }
}


// channel subband equalizer, called by Do_Perform_Equalizer() for each channel
static void
Channel_Equalizer ( float  Y    [  36] [32],    // current subband samples
                    float  SAVE [3*36] [32],    // storage for the last 2 frames + current frame
                    int    MaxBand )
{
    int  band;

    memmove ( SAVE [   0], SAVE [36], 2 * 36 * sizeof(SAVE[0]) );
    memcpy  ( SAVE [2*36], Y        , 1 * 36 * sizeof(SAVE[0]) );

    band = 0;
    for ( ; band < FIR_BANDS1; band++ )
        Subband_Equalizer ( (float (*)[32]) & Y [0] [band], (const float (*)[32]) & SAVE [36] [band], EQ_Filter [band], DELAY1 );
    for ( ; band < FIR_BANDS2; band++ )
        Subband_Equalizer ( (float (*)[32]) & Y [0] [band], (const float (*)[32]) & SAVE [36] [band], EQ_Filter [band], DELAY2 );
    for ( ; band < FIR_BANDS3; band++ )
        Subband_Equalizer ( (float (*)[32]) & Y [0] [band], (const float (*)[32]) & SAVE [36] [band], EQ_Filter [band], DELAY3 );
    for ( ; band < FIR_BANDS4; band++ )
        Subband_Equalizer ( (float (*)[32]) & Y [0] [band], (const float (*)[32]) & SAVE [36] [band], EQ_Filter [band], DELAY4 );
    for ( ; band <= MaxBand  ; band++ )
        Subband_Equalizer ( (float (*)[32]) & Y [0] [band], (const float (*)[32]) & SAVE [36] [band], EQ_Filter [band], 0      );
}


// subband equalizer, should be called by the Musepack decoder with the right parameters
void
Do_Perform_Equalizer ( float         Y    [] [  36] [32],   // current subband samples
                       float         SAVE [] [3*36] [32],   // buffer for the last 2 frames and the current frame subband samples
                       int           MaxBand,               // last subband in use (0...31)
                       unsigned int  channels )             // number of channels
{
    unsigned int  ch;

    if ( ! EQ_Activated )
        return;

    for ( ch = 0; ch < channels; ch++ )
        Channel_Equalizer ( Y [ch], SAVE [ch], MaxBand );
}


// Is EQ activated?  (needed by DECODE() to compensate additional delay of equalizer)
unsigned int
is_EQ_Activated ( void )
{
    return EQ_Activated  ?  1  :  0;
}


#ifdef MAIN

int
main ( int argc, char** argv )
{
    //static float  eq [10] = { 0.,0.,0.,0.,0.,0.,0.,0.,0.,0. };
    static float  eq [10] = { 20.,4.,0.,-1.,-2.,-1.,0.,0.,0.,8. };

    Do_Equalizer_Setup ( 44100., 0, 0., eq, linear, linear );
    return 0;
}

#endif

/* end of equalizer.c/cpp */
