//Copyright (c) 2004, Gunnar Roth
//All rights reserved.
//
//Redistribution and use in source and binary forms, with or without modification, are permitted provided 
//that the following conditions are met:
//
//    * Redistributions of source code must retain the above copyright notice, this list of conditions and 
//      the following disclaimer.
//    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions 
//      and the following disclaimer in the documentation and/or other materials provided with the distribution.
//    * Neither the name of the Gunnar Roth nor the names of its contributors may be used to endorse or 
//      promote products derived from this software without specific prior written permission.
//
//THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
//INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
//DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
//SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
//SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
//WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
//OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef MUSIKENGINE_EQUALIZER_H
#define MUSIKENGINE_EQUALIZER_H

#include "engine.h"
#include <vector>

class MUSIKEqualizer
{
public:
    class Bands 
    {
    public:
        class Band 
        {
        public:
            explicit Band()
                :m_pBands(NULL)
                ,m_Index((size_t)-1)
                ,m_Value(0.0)
                ,m_Frequency(0)
            {

            }
            explicit Band(Bands & bands,size_t i, int f,double v)
                :m_pBands(&bands)
                ,m_Index(i)
                ,m_Value(v)
                ,m_Frequency(f)
            {

            }
            void operator=(double v)
            {
                if(m_pBands && m_pBands->OnChange(this,v))
                    m_Value = v;
            }
            Band(const Band & rhs)
            {
                *this = rhs;
            }
            const Band & operator=(const Band & rhs)
            {
                if(this != &rhs)
                {
                    m_Index = rhs.m_Index;
                    m_Value = rhs.m_Value;
                    m_pBands = rhs.m_pBands;  
                    m_Frequency = rhs.m_Frequency;
                }
                return *this;
            }
            operator double()
            {
                return m_Value;
            }
            size_t Index() const
            {
                return m_Index;
            }
            int Frequency() const
            {
                return m_Frequency;
            }
        protected:
        private:
            Bands *m_pBands;
            size_t m_Index;
            double m_Value;
            int    m_Frequency;
            friend class Bands;
        };
        
    public:
        enum Channel {Left,Right};

        explicit Bands(MUSIKEqualizer &eq,Channel channel,size_t cntBands)
            :m_Eq(eq)
            ,m_Channel(channel)
        {
            m_vecBand.resize(cntBands);
        }
        void Reset()
        {
            for(size_t i = 0; i < m_vecBand.size();++i)
            {
                m_vecBand[i] = Band(*this,i,m_Eq.GetBandFrequency(i),1.0);
            }
        }
        size_t Count() const
        {
            return m_vecBand.size();   
        }
        enum Channel Channel() const
        {
            return m_Channel;
        }
        Band & operator[](size_t i)
        {
            return m_vecBand[i];
        }
    protected:
        bool OnChange(Band *p,double v)
        {
            return  m_Eq.OnBandChange(this,p->Index(),v);
        }
    private:
        std::vector<Band> m_vecBand;
        MUSIKEqualizer & m_Eq;
        enum Channel m_Channel;
        friend class Band;
    };
public: 
   
	explicit MUSIKEqualizer(double f,size_t cntBands)
        :m_LeftBands(*this,Bands::Left,cntBands),
        m_RightBands(*this,Bands::Right,cntBands),
        m_Frequency(f)
	{
	}
    virtual ~MUSIKEqualizer()
    {    }
    virtual void Enable(bool e) = 0;
    virtual bool Enabled() = 0;
    virtual void SetFrequency(double f)
    {
        m_Frequency = f;
    }
    double GetFrequency() const
    {
        return m_Frequency;
    }
    virtual void ApplyChanges() = 0;
    Bands & ChannelBands(enum Bands::Channel c) 
    {
        if(m_LeftBands[0].Index() == (size_t)-1)
            Reset(); // call reset to initialize the bands
        switch(c)
        {
        case Bands::Left:
            return m_LeftBands;
        case Bands::Right:
            return m_RightBands;
        }
        return m_RightBands;
    }
    void Reset()
    {
        m_LeftBands.Reset();
        m_RightBands.Reset();
    }
protected:
    virtual bool OnBandChange(Bands * p,size_t i,double v) = 0;
    virtual int GetBandFrequency(size_t i) const = 0;
 private:
    Bands m_LeftBands;
    Bands m_RightBands;
    double m_Frequency;
    friend class Bands;
};

#endif
