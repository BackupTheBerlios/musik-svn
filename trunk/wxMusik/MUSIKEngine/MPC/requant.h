#ifndef _requant_h
#define _requant_h_

#include "mpc_dec.h"

/* V A R I A B L E S */
#if 0
extern float              SCF  [256];       // holds adapted scalefactors (for clipping prevention)
extern unsigned int       Q_bit [32];       // holds amount of bits to save chosen quantizer (SV6)
extern unsigned int       Q_res [32] [16];  // holds conversion: index -> quantizer (SV6)
extern int                Max_Band;
#endif

/* C O N S T A N T S */
extern const unsigned int Res_bit [18];     // bits per sample for chosen quantizer
extern const float        __Cc7   [1 + 18];     // coefficients for requantization
extern const int          __Dc7   [1 + 18];     // offset for requantization

#define Cc7     (__Cc7 + 1)
#define Dc7     (__Dc7 + 1)

/* F U N C T I O N S */
#if 0
void initialisiere_Quantisierungstabellen ( void );
void ScaleOutput                          ( double factor );
#endif
#endif
