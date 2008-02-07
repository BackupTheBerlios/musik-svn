#include "fmodexequalizer.h"
#include "shibatch/shibatch.h"
#include <fmodex/fmod.hpp>
#include <fmodex/fmod_errors.h>

static FMOD_RESULT F_CALLBACK dspcallback(FMOD_DSP_STATE *dsp_state, float *inbuffer, float *outbuffer, unsigned int length, int inchannels, int outchannels)
{
	FMOD::DSP *thisdsp = (FMOD::DSP *)dsp_state->instance;
	void * userdata = NULL;
	thisdsp->getUserData((void **)&userdata);


	ShibatchEQ * p = (ShibatchEQ*)userdata;
	int bytelen = length*outchannels*sizeof(float);
	memcpy(outbuffer,inbuffer,bytelen);
	p->ProcessSamples( outbuffer, length, outchannels, -32 );
	return FMOD_OK; 
}

FMODExEqualizer::FMODExEqualizer(FMOD::System *system,int nFrequency)
:MUSIKEqualizer((double)nFrequency,ShibatchEQ::GetBandCount())
,m_pEq(new ShibatchEQ((float)GetFrequency()))
{
	/*
	Create the DSP effects.
	*/  
	FMOD_DSP_DESCRIPTION  dspdesc; 

	memset(&dspdesc, 0, sizeof(FMOD_DSP_DESCRIPTION)); 

	strcpy(dspdesc.name, "My first DSP unit"); 
	dspdesc.channels     = 0;                   // 0 = whatever comes in, else specify. 
	dspdesc.read         = dspcallback; 
	dspdesc.userdata     = (void *)m_pEq; 

	FMOD_RESULT result = system->createDSP(&dspdesc, &m_DSP); 

	/*
	Inactive by default.
	*/
	m_DSP->setBypass(true);
	result = system->addDSP(m_DSP); 

}

FMODExEqualizer::~FMODExEqualizer()
{
     m_DSP->remove();
	 m_DSP->release();
    delete m_pEq;
}

bool FMODExEqualizer::Enabled()
{
    return m_pEq->Enabled();
}

void FMODExEqualizer::Enable(bool e)
{
    if(e)
    {
        m_pEq->Enable(e);
        m_DSP->setBypass(false);
    }
    else
    {
        m_DSP->setBypass(true);
        m_pEq->Enable(e);
    }
    
}
int FMODExEqualizer::GetBandFrequency(size_t i) const
{
    return ShibatchEQ::GetBandFreq(i);
}

void FMODExEqualizer::ApplyChanges()
{
    m_pEq->SetFreq((float)GetFrequency());
    m_pEq->ApplyChanges();
}

bool FMODExEqualizer::OnBandChange(MUSIKEqualizer::Bands * p,size_t i,double v)
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