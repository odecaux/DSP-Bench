#include "plugin_header.h"

struct Parameters{
    FLOAT_PARAM(0.01f, 1.0f) gain;
};

struct State{
    float theta;
    void* fft_context;
    real32 *fft_real_buffer;
    real32 *fft_im_buffer;
    real32 *fft_reverse_out_buffer;
};


Parameters default_parameters()
{
    Parameters parameters = {0.01f};
    return parameters;
}

State initialize_state(const Parameters& param, 
                       const unsigned int num_channels, 
                       const float sample_rate,
                       void *initialization_context)
{
    void *fft_context = fft_initialize(initialization_context);
    
    real32 *fft_real_buffer = allocate_buffer(512, initialization_context);
    real32 *fft_im_buffer = allocate_buffer(512, initialization_context);
    real32 *fft_reverse_out_buffer = allocate_buffer(512, initialization_context);
    
    State initial_state = {0.0f, fft_context, fft_real_buffer, fft_im_buffer, fft_reverse_out_buffer};
    return initial_state;
}

void audio_callback(const Parameters& param,
                    State& state,
                    float** out_buffer, 
                    const u32  num_channels, 
                    const u32  num_samples,
                    const real32 sample_rate)
{
    real32 channel_theta;
    for(i32 channel = 0; channel < num_channels; channel++)
    {
        channel_theta = state.theta;
        sin_32_array(out_buffer[channel], param.gain, 0.02f, num_samples, &channel_theta);
        fft_forward(out_buffer[channel], state.fft_real_buffer, state.fft_im_buffer, 512, state.fft_context);
        
        gain_ip_32_array(state.fft_real_buffer, 12.0f, 512);
        gain_ip_32_array(state.fft_im_buffer, 12.0f, 512);
        
        fft_reverse(state.fft_real_buffer, state.fft_im_buffer, out_buffer[channel], 512, state.fft_context);
    }
    state.theta = channel_theta;
    
    
}
