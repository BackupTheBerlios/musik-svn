#ifndef fmodexequalizer_h__
#define fmodexequalizer_h__

#include "MUSIKEngine/inc/equalizer.h"
class ShibatchEQ;
namespace FMOD
{
class  DSP;
class System;
}
class FMODExEqualizer : public MUSIKEqualizer
{
public:
    FMODExEqualizer(FMOD::System *system,int nFrequency);
    ~FMODExEqualizer();

    virtual bool Enabled();
    virtual void Enable(bool e);
    virtual void ApplyChanges();
protected:
    virtual bool OnBandChange(MUSIKEqualizer::Bands * p,size_t i,double v);
    virtual int GetBandFrequency(size_t i) const;
private:
    ShibatchEQ *m_pEq;
    FMOD::DSP          *m_DSP;
};
#endif // fmodexequalizer_h__
