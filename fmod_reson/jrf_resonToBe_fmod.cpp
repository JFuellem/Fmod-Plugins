//
//  jrf_resonToBe_fmod.cpp
//  jrf_resonToBe_fmod
//
//  Created by Jonas Füllemann on 02.11.23.
//

#include <stdio.h>

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <vector>

#include "fmod.hpp"
#include "jrf_reson.hpp"

extern "C" {
    F_EXPORT FMOD_DSP_DESCRIPTION* F_CALL FMODGetDSPDescription();
}


enum
{
    FMOD_RESON_PARAM_PITCH = 0,
    FMOD_RESON_PARAM_Q,
    FMOD_RESON_NUM_PARAMETERS
};

FMOD_RESULT F_CALLBACK FMOD_RESON_dspcreate (FMOD_DSP_STATE *dsp_state);
FMOD_RESULT F_CALLBACK FMOD_RESON_dsprelease (FMOD_DSP_STATE *dsp_state);
FMOD_RESULT F_CALLBACK FMOD_RESON_dspreset (FMOD_DSP_STATE *dsp_state);
FMOD_RESULT F_CALLBACK FMOD_RESON_dspprocess (FMOD_DSP_STATE *dsp_state, unsigned int length, const FMOD_DSP_BUFFER_ARRAY *inbufferarray, FMOD_DSP_BUFFER_ARRAY *outbufferarray, FMOD_BOOL inputsidle, FMOD_DSP_PROCESS_OPERATION op);
FMOD_RESULT F_CALLBACK FMOD_RESON_dspsetparamfloat (FMOD_DSP_STATE *dsp_state, int index, float value);
FMOD_RESULT F_CALLBACK FMOD_RESON_dspgetparamfloat (FMOD_DSP_STATE *dsp_state, int index, float *value, char *valuestr);
FMOD_RESULT F_CALLBACK FMOD_RESON_shouldiprocess (FMOD_DSP_STATE *dsp_state, FMOD_BOOL inputsidle, unsigned int length, FMOD_CHANNELMASK inmask, int inchannels, FMOD_SPEAKERMODE speakermode);
FMOD_RESULT F_CALLBACK FMOD_RESON_sys_register (FMOD_DSP_STATE *dsp_state);
FMOD_RESULT F_CALLBACK FMOD_RESON_sys_deregister (FMOD_DSP_STATE *dsp_state);
FMOD_RESULT F_CALLBACK FMOD_RESON_sys_mix (FMOD_DSP_STATE *dsp_state, int stage);

static bool FMOD_Reson_Running = false;
static FMOD_DSP_PARAMETER_DESC p_pitch;
static FMOD_DSP_PARAMETER_DESC p_q;

FMOD_DSP_PARAMETER_DESC *FMOD_RESON_dspparam[FMOD_RESON_NUM_PARAMETERS] =
{
    &p_pitch,
    &p_q
};

FMOD_DSP_DESCRIPTION FMOD_RESON_Desc =
{
    FMOD_PLUGIN_SDK_VERSION,
    "JRF ResonToBe", //name
    0x00010000, //plugin version
    1, //inbuffer
    1, //outbuffer
    FMOD_RESON_dspcreate,
    FMOD_RESON_dsprelease,
    FMOD_RESON_dspreset,
    0,
    FMOD_RESON_dspprocess,
    0,
    FMOD_RESON_NUM_PARAMETERS,
    FMOD_RESON_dspparam,
    FMOD_RESON_dspsetparamfloat, //setfloat
    0, //setint
    0, //setbool
    0, //setdata
    FMOD_RESON_dspgetparamfloat, //getfloat
    0, //getint
    0, //getbool
    0, //getdata
    FMOD_RESON_shouldiprocess,
    0, //userdata
    FMOD_RESON_sys_register,
    FMOD_RESON_sys_deregister,
    FMOD_RESON_sys_mix
};

extern "C"
{
F_EXPORT FMOD_DSP_DESCRIPTION* F_CALL FMODGetDSPDescription()
{
    float mapping_val[]  = {1, 127};
    float mapping_pos[]  = {0, 127};
    FMOD_DSP_INIT_PARAMDESC_FLOAT_WITH_MAPPING(p_pitch, "Pitch", "st", "Pitch as Midi note, Default 69", 69, mapping_val, mapping_pos);
    FMOD_DSP_INIT_PARAMDESC_FLOAT(p_q, "Bandwidth", "Hz", "Hz around the Pitch", 5, 20000 , 20000);
    return &FMOD_RESON_Desc;
}

}

FMOD_RESULT F_CALLBACK FMOD_RESON_dspcreate(FMOD_DSP_STATE *dsp_state)
{
    dsp_state->plugindata = (JRFReson *)FMOD_DSP_ALLOC(dsp_state, sizeof(JRFReson));
    if (!dsp_state->plugindata)
    {
        return FMOD_ERR_MEMORY;
    }
        
    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK FMOD_RESON_dsprelease(FMOD_DSP_STATE *dsp_state)
{
    JRFReson *state = (JRFReson *)dsp_state->plugindata;
    FMOD_DSP_FREE(dsp_state, state);
    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK FMOD_RESON_dspprocess(FMOD_DSP_STATE *dsp_state, unsigned int length, const FMOD_DSP_BUFFER_ARRAY *inbufferarray, FMOD_DSP_BUFFER_ARRAY *outbufferarray, FMOD_BOOL inputsidle, FMOD_DSP_PROCESS_OPERATION op)
{
    JRFReson *state = (JRFReson *)dsp_state->plugindata;
    
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
        state->ProcessInterleaved(inbufferarray[0].buffers[0], outbufferarray[0].buffers[0], length, inbufferarray[0].buffernumchannels[0]);
    }
    
    return FMOD_OK;
}


FMOD_RESULT F_CALLBACK FMOD_RESON_dspreset(FMOD_DSP_STATE *dsp_state)
{
    int s_sampRate;
    JRFReson *state = (JRFReson *)dsp_state->plugindata;
    dsp_state->functions->getsamplerate(dsp_state, &s_sampRate);
    state->Reset(s_sampRate);
    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK FMOD_RESON_dspsetparamfloat(FMOD_DSP_STATE *dsp_state, int index, float value)
{
    JRFReson *state = (JRFReson *)dsp_state->plugindata;
    
    switch (index)
    {
        case FMOD_RESON_PARAM_PITCH:
            state->SetFreqAsMidi(value);
            return FMOD_OK;
        case FMOD_RESON_PARAM_Q:
            state->SetBandwidth(value);
            return FMOD_OK;
    }
    return FMOD_ERR_INVALID_PARAM;
}

FMOD_RESULT F_CALLBACK FMOD_RESON_dspgetparamfloat(FMOD_DSP_STATE *dsp_state, int index, float *value, char *valuestr)
{
    JRFReson *state = (JRFReson *)dsp_state->plugindata;
    
    switch (index)
    {
        case FMOD_RESON_PARAM_PITCH:
            *value = state->GetFromMidi();
            return FMOD_OK;
        case FMOD_RESON_PARAM_Q:
            *value = state->GetBandwidth();
            return FMOD_OK;
    }
    
    return FMOD_ERR_INVALID_PARAM;
}

FMOD_RESULT F_CALLBACK FMOD_RESON_shouldiprocess(FMOD_DSP_STATE * /*dspstate*/, FMOD_BOOL inputsidle, unsigned int /*len*/, FMOD_CHANNELMASK , int /*inchans*/, FMOD_SPEAKERMODE /*speakermode*/)
{
    if (inputsidle)
    {
        return FMOD_ERR_DSP_DONTPROCESS;
    }
    
    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK FMOD_RESON_sys_register(FMOD_DSP_STATE * /*state*/)
{
    FMOD_Reson_Running = true;
    
    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK FMOD_RESON_sys_deregister(FMOD_DSP_STATE * /*state*/)
{
    FMOD_Reson_Running = false;
    
    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK FMOD_RESON_sys_mix(FMOD_DSP_STATE * /*state*/, int /*stage*/)
{
    return FMOD_OK;
}

