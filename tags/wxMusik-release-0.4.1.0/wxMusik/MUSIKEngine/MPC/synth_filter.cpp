#include <string.h>
#include <stdlib.h>
#include "synth_filter.h"



/* C O N S T A N T S */
#undef _
#define _(value)  (float)(value##.##L / 0x10000)

const float  Di_opt [32] [16] = {
   { _(  0), _( -29), _( 213), _( -459), _( 2037), _(-5153), _(  6574), _(-37489), _(75038), _(37489), _(6574), _( 5153), _(2037), _( 459), _(213), _(29) },
   { _( -1), _( -31), _( 218), _( -519), _( 2000), _(-5517), _(  5959), _(-39336), _(74992), _(35640), _(7134), _( 4788), _(2063), _( 401), _(208), _(26) },
   { _( -1), _( -35), _( 222), _( -581), _( 1952), _(-5879), _(  5288), _(-41176), _(74856), _(33791), _(7640), _( 4425), _(2080), _( 347), _(202), _(24) },
   { _( -1), _( -38), _( 225), _( -645), _( 1893), _(-6237), _(  4561), _(-43006), _(74630), _(31947), _(8092), _( 4063), _(2087), _( 294), _(196), _(21) },
   { _( -1), _( -41), _( 227), _( -711), _( 1822), _(-6589), _(  3776), _(-44821), _(74313), _(30112), _(8492), _( 3705), _(2085), _( 244), _(190), _(19) },
   { _( -1), _( -45), _( 228), _( -779), _( 1739), _(-6935), _(  2935), _(-46617), _(73908), _(28289), _(8840), _( 3351), _(2075), _( 197), _(183), _(17) },
   { _( -1), _( -49), _( 228), _( -848), _( 1644), _(-7271), _(  2037), _(-48390), _(73415), _(26482), _(9139), _( 3004), _(2057), _( 153), _(176), _(16) },
   { _( -2), _( -53), _( 227), _( -919), _( 1535), _(-7597), _(  1082), _(-50137), _(72835), _(24694), _(9389), _( 2663), _(2032), _( 111), _(169), _(14) },
   { _( -2), _( -58), _( 224), _( -991), _( 1414), _(-7910), _(    70), _(-51853), _(72169), _(22929), _(9592), _( 2330), _(2001), _(  72), _(161), _(13) },
   { _( -2), _( -63), _( 221), _(-1064), _( 1280), _(-8209), _(  -998), _(-53534), _(71420), _(21189), _(9750), _( 2006), _(1962), _(  36), _(154), _(11) },
   { _( -2), _( -68), _( 215), _(-1137), _( 1131), _(-8491), _( -2122), _(-55178), _(70590), _(19478), _(9863), _( 1692), _(1919), _(   2), _(147), _(10) },
   { _( -3), _( -73), _( 208), _(-1210), _(  970), _(-8755), _( -3300), _(-56778), _(69679), _(17799), _(9935), _( 1388), _(1870), _( -29), _(139), _( 9) },
   { _( -3), _( -79), _( 200), _(-1283), _(  794), _(-8998), _( -4533), _(-58333), _(68692), _(16155), _(9966), _( 1095), _(1817), _( -57), _(132), _( 8) },
   { _( -4), _( -85), _( 189), _(-1356), _(  605), _(-9219), _( -5818), _(-59838), _(67629), _(14548), _(9959), _(  814), _(1759), _( -83), _(125), _( 7) },
   { _( -4), _( -91), _( 177), _(-1428), _(  402), _(-9416), _( -7154), _(-61289), _(66494), _(12980), _(9916), _(  545), _(1698), _(-106), _(117), _( 7) },
   { _( -5), _( -97), _( 163), _(-1498), _(  185), _(-9585), _( -8540), _(-62684), _(65290), _(11455), _(9838), _(  288), _(1634), _(-127), _(111), _( 6) },
   { _( -5), _(-104), _( 146), _(-1567), _(  -45), _(-9727), _( -9975), _(-64019), _(64019), _( 9975), _(9727), _(   45), _(1567), _(-146), _(104), _( 5) },
   { _( -6), _(-111), _( 127), _(-1634), _( -288), _(-9838), _(-11455), _(-65290), _(62684), _( 8540), _(9585), _( -185), _(1498), _(-163), _( 97), _( 5) },
   { _( -7), _(-117), _( 106), _(-1698), _( -545), _(-9916), _(-12980), _(-66494), _(61289), _( 7154), _(9416), _( -402), _(1428), _(-177), _( 91), _( 4) },
   { _( -7), _(-125), _(  83), _(-1759), _( -814), _(-9959), _(-14548), _(-67629), _(59838), _( 5818), _(9219), _( -605), _(1356), _(-189), _( 85), _( 4) },
   { _( -8), _(-132), _(  57), _(-1817), _(-1095), _(-9966), _(-16155), _(-68692), _(58333), _( 4533), _(8998), _( -794), _(1283), _(-200), _( 79), _( 3) },
   { _( -9), _(-139), _(  29), _(-1870), _(-1388), _(-9935), _(-17799), _(-69679), _(56778), _( 3300), _(8755), _( -970), _(1210), _(-208), _( 73), _( 3) },
   { _(-10), _(-147), _(  -2), _(-1919), _(-1692), _(-9863), _(-19478), _(-70590), _(55178), _( 2122), _(8491), _(-1131), _(1137), _(-215), _( 68), _( 2) },
   { _(-11), _(-154), _( -36), _(-1962), _(-2006), _(-9750), _(-21189), _(-71420), _(53534), _(  998), _(8209), _(-1280), _(1064), _(-221), _( 63), _( 2) },
   { _(-13), _(-161), _( -72), _(-2001), _(-2330), _(-9592), _(-22929), _(-72169), _(51853), _(  -70), _(7910), _(-1414), _( 991), _(-224), _( 58), _( 2) },
   { _(-14), _(-169), _(-111), _(-2032), _(-2663), _(-9389), _(-24694), _(-72835), _(50137), _(-1082), _(7597), _(-1535), _( 919), _(-227), _( 53), _( 2) },
   { _(-16), _(-176), _(-153), _(-2057), _(-3004), _(-9139), _(-26482), _(-73415), _(48390), _(-2037), _(7271), _(-1644), _( 848), _(-228), _( 49), _( 1) },
   { _(-17), _(-183), _(-197), _(-2075), _(-3351), _(-8840), _(-28289), _(-73908), _(46617), _(-2935), _(6935), _(-1739), _( 779), _(-228), _( 45), _( 1) },
   { _(-19), _(-190), _(-244), _(-2085), _(-3705), _(-8492), _(-30112), _(-74313), _(44821), _(-3776), _(6589), _(-1822), _( 711), _(-227), _( 41), _( 1) },
   { _(-21), _(-196), _(-294), _(-2087), _(-4063), _(-8092), _(-31947), _(-74630), _(43006), _(-4561), _(6237), _(-1893), _( 645), _(-225), _( 38), _( 1) },
   { _(-24), _(-202), _(-347), _(-2080), _(-4425), _(-7640), _(-33791), _(-74856), _(41176), _(-5288), _(5879), _(-1952), _( 581), _(-222), _( 35), _( 1) },
   { _(-26), _(-208), _(-401), _(-2063), _(-4788), _(-7134), _(-35640), _(-74992), _(39336), _(-5959), _(5517), _(-2000), _( 519), _(-218), _( 31), _( 1) }
};

#undef  _




/* F U N K T I O N E N */
void
SynthFilter::Reset_V ( void )
{
    memset ( VV, 0, sizeof VV );
}
void
SynthFilter::Reset_Y ( void )
{
	memset ( YY, 0, sizeof YY );
}

void
SynthFilter::Calculate_New_V ( const float* Sample, float* V )
{
    // Calculating new V-buffer values for left channel
    // calculate new V-values (ISO-11172-3, p. 39)
    // based upon fast-MDCT algorithm by Byeong Gi Lee
    float         tmp;

    A00 = Sample[ 0] + Sample[31];
    A01 = Sample[ 1] + Sample[30];
    A02 = Sample[ 2] + Sample[29];
    A03 = Sample[ 3] + Sample[28];
    A04 = Sample[ 4] + Sample[27];
    A05 = Sample[ 5] + Sample[26];
    A06 = Sample[ 6] + Sample[25];
    A07 = Sample[ 7] + Sample[24];
    A08 = Sample[ 8] + Sample[23];
    A09 = Sample[ 9] + Sample[22];
    A10 = Sample[10] + Sample[21];
    A11 = Sample[11] + Sample[20];
    A12 = Sample[12] + Sample[19];
    A13 = Sample[13] + Sample[18];
    A14 = Sample[14] + Sample[17];
    A15 = Sample[15] + Sample[16];

    B00 =  A00 + A15;
    B01 =  A01 + A14;
    B02 =  A02 + A13;
    B03 =  A03 + A12;
    B04 =  A04 + A11;
    B05 =  A05 + A10;
    B06 =  A06 + A09;
    B07 =  A07 + A08;
    B08 = (A00 - A15) * 0.5024192929f;
    B09 = (A01 - A14) * 0.5224986076f;
    B10 = (A02 - A13) * 0.5669440627f;
    B11 = (A03 - A12) * 0.6468217969f;
    B12 = (A04 - A11) * 0.7881546021f;
    B13 = (A05 - A10) * 1.0606776476f;
    B14 = (A06 - A09) * 1.7224471569f;
    B15 = (A07 - A08) * 5.1011486053f;

    A00 =  B00 + B07;
    A01 =  B01 + B06;
    A02 =  B02 + B05;
    A03 =  B03 + B04;
    A04 = (B00 - B07) * 0.5097956061f;
    A05 = (B01 - B06) * 0.6013448834f;
    A06 = (B02 - B05) * 0.8999761939f;
    A07 = (B03 - B04) * 2.5629155636f;
    A08 =  B08 + B15;
    A09 =  B09 + B14;
    A10 =  B10 + B13;
    A11 =  B11 + B12;
    A12 = (B08 - B15) * 0.5097956061f;
    A13 = (B09 - B14) * 0.6013448834f;
    A14 = (B10 - B13) * 0.8999761939f;
    A15 = (B11 - B12) * 2.5629155636f;

    B00 =  A00 + A03;
    B01 =  A01 + A02;
    B02 = (A00 - A03) * 0.5411961079f;
    B03 = (A01 - A02) * 1.3065630198f;
    B04 =  A04 + A07;
    B05 =  A05 + A06;
    B06 = (A04 - A07) * 0.5411961079f;
    B07 = (A05 - A06) * 1.3065630198f;
    B08 =  A08 + A11;
    B09 =  A09 + A10;
    B10 = (A08 - A11) * 0.5411961079f;
    B11 = (A09 - A10) * 1.3065630198f;
    B12 =  A12 + A15;
    B13 =  A13 + A14;
    B14 = (A12 - A15) * 0.5411961079f;
    B15 = (A13 - A14) * 1.3065630198f;

    A00 =  B00 + B01;
    A01 = (B00 - B01) * 0.7071067691f;
    A02 =  B02 + B03;
    A03 = (B02 - B03) * 0.7071067691f;
    A04 =  B04 + B05;
    A05 = (B04 - B05) * 0.7071067691f;
    A06 =  B06 + B07;
    A07 = (B06 - B07) * 0.7071067691f;
    A08 =  B08 + B09;
    A09 = (B08 - B09) * 0.7071067691f;
    A10 =  B10 + B11;
    A11 = (B10 - B11) * 0.7071067691f;
    A12 =  B12 + B13;
    A13 = (B12 - B13) * 0.7071067691f;
    A14 =  B14 + B15;
    A15 = (B14 - B15) * 0.7071067691f;

    V[48] = -A00;
    V[ 0] =  A01;
    V[40] = -A02 - (V[ 8] = A03);
    V[36] = -((V[ 4] = A05 + (V[12] = A07)) + A06);
    V[44] = - A04 - A06 - A07;
    V[ 6] = (V[10] = A11 + (V[14] = A15)) + A13;
    V[38] = (V[34] = -(V[ 2] = A09 + A13 + A15) - A14) + A09 - A10 - A11;
    V[46] = (tmp = -(A12 + A14 + A15)) - A08;
    V[42] = tmp - A10 - A11;

    A00 = (Sample[ 0] - Sample[31]) * 0.5006030202f;
    A01 = (Sample[ 1] - Sample[30]) * 0.5054709315f;
    A02 = (Sample[ 2] - Sample[29]) * 0.5154473186f;
    A03 = (Sample[ 3] - Sample[28]) * 0.5310425758f;
    A04 = (Sample[ 4] - Sample[27]) * 0.5531039238f;
    A05 = (Sample[ 5] - Sample[26]) * 0.5829349756f;
    A06 = (Sample[ 6] - Sample[25]) * 0.6225041151f;
    A07 = (Sample[ 7] - Sample[24]) * 0.6748083234f;
    A08 = (Sample[ 8] - Sample[23]) * 0.7445362806f;
    A09 = (Sample[ 9] - Sample[22]) * 0.8393496275f;
    A10 = (Sample[10] - Sample[21]) * 0.9725682139f;
    A11 = (Sample[11] - Sample[20]) * 1.1694399118f;
    A12 = (Sample[12] - Sample[19]) * 1.4841645956f;
    A13 = (Sample[13] - Sample[18]) * 2.0577809811f;
    A14 = (Sample[14] - Sample[17]) * 3.4076085091f;
    A15 = (Sample[15] - Sample[16]) *10.1900081635f;

    B00 =  A00 + A15;
    B01 =  A01 + A14;
    B02 =  A02 + A13;
    B03 =  A03 + A12;
    B04 =  A04 + A11;
    B05 =  A05 + A10;
    B06 =  A06 + A09;
    B07 =  A07 + A08;
    B08 = (A00 - A15) * 0.5024192929f;
    B09 = (A01 - A14) * 0.5224986076f;
    B10 = (A02 - A13) * 0.5669440627f;
    B11 = (A03 - A12) * 0.6468217969f;
    B12 = (A04 - A11) * 0.7881546021f;
    B13 = (A05 - A10) * 1.0606776476f;
    B14 = (A06 - A09) * 1.7224471569f;
    B15 = (A07 - A08) * 5.1011486053f;

    A00 =  B00 + B07;
    A01 =  B01 + B06;
    A02 =  B02 + B05;
    A03 =  B03 + B04;
    A04 = (B00 - B07) * 0.5097956061f;
    A05 = (B01 - B06) * 0.6013448834f;
    A06 = (B02 - B05) * 0.8999761939f;
    A07 = (B03 - B04) * 2.5629155636f;
    A08 =  B08 + B15;
    A09 =  B09 + B14;
    A10 =  B10 + B13;
    A11 =  B11 + B12;
    A12 = (B08 - B15) * 0.5097956061f;
    A13 = (B09 - B14) * 0.6013448834f;
    A14 = (B10 - B13) * 0.8999761939f;
    A15 = (B11 - B12) * 2.5629155636f;

    B00 =  A00 + A03;
    B01 =  A01 + A02;
    B02 = (A00 - A03) * 0.5411961079f;
    B03 = (A01 - A02) * 1.3065630198f;
    B04 =  A04 + A07;
    B05 =  A05 + A06;
    B06 = (A04 - A07) * 0.5411961079f;
    B07 = (A05 - A06) * 1.3065630198f;
    B08 =  A08 + A11;
    B09 =  A09 + A10;
    B10 = (A08 - A11) * 0.5411961079f;
    B11 = (A09 - A10) * 1.3065630198f;
    B12 =  A12 + A15;
    B13 =  A13 + A14;
    B14 = (A12 - A15) * 0.5411961079f;
    B15 = (A13 - A14) * 1.3065630198f;

    A00 =  B00 + B01;
    A01 = (B00 - B01) * 0.7071067691f;
    A02 =  B02 + B03;
    A03 = (B02 - B03) * 0.7071067691f;
    A04 =  B04 + B05;
    A05 = (B04 - B05) * 0.7071067691f;
    A06 =  B06 + B07;
    A07 = (B06 - B07) * 0.7071067691f;
    A08 =  B08 + B09;
    A09 = (B08 - B09) * 0.7071067691f;
    A10 =  B10 + B11;
    A11 = (B10 - B11) * 0.7071067691f;
    A12 =  B12 + B13;
    A13 = (B12 - B13) * 0.7071067691f;
    A14 =  B14 + B15;
    A15 = (B14 - B15) * 0.7071067691f;

    // mehrfach verwendete Ausdr�cke: A04+A06+A07, A09+A13+A15
    V[ 5] = (V[11] = (V[13] = A07 + (V[15] = A15)) + A11) + A05 + A13;
    V[ 7] = (V[ 9] = A03 + A11 + A15) + A13;
    V[33] = -(V[ 1] = A01 + A09 + A13 + A15) - A14;
    V[35] = -(V[ 3] = A05 + A07 + A09 + A13 + A15) - A06 - A14;
    V[37] = (tmp = -(A10 + A11 + A13 + A14 + A15)) - A05 - A06 - A07;
    V[39] = tmp - A02 - A03;                      // abh�ngig vom Befehl dr�ber
    V[41] = (tmp += A13 - A12) - A02 - A03;       // abh�ngig vom Befehl 2 dr�ber
    V[43] = tmp - A04 - A06 - A07;                // abh�ngig von Befehlen 1 und 3 dr�ber
    V[47] = (tmp = -(A08 + A12 + A14 + A15)) - A00;
    V[45] = tmp - A04 - A06 - A07;                // abh�ngig vom Befehl dr�ber

    V[32] = -V[ 0];
    V[31] = -V[ 1];
    V[30] = -V[ 2];
    V[29] = -V[ 3];
    V[28] = -V[ 4];
    V[27] = -V[ 5];
    V[26] = -V[ 6];
    V[25] = -V[ 7];
    V[24] = -V[ 8];
    V[23] = -V[ 9];
    V[22] = -V[10];
    V[21] = -V[11];
    V[20] = -V[12];
    V[19] = -V[13];
    V[18] = -V[14];
    V[17] = -V[15];

    V[63] =  V[33];
    V[62] =  V[34];
    V[61] =  V[35];
    V[60] =  V[36];
    V[59] =  V[37];
    V[58] =  V[38];
    V[57] =  V[39];
    V[56] =  V[40];
    V[55] =  V[41];
    V[54] =  V[42];
    V[53] =  V[43];
    V[52] =  V[44];
    V[51] =  V[45];
    V[50] =  V[46];
    V[49] =  V[47];
}

void
SynthFilter::Vectoring ( short* Data, const float* V )
{
    const float*  D = (float*) Di_opt;
    float         Sum;
    int           k;
    int           tmp;

    for ( k = 0; k < 32; k++, D += 16, V++ ) {
        Sum = V[  0]*D[ 0] + V[ 96]*D[ 1] + V[128]*D[ 2] + V[224]*D[ 3]
            + V[256]*D[ 4] + V[352]*D[ 5] + V[384]*D[ 6] + V[480]*D[ 7]
            + V[512]*D[ 8] + V[608]*D[ 9] + V[640]*D[10] + V[736]*D[11]
            + V[768]*D[12] + V[864]*D[13] + V[896]*D[14] + V[992]*D[15] + 0xFF8000;
        ftol ( Sum, *Data );    // copy to PCM (ftol mit S�ttigung, +0xFF8000 geh�rt bereits dazu)
        Data += 2;
    }
}

void
SynthFilter::Synthese_Filter_opt ( short* OutData )
{
    int           n;
    float*        V;
    const float*  Y;

    /********* left channel ********/
    memmove ( VV[0] + V_MEM, VV[0], 960 * sizeof(float) );
    V = VV[0] + V_MEM;
    Y = YY[0] [0];

    for ( n = 0; n < 36; n++, OutData += 64, Y += 32 ) {
        V -= 64;
        Calculate_New_V ( Y, V );
        Vectoring       ( OutData, V );
    }
    // set pointer to OutData[1] for filling right channel
    OutData -= 2*1152 - 1;

    /******** right channel ********/
    memmove ( VV[1] + V_MEM, VV[1], 960 * sizeof(float) );
    V = VV[1] + V_MEM;
    Y = YY[1] [0];

    for ( n = 0; n < 36; n++, OutData += 64, Y += 32 ) {
        V -= 64;
        Calculate_New_V ( Y, V );
        Vectoring       ( OutData, V );
    }
}

/*******************************************/
/*                                         */
/*            dithered synthesis           */
/*                                         */
/*******************************************/


static const unsigned char    Parity [256] = {  // parity
    0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,1,0,0,1,0,1,1,0,0,1,1,0,1,0,0,1,
    1,0,0,1,0,1,1,0,0,1,1,0,1,0,0,1,0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,
    1,0,0,1,0,1,1,0,0,1,1,0,1,0,0,1,0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,
    0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,1,0,0,1,0,1,1,0,0,1,1,0,1,0,0,1,
    1,0,0,1,0,1,1,0,0,1,1,0,1,0,0,1,0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,
    0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,1,0,0,1,0,1,1,0,0,1,1,0,1,0,0,1,
    0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,1,0,0,1,0,1,1,0,0,1,1,0,1,0,0,1,
    1,0,0,1,0,1,1,0,0,1,1,0,1,0,0,1,0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0
};

static unsigned int  __r1 = 1;
static unsigned int  __r2 = 1;

/*
 *  This is a simple random number generator with good quality for audio purposes.
 *  It consists of two polycounters with opposite rotation direction and different
 *  periods. The periods are coprime, so the total period is the product of both.
 *
 *     -------------------------------------------------------------------------------------------------
 * +-> |31:30:29:28:27:26:25:24:23:22:21:20:19:18:17:16:15:14:13:12:11:10: 9: 8: 7: 6: 5: 4: 3: 2: 1: 0|
 * |   -------------------------------------------------------------------------------------------------
 * |                                                                          |  |  |  |     |        |
 * |                                                                          +--+--+--+-XOR-+--------+
 * |                                                                                      |
 * +--------------------------------------------------------------------------------------+
 *
 *     -------------------------------------------------------------------------------------------------
 *     |31:30:29:28:27:26:25:24:23:22:21:20:19:18:17:16:15:14:13:12:11:10: 9: 8: 7: 6: 5: 4: 3: 2: 1: 0| <-+
 *     -------------------------------------------------------------------------------------------------   |
 *       |  |           |  |                                                                               |
 *       +--+----XOR----+--+                                                                               |
 *                |                                                                                        |
 *                +----------------------------------------------------------------------------------------+
 *
 *
 *  The first has an period of 3*5*17*257*65537, the second of 7*47*73*178481,
 *  which gives a period of 18.410.713.077.675.721.215. The result is the
 *  XORed values of both generators.
 */

unsigned int
random_int ( void )
{
#if 1
    unsigned int  t1, t2, t3, t4;

    t3   = t1 = __r1;   t4   = t2 = __r2;       // Parity calculation is done via table lookup, this is also available
    t1  &= 0xF5;        t2 >>= 25;              // on CPUs without parity, can be implemented in C and avoid unpredictable
    t1   = Parity [t1]; t2  &= 0x63;            // jumps and slow rotate through the carry flag operations.
    t1 <<= 31;          t2   = Parity [t2];

    return (__r1 = (t3 >> 1) | t1 ) ^ (__r2 = (t4 + t4) | t2 );
#else
    return (__r1 = (__r1 >> 1) | ((unsigned int)Parity [__r1 & 0xF5] << 31) ) ^
           (__r2 = (__r2 << 1) |  (unsigned int)Parity [(__r2 >> 25) & 0x63] );
#endif
}

// generates triangular dither
static void
GenerateDither_old ( float* p, size_t len )
{
    while (len--)
        *p++ = ( (float)(int)random_int() + (float)(int)random_int() ) * (1.f/65536.f/65536.f);
}

static void
GenerateDither ( float* p, size_t len )
{
    static int  last = 0;
    int         tmp;

    while (len--) {
        tmp  = (int) random_int();
        *p++ = ((float)tmp - last) * (1.f/65536.f/65536.f);
        last = tmp;
    }
}

// dithered vectoring
void
SynthFilter::Vectoring_dithered ( short* Data, const float* V, const float* dither )
{
    const float*  D = (float*) Di_opt;
    float         Sum;
    int           k;
    int           tmp;

    for ( k = 0; k < 32; k++, D += 16, V++ ) {
        Sum = V[  0]*D[ 0] + V[ 96]*D[ 1] + V[128]*D[ 2] + V[224]*D[ 3]
            + V[256]*D[ 4] + V[352]*D[ 5] + V[384]*D[ 6] + V[480]*D[ 7]
            + V[512]*D[ 8] + V[608]*D[ 9] + V[640]*D[10] + V[736]*D[11]
            + V[768]*D[12] + V[864]*D[13] + V[896]*D[14] + V[992]*D[15] + *dither++ + 0xFF8000 ;
        ftol ( Sum, *Data );    // copy to PCM (ftol mit S�ttigung, +0xFF8000 geh�rt bereits dazu)
        Data += 2;
    }
}

// dithered synthesis
void
SynthFilter::Synthese_Filter_dithered ( short* OutData )
{
    int           n;
    float*        V;
    float         DitherData [36 * 32];
    const float*  Y;

    GenerateDither ( DitherData, 32*36 );
    /********* left channel ********/
    memmove ( VV[0] + V_MEM, VV[0], 960 * sizeof(float) );
    V = VV[0] + V_MEM;
    Y = YY[0] [0];

    for ( n = 0; n < 36; n++, OutData += 64, Y += 32 ) {
        V -= 64;
        Calculate_New_V    ( Y, V );
        Vectoring_dithered ( OutData, V, DitherData + 32*n );
    }
    // set pointer to OutData[1] for filling right channel
    OutData    -= 2*1152 - 1;

    /******** right channel ********/
    memmove ( VV[1] + V_MEM, VV[1], 960 * sizeof(float) );
    V = VV[1] + V_MEM;
    Y = YY[1] [0];

    for ( n = 0; n < 36; n++, OutData += 64, Y += 32 ) {
        V -= 64;
        Calculate_New_V    ( Y, V );
        Vectoring_dithered ( OutData, V, DitherData + 32*n );
    }
}

/* end of synth_filter.cpp */
