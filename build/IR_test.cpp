#include "plugin_header.h"



enum random_enum{
    A, B, C, D
};




struct Parameters{
    FLOAT_ARAM(0.0f, 1.0f) gain;
};

struct State{
    
};


Parameters default_parameters()
{
    Parameters initial_state = {0.9f};
    return initial_state;
}

State initialize_state(const Parameters& param, 
                       const unsigned int num_channels, 
                       const float sample_rate,
                       allocator_t allocator)
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
    double step = 1.0 / (double)num_samples;
    double gain = param.gain;
    
    
    for(int sample = 1; sample < num_samples; sample++)
    {
        
        for(int channel = 0; channel < num_channels; channel++)
        {
            out_buffer[channel][sample] = gain * 1.0f; 
        }
        
        //gain -= step;
        gain = gain * 0.9999f;
    }
}

