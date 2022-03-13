#include "plugin_header.h"


typedef float real32;
typedef unsigned int u32;

enum random_enum{
    A, B, C
};




struct Parameters{
    //INT_ARAM(0, 4) test_int_param;
    FLOAT_ARAM(0.0f, 1.0f) gain;
    FLOAT_ARAM(500.0f, 20000.0f) frequency;
    ENUM_ARAM(random_enum) test_enum_param;
};

struct State{
    double theta;
};


Parameters default_parameters()
{
    Parameters initial_state = {0.01f, 500.0f};//, 2000.0f};
    return initial_state;
}

State initialize_state(const Parameters& param, 
                       const unsigned int num_channels, 
                       const float sample_rate,
                       void *allocator)
{
    State initial_state = {0.0f};
    return initial_state;
}

void audio_callback(const Parameters& param,
                    State& state,
                    float** out_buffer, 
                    const u32  num_channels, 
                    const u32  num_samples,
                    const real32 sample_rate)
{
    float x = 1.0f;
    float gain = param.gain;
    double lfo_freq = param.frequency;
    double lfo_step = two_pi * lfo_freq / (double(sample_rate));
    for(int sample = 0; sample < num_samples; sample++)
    {
        
        double lfo_gain = cos_64(state.theta);
        for(int channel = 0; channel < num_channels; channel++)
        {
            out_buffer[channel][sample] = lfo_gain * gain;
            //out_buffer[channel][sample] = gain * x;
            //out_buffer[channel][sample] = gain * lfo_gain * lfo_gain * lfo_gain * out_buffer[channel][sample];
        }
        
        x = -x;
        state.theta += lfo_step;
        if(state.theta > two_pi)
            state.theta -= two_pi;
    }
}

