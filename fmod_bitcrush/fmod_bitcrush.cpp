//
//  fmod_bitcrush.cpp
//  fmod_bitcrush
//
//  Created by Jonas Füllemann on 01.03.23.
//

#include <stdio.h>

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <vector>

#include "fmod.hpp"

extern "C" {
    F_EXPORT FMOD_DSP_DESCRIPTION* F_CALL FMODGetDSPDescription();
}

const float FMOD_BITCR_PARAM_BITDPT_MIN = 2;
const float FMOD_BITCR_PARAM_BITDPT_MAX = 24;
const float FMOD_BITCR_PARAM_DIVIDE_MIN = 1;
const float FMOD_BITCR_PARAM_DIVIDE_MAX = 128;

enum
{
    FMOD_BITCR_PARAM_BITDPT = 0,
    FMOD_BITCR_PARAM_DIVIDE,
    FMOD_BITCR_NUM_PARAMETERS
};

FMOD_RESULT F_CALLBACK FMOD_Bitcr_dspcreate (FMOD_DSP_STATE *dsp_state);
FMOD_RESULT F_CALLBACK FMOD_Bitcr_dsprelease (FMOD_DSP_STATE *dsp_state);
FMOD_RESULT F_CALLBACK FMOD_Bitcr_dspreset (FMOD_DSP_STATE *dsp_state);
FMOD_RESULT F_CALLBACK FMOD_Bitcr_dspprocess (FMOD_DSP_STATE *dsp_state, unsigned int length, const FMOD_DSP_BUFFER_ARRAY *inbufferarray, FMOD_DSP_BUFFER_ARRAY *outbufferarray, FMOD_BOOL inputsidle, FMOD_DSP_PROCESS_OPERATION op);
FMOD_RESULT F_CALLBACK FMOD_Bitcr_dspsetparamfloat (FMOD_DSP_STATE *dsp_state, int index, float value);
FMOD_RESULT F_CALLBACK FMOD_Bitcr_dspgetparamfloat (FMOD_DSP_STATE *dsp_state, int index, float *value, char *valuestr);
FMOD_RESULT F_CALLBACK FMOD_Bitcr_shouldiprocess (FMOD_DSP_STATE *dsp_state, FMOD_BOOL inputsidle, unsigned int length, FMOD_CHANNELMASK inmask, int inchannels, FMOD_SPEAKERMODE speakermode);
FMOD_RESULT F_CALLBACK FMOD_Bitcr_sys_register (FMOD_DSP_STATE *dsp_state);
FMOD_RESULT F_CALLBACK FMOD_Bitcr_sys_deregister (FMOD_DSP_STATE *dsp_state);
FMOD_RESULT F_CALLBACK FMOD_Bitcr_sys_mix (FMOD_DSP_STATE *dsp_state, int stage);

static bool FMOD_Bitcr_Running = false;
static FMOD_DSP_PARAMETER_DESC p_bitdpt;
static FMOD_DSP_PARAMETER_DESC p_divide;

FMOD_DSP_PARAMETER_DESC *FMOD_Bitcr_dspparam[FMOD_BITCR_NUM_PARAMETERS] =
{
    &p_bitdpt,
    &p_divide
};

FMOD_DSP_DESCRIPTION FMOD_Bitcr_Desc =
{
    FMOD_PLUGIN_SDK_VERSION,
    "JRF Bitcrusher", //name
    0x00010000, //plugin version
    1, //inbuffer
    1, //outbuffer
    FMOD_Bitcr_dspcreate,
    FMOD_Bitcr_dsprelease,
    FMOD_Bitcr_dspreset,
    0,
    FMOD_Bitcr_dspprocess,
    0,
    FMOD_BITCR_NUM_PARAMETERS,
    FMOD_Bitcr_dspparam,
    FMOD_Bitcr_dspsetparamfloat, //setfloat
    0, //setint
    0, //setbool
    0, //setdata
    FMOD_Bitcr_dspgetparamfloat, //getfloat
    0, //getint
    0, //getbool
    0, //getdata
    FMOD_Bitcr_shouldiprocess,
    0, //userdata
    FMOD_Bitcr_sys_register,
    FMOD_Bitcr_sys_deregister,
    FMOD_Bitcr_sys_mix
};

extern "C"
{
F_EXPORT FMOD_DSP_DESCRIPTION* F_CALL FMODGetDSPDescription()
{
    FMOD_DSP_INIT_PARAMDESC_FLOAT(p_bitdpt, "BitDepth", "bit", "Bit depth in bits. Default = 24", FMOD_BITCR_PARAM_BITDPT_MIN, FMOD_BITCR_PARAM_BITDPT_MAX, FMOD_BITCR_PARAM_BITDPT_MAX);
    FMOD_DSP_INIT_PARAMDESC_FLOAT(p_divide, "Divide", "x", "Sample Divide. Default = 1", FMOD_BITCR_PARAM_DIVIDE_MIN, FMOD_BITCR_PARAM_DIVIDE_MAX, FMOD_BITCR_PARAM_DIVIDE_MIN);
    return &FMOD_Bitcr_Desc;
}

}

class FMODBitcrState
{
public:
    FMODBitcrState();
    
    void read(float *inbuffer, float *outbuffer, unsigned int length, int channels);
    void reset();
    void setBitcr(float);
    void setDivide(float);
    void CalculateFactor();
    float bitCrush() { return m_target_crush; };
    float divide() {return m_target_divide; };
    
private:
    float m_target_crush;
    float m_target_divide;
    float m_bitFactor;
};

FMODBitcrState::FMODBitcrState()
{
    reset();
}

void FMODBitcrState::CalculateFactor()
{
    m_bitFactor = powf(2, m_target_crush - 1) - 1;
}

void FMODBitcrState::reset()
{
}

void FMODBitcrState::read(float *inbuffer, float *outbuffer, unsigned int length, const int channels)
{
    int divideValInt = (int)ceilf(m_target_divide);
    
    std::vector<float> lastSample(channels);
    
    for(size_t i = 0; i < length; i++)
    {
        if(!(i % divideValInt))
        {
            for (size_t curChan = 0; curChan < channels; curChan++)
            {
                *outbuffer++ = lastSample[curChan] = roundf(*inbuffer++ * m_bitFactor) / m_bitFactor;
            }
        }
        else
        {
            for (size_t curChan = 0; curChan < channels; curChan++)
            {
                *outbuffer++ = lastSample[curChan];
                inbuffer++;
            }
        }
    }
}

void FMODBitcrState::setBitcr(float bitCrVal)
{
    m_target_crush = bitCrVal;
    CalculateFactor();
}

void FMODBitcrState::setDivide(float divideVal)
{
    m_target_divide = divideVal;
}

FMOD_RESULT F_CALLBACK FMOD_Bitcr_dspcreate(FMOD_DSP_STATE *dsp_state)
{
    dsp_state->plugindata = (FMODBitcrState *)FMOD_DSP_ALLOC(dsp_state, sizeof(FMODBitcrState));
    if (!dsp_state->plugindata)
    {
        return FMOD_ERR_MEMORY;
    }
        
    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK FMOD_Bitcr_dsprelease(FMOD_DSP_STATE *dsp_state)
{
    FMODBitcrState *state = (FMODBitcrState *)dsp_state->plugindata;
    FMOD_DSP_FREE(dsp_state, state);
    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK FMOD_Bitcr_dspprocess(FMOD_DSP_STATE *dsp_state, unsigned int length, const FMOD_DSP_BUFFER_ARRAY *inbufferarray, FMOD_DSP_BUFFER_ARRAY *outbufferarray, FMOD_BOOL inputsidle, FMOD_DSP_PROCESS_OPERATION op)
{
    FMODBitcrState *state = (FMODBitcrState *)dsp_state->plugindata;
    
    if(op == FMOD_DSP_PROCESS_QUERY)
    {
        if (outbufferarray && inbufferarray)
        {
            outbufferarray[0].buffernumchannels[0] = inbufferarray[0].buffernumchannels[0];
            outbufferarray[0].speakermode = inbufferarray[0].speakermode;
        }
        
        if (inputsidle)
        {
            return FMOD_ERR_DSP_DONTPROCESS;
        }
    }
    else
    {
        state->read(inbufferarray[0].buffers[0], outbufferarray[0].buffers[0], length, inbufferarray[0].buffernumchannels[0]);
    }
    
    return FMOD_OK;
}


FMOD_RESULT F_CALLBACK FMOD_Bitcr_dspreset(FMOD_DSP_STATE *dsp_state)
{
    FMODBitcrState *state = (FMODBitcrState *)dsp_state->plugindata;
    state->reset();
    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK FMOD_Bitcr_dspsetparamfloat(FMOD_DSP_STATE *dsp_state, int index, float value)
{
    FMODBitcrState *state = (FMODBitcrState *)dsp_state->plugindata;
    
    switch (index)
    {
        case FMOD_BITCR_PARAM_DIVIDE:
            state->setDivide(value);
            return FMOD_OK;
        case FMOD_BITCR_PARAM_BITDPT:
            state->setBitcr(value);
            return FMOD_OK;
    }
    return FMOD_ERR_INVALID_PARAM;
}

FMOD_RESULT F_CALLBACK FMOD_Bitcr_dspgetparamfloat(FMOD_DSP_STATE *dsp_state, int index, float *value, char *valuestr)
{
    FMODBitcrState *state = (FMODBitcrState *)dsp_state->plugindata;
    
    switch (index)
    {
        case FMOD_BITCR_PARAM_DIVIDE:
            *value = state->divide();
            return FMOD_OK;
        case FMOD_BITCR_PARAM_BITDPT:
            *value = state->bitCrush();
            return FMOD_OK;
    }
    
    return FMOD_ERR_INVALID_PARAM;
}

FMOD_RESULT F_CALLBACK FMOD_Bitcr_shouldiprocess(FMOD_DSP_STATE * /*dspstate*/, FMOD_BOOL inputsidle, unsigned int /*len*/, FMOD_CHANNELMASK , int /*inchans*/, FMOD_SPEAKERMODE /*speakermode*/)
{
    if (inputsidle)
    {
        return FMOD_ERR_DSP_DONTPROCESS;
    }
    
    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK FMOD_Bitcr_sys_register(FMOD_DSP_STATE * /*state*/)
{
    FMOD_Bitcr_Running = true;
    
    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK FMOD_Bitcr_sys_deregister(FMOD_DSP_STATE * /*state*/)
{
    FMOD_Bitcr_Running = false;
    
    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK FMOD_Bitcr_sys_mix(FMOD_DSP_STATE * /*state*/, int /*stage*/)
{
    return FMOD_OK;
}
