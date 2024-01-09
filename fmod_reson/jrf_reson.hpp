//
//  jrf_reson.hpp
//  JRFReson - Shared Code
//
//  Created by Jonas Füllemann on 30.10.23.
//

#ifndef jrf_reson_hpp
#define jrf_reson_hpp

#define _USE_MATH_DEFINES

#include <stdio.h>
#include <cmath>

const float DEFAULT_FREQ = 1000;
const float DEFAULT_Q = 1;
#define MAX_CHANS 16

#define m_to_f(__mVal__) (440.0 * pow(2.0, ((__mVal__ - 69) / 12.0)))
#define f_to_m(__fVal__) (12.0 * log2(__fVal__ / 440.0) + 69.0)

class JRFReson
{
private:
    float frequency;
    float bandwidth;
    unsigned int sampRate = 48000;
    
    double cosFreq;
    double c1, c2, c3;
    double c3p1, c3t4, omc3, c2sqr;
    
    double tempIn;
    double tempOut;
    
    double x1[MAX_CHANS], x2[MAX_CHANS], y1[MAX_CHANS], y2[MAX_CHANS];
    
    double tpidsr;
    
    //balancing
    double bc1, bc2;
    double q[MAX_CHANS], r[MAX_CHANS], a;
    
    size_t c;
    
public:
    JRFReson();
    ~JRFReson();

    void Init();
    void ComputeCoefficients();
    
    void SetFreq(float s_freq);
    void SetFreqAsMidi(float s_midi);
    float GetFreq() {return frequency;}
    float GetFromMidi() {return m_to_f(frequency);}
    void SetBandwidth(float s_bandwidth);
    float GetBandwidth() {return bandwidth;}
    void SetSampRate(unsigned int s_sampRate);
    void Reset(unsigned int s_sampleRate);
    
    void ComputeBalanceingCoefficients();
    double CalculateBalance(size_t currChan);
    
    void Process(const float* inBuff, float* outbuff, unsigned int buffSize, int currentChannel);
    void ProcessInterleaved(const float* inBuff, float* outbuff, unsigned int buffSize, int numChans);
};

#endif /* jrf_reson_hpp */
