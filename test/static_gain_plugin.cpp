
typedef void*(*allocator_t)(unsigned int);

struct Parameters{
};

struct State {
    float gain;
};

typedef void*(*allocator_t)(unsigned int);


Parameters default_parameters() { 
    Parameters ish = {}; 
    return ish; 
}
State initialize_state(Parameters& param, 
                       const unsigned int num_channels, 
                       const float sample_rate,
                       void *allocator) {
    State state = {0.1f};
    return state; 
}


void audio_callback(const Parameters& param,
                    State& state,
                    float** out_buffer, 
                    const unsigned int num_channels, 
                    const unsigned int num_samples,
                    const float sample_rate)
{
    for(auto channel = 0; channel < num_channels; channel++)
    {
        for(auto sample = 0; sample < num_samples; sample++)
        {
            out_buffer[channel][sample] *= state.gain;
        }
    }
}