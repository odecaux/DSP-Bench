#include "plugin_header.h"


enum random_enum{
    A, B, C, D
};




struct Parameters{
    INT_ARAM(0, 4) test_int_param;
    FLOAT_ARAM(0.0f, 1.0f) gain;
    ENUM_ARAM(random_enum) test_enum_param;
};

struct State{
    double theta;
};


Parameters default_parameters()
{
    Parameters initial_state = {0,0.9f, A};
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
    float gain = param.gain;
    double lfo_freq = 440.0;
    double lfo_step = two_pi * lfo_freq / (double(sample_rate));
    for(int sample = 0; sample < num_samples; sample++)
    {
        
        double lfo_gain = cos_64(state.theta);
        for(int channel = 0; channel < num_channels; channel++)
        {
            out_buffer[channel][sample] = 0.05f * lfo_gain;
            
            //out_buffer[channel][sample] = gain * lfo_gain * lfo_gain * lfo_gain * out_buffer[channel][sample];
        }
        state.theta += lfo_step;
        if(state.theta > two_pi)
            state.theta -= two_pi;
    }
}

