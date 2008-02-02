#include "fmodequalizer.h"
#include "shibatch/shibatch.h"
#include <fmod/fmod.h>

void * F_CALLBACKAPI dspcallback(void * /*originalbuffer*/, void *newbuffer, int length, void * userdata)
{
    ShibatchEQ * p = (ShibatchEQ*)userdata;
	// 2 channels (stereo), 16 bit sound
    if(p->Enabled())
	    p->ProcessSamples( newbuffer, length, 2, 16 );
//     for(int i = 0 ; i < length;i++)
//            *((short*)newbuffer+i) /= 2;
	return newbuffer;
}

FMODEqualizer::FMODEqualizer(int nFrequency)
    :MUSIKEqualizer((double)nFrequency,ShibatchEQ::GetBandCount())
    ,m_pEq(new ShibatchEQ((float)GetFrequency()))
{
    m_DSP = FSOUND_DSP_Create( &dspcallback, FSOUND_DSP_DEFAULTPRIORITY_USER + 3, m_pEq );
}
FMODEqualizer::~FMODEqualizer()
{
    FSOUND_DSP_Free( m_DSP );
    delete m_pEq;
}

bool FMODEqualizer::Enabled()
{
    return m_pEq->Enabled();
}

void FMODEqualizer::Enable(bool e)
{
    if(e)
    {
        m_pEq->Enable(e);
        FSOUND_DSP_SetActive( m_DSP, e ? 1 : 0 );
    }
    else
    {
        FSOUND_DSP_SetActive( m_DSP, e ? 1 : 0 );
        m_pEq->Enable(e);
    }
    
}
int FMODEqualizer::GetBandFrequency(size_t i) const
{
    return ShibatchEQ::GetBandFreq(i);
}

void FMODEqualizer::ApplyChanges()
{
    m_pEq->SetFreq((float)GetFrequency());
    m_pEq->ApplyChanges();
}

bool FMODEqualizer::OnBandChange(MUSIKEqualizer::Bands * p,size_t i,double v)
{
    switch(p->Channel())
    {
        case MUSIKEqualizer::Bands::Left:
            m_pEq->SetBand(ShibatchEQ::Left,i,(float)v);
            break;
        case MUSIKEqualizer::Bands::Right:
            m_pEq->SetBand(ShibatchEQ::Right,i,(float)v);
            break;
    }
    return true;
}