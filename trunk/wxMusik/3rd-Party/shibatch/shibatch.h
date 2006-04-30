#ifndef shibatch_h__
#define shibatch_h__
#include "paramlist.hpp"

extern void equ_init(int wb);
extern void equ_makeTable(float *lbc,float *rbc,paramlist *,float fs);
extern void equ_quit(void);
extern int equ_modifySamples(char *buf,int nsamples,int nch,int bps);
extern void equ_clearbuf(int,int);


class ShibatchEQ
{
public:
    enum {Bands = 18};
    enum Channel {Left=0,Right=1,NChannels};
    ShibatchEQ(float freq)
    {
        m_bEnabled = false;
        for(size_t c = 0 ;c < NChannels;++c)
        for(size_t i = 0; i < Bands;++i)
        {
            m_Bands[c][i]= 1.0f;  
        }
        m_Frequency = freq;
    }
    ~ShibatchEQ()
    {
        if(m_bEnabled)
            equ_quit();
    }
    void SetBand(Channel c,size_t i, float data )
    {
        m_Bands[c][i]=data;
    }
    float GetBand(Channel c,size_t i)
    {
        return m_Bands[c][i];
    }
    static size_t GetBandCount()
    {
        return Bands;
    }
    static int GetBandFreq(size_t i)
    {
        static int freqs[Bands] = {55,77,110,156,220,311,440,622,880,1244,1760,2489,3520,4978,7040,9956,14080,19912};
        return freqs[i];
    }

    void ApplyChanges()
    {
        if(m_bEnabled)
            equ_makeTable( m_Bands[Left], m_Bands[Right], &m_paramroot, m_Frequency );
    }
    
    void SetFreq(float freq)
    {
        m_Frequency = freq;
    }
    bool Enabled()
    {
        return m_bEnabled;
    }
    void Enable(bool e)
    {
        if(e)
        {
            if(m_bEnabled)
                return;
            equ_init( 14 );		//--- no one knows why, 14 is the magic number ---//
            m_bEnabled = true;
            ApplyChanges();
        }
        else
        {
            if(m_bEnabled)
                equ_quit();
            m_bEnabled = false;
        }
    }
    void ProcessSamples( void *pBuffer, int length, int channels, int bitspersample )
    {
        equ_modifySamples( (char*)pBuffer, length, channels, bitspersample );
    }
protected:
    float m_Bands[NChannels][Bands];
    bool m_bEnabled;
    float	m_Frequency;
    paramlist m_paramroot;
};

#endif // shibatch_h__