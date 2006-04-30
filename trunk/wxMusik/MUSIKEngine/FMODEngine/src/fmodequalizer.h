#ifndef fmodequalizer_h__
#define fmodequalizer_h__

#include "MUSIKEngine/inc/equalizer.h"
class ShibatchEQ;
struct FSOUND_DSPUNIT;
class FMODEqualizer : public MUSIKEqualizer
{
public:
    FMODEqualizer(int nFrequency);
    ~FMODEqualizer();

    virtual bool Enabled();
    virtual void Enable(bool e);
    virtual void ApplyChanges();
protected:
    virtual bool OnBandChange(MUSIKEqualizer::Bands * p,size_t i,double v);
    virtual int GetBandFrequency(size_t i) const;
private:
    ShibatchEQ *m_pEq;
    FSOUND_DSPUNIT	*m_DSP;
};
#endif // fmodequalizer_h__