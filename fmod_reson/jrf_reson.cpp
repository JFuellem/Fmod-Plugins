//
//  jrf_reson.cpp
//  JRFReson - Shared Code
//
//  Created by Jonas Füllemann on 30.10.23.
//

#include "jrf_reson.hpp"

JRFReson::JRFReson() : frequency(440), bandwidth(20000), sampRate(48000)
{
    Init();
}

JRFReson::~JRFReson()
{
}

void JRFReson::Init()
{
    ComputeBalanceingCoefficients();
    ComputeCoefficients();
}

void JRFReson::ComputeCoefficients()
{
    cosFreq = cos(frequency * tpidsr);
    c3 = exp(bandwidth * -(tpidsr));
    
    c3p1 = c3 + 1.0;
    c3t4 = c3 * 4.0;
    omc3 = 1.0 - c3;
    c2 = c3t4 * cosFreq / c3p1;
    c2sqr = c2 * c2;
    c1 = omc3 * sqrt(1.0 - c2sqr / c3t4);

}

void JRFReson::ComputeBalanceingCoefficients()
{
    tpidsr = 2.0 * M_PI / sampRate;
    auto b = 2.0 -  cos((double) (1 * tpidsr));
    bc2 = b - sqrt(b * b - 1.0);
    bc1 = 1.0 - bc2;
    
    for (size_t i = 0; i<MAX_CHANS;i++)
    {
        q[i] = r[i] = 0.0;
    }
    
    a = 0.0;
}

void JRFReson::Process(const float *inBuff, float *outBuff, unsigned int buffSize, int currChan)
{
    while (buffSize--)
    {
        tempIn = *inBuff;
        tempOut = c1 * tempIn + c2 * y1[currChan] - c3 * y2[currChan];

        *outBuff = tempOut * CalculateBalance(currChan);
        
        y2[currChan] = y1[currChan];
        y1[currChan] = tempOut;
        
        inBuff++;
        outBuff++;
    }
}

void JRFReson::ProcessInterleaved(const float *inBuff, float *outBuff, unsigned int buffSize, int numChans)
{

    while (buffSize--)
    {
        for(c = 0; c < numChans; c++)
        {
            tempIn = *inBuff;
            tempOut = c1 * tempIn + c2 * y1[c] - c3 * y2[c];
            
            *outBuff = tempOut * CalculateBalance(c);
            
            y2[c] = y1[c];
            y1[c] = tempOut;
            
            inBuff++;
            outBuff++;
        }
    }
}

double JRFReson::CalculateBalance(size_t currChan)
{
    q[currChan] = bc1 * tempOut * tempOut + bc2 * q[currChan];
    r[currChan] = bc1 * tempIn * tempIn + bc2 * r[currChan];
    if(q[currChan] != 0)
        return sqrt(r[currChan]/q[currChan]);
    else
        return sqrt(r[currChan]);
}

void JRFReson::SetFreq(float s_freq)
{
    frequency = s_freq;
    ComputeCoefficients();
    
}

void JRFReson::SetFreqAsMidi(float s_freq)
{
    frequency = m_to_f(s_freq);
    ComputeCoefficients();
    
}

void JRFReson::SetBandwidth(float s_bandwidth)
{
    bandwidth = s_bandwidth;
    ComputeCoefficients();
    
}
void JRFReson::SetSampRate(unsigned int s_sampRate)
{
    sampRate = s_sampRate;
    Init();
}

void JRFReson::Reset(unsigned int s_sampleRate)
{
    SetSampRate(s_sampleRate);
}
