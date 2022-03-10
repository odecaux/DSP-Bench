#include "plugin_header.h"



enum slope_type{
    linear,
    logarythmic
};




struct Parameters{
    FLOAT_ARAM(0.0f, 2.0f) initial_gain;
    ENUM_ARAM(slope_type) slope;
    FLOAT_ARAM(0.0f, 0.1f) step;
};

struct State{
    
};


Parameters default_parameters()
{
    Parameters initial_state = {0.9f, linear, 0.001f};
    return initial_state;
}

State initialize_state(const Parameters& param, 
                       const unsigned int num_channels, 
                       const float sample_rate,
                       void *allocator)
{
    State initial_state = {};
    return initial_state;
}

typedef float real32;
typedef unsigned int u32;

void audio_callback(const Parameters& param,
                    State& state,
                    float** out_buffer, 
                    const u32  num_channels, 
                    const u32  num_samples,
                    const real32 sample_rate)
{
    float value = 0.5 * param.initial_gain;
    
    for(int sample = 0; sample < num_samples; sample++)
    {
        
        for(int channel = 0; channel < num_channels; channel++)
        {
            out_buffer[channel][sample] = value; 
        }
        if(param.slope == logarythmic)
        {
            //value = param.step;
        }
        else
        {
            value -= param.step;
        }
    }
}

