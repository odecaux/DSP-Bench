#include "plugin_header.h"

struct Parameters{
};

struct State{
};


Parameters default_parameters()
{
    Parameters parameters = {};
    return parameters;
}

State initialize_state(const Parameters& param, 
                       const unsigned int num_channels, 
                       const float sample_rate,
                       void *initialization_context)
{
    State initial_state{};
    return initial_state;
}

void audio_callback(const Parameters& param,
                    State& state,
                    float** out_buffer, 
                    const u32  num_channels, 
                    const u32  num_samples,
                    const real32 sample_rate)
{
    
}
