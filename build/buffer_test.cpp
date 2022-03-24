#include "plugin_header.h"

struct Parameters{
    FLOAT_PARAM(0.0f, 1.0f) dummy;
};


struct State{
    real32 **buffer;
    u32 buffer_size;
    u32 cursor;
    void *fft_context;
    Vec2 *vec_buffer;
};




Parameters default_parameters()
{
    Parameters param = {0.0f};
    return param;
}

State initialize_state(const Parameters& param, 
                       const unsigned int num_channels, 
                       const float sample_rate,
                       void *initialization_context)
{
    State initial_state = {};
    float freq = 500.0f;
    u32 buffer_size = (u32)(sample_rate / freq);
    initial_state.buffer_size = buffer_size;
    initial_state.buffer = allocate_buffers(buffer_size, num_channels, initialization_context);
    initial_state.cursor = 0;
    
    double theta = 0.0;
    double sine_step = two_pi / (double(buffer_size));
    
    for(int sample = 0; sample < buffer_size; sample++)
    {
        double val = cos_64(theta);
        for(int channel = 0; channel < num_channels; channel++)
        {
            initial_state.buffer[channel][sample] = 0.2f * (float) val;
        }
        
        theta += sine_step;
        if(theta > two_pi)
            theta -= two_pi;
    }
    Vec2 zero_vec;
    zero_vec.x = 0.0f;
    zero_vec.y = 0.0f;
    initial_state.vec_buffer = (Vec2*)allocate_bytes(sizeof(Vec2) * 256, initialization_context);
    for(i32 i = 0; i < 256; i++)
    {
        initial_state.vec_buffer[i] =  zero_vec;
    }
    
    initial_state.fft_context = fft_initialize(initialization_context);
    fft_forward(initial_state.buffer[0], initial_state.vec_buffer, 256, initial_state.fft_context);
    
    return initial_state;
}

void audio_callback(const Parameters& param,
                    State& state,
                    float** out_buffer, 
                    const u32  num_channels, 
                    const u32  num_samples,
                    const real32 sample_rate)
{
    for(int sample = 0; sample < 256; sample++)
    {
        for(int channel = 0; channel < num_channels; channel++)
        {
            out_buffer[channel][sample] =  state.vec_buffer[sample].x ;//state.buffer[channel][state.cursor]; 
        }
        state.cursor = (state.cursor + 1) % state.buffer_size;
    }
    
    for(int sample = 256; sample < num_samples; sample++)
    {
        for(int channel = 0; channel < num_channels; channel++)
        {
            out_buffer[channel][sample] = 0.0f;//state.buffer[channel][state.cursor]; 
        }
        state.cursor = (state.cursor + 1) % state.buffer_size;
    }
}

