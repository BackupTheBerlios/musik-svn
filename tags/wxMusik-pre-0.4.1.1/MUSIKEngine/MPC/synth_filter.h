#ifndef _synth_filter_h_
#define _synth_filter_h_

/* D E F I N E S */
#define ftol(A,B)                             \
     { tmp = *(int*) & A - 0x4B7F8000;        \
       if ( tmp != (short)tmp )               \
           tmp = (tmp>>31) ^ 0x7FFF, clips++; \
       B   = (short) tmp;                     \
     }

class SynthFilter
{
public:
	SynthFilter()
	{
		clips = 0;
		 Reset_V ();
		 Reset_Y ();
	}
	void            Synthese_Filter_opt      ( short* dst );
	void            Synthese_Filter_dithered ( short* dst );
	void            Reset_Y( void );
	void            Reset_V( void );

	float  YY [2] [36] [32];
	unsigned long  clips;
protected:
	void Calculate_New_V ( const float* Sample, float* V );
	void Vectoring ( short* Data, const float* V );
	void Vectoring_dithered ( short* Data, const float* V, const float* dither );
private:
	/* D E F I N E S */
#define V_MEM   2304

	/* V A R I A B L E S */
	float  VV [2] [V_MEM + 960];
	float  A00, A01, A02, A03, A04, A05, A06, A07, A08, A09, A10, A11, A12, A13, A14, A15;
	float  B00, B01, B02, B03, B04, B05, B06, B07, B08, B09, B10, B11, B12, B13, B14, B15;

};

/* F U N C T I O N S */
unsigned int    random_int               ( void );

#endif
