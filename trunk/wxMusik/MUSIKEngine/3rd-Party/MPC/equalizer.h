#ifndef XMMS_EQUALIZER_H
#define XMMS_EQUALIZER_H

typedef enum {
    linear   = 0,
    akg_k401,
    akg_k501,
    sennheiser_hd580,
    sennheiser_hd600,
    sf_amati
} param_eq_t;


void
Do_Equalizer_Setup   ( const float       fs,
                       const int         mode,
                       const float       preamp_ctrl,
                       const float*      eq_ctrl,
                       const param_eq_t  equalize,
                       const param_eq_t  colorize );

void
Do_Perform_Equalizer ( float         Y    [] [  36] [32],   // current subband samples
                       float         SAVE [] [3*36] [32],   // buffer for the last 2 frames and the current frame subband samples
                       int           MaxBand,               // last subband in use (0...31)
                       unsigned int  channels );            // number of channels

unsigned int
is_EQ_Activated      ( void );

#endif /* XMMS_EQUALIZER_H */

/* end of equalizer.h */
