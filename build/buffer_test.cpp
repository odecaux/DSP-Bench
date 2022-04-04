#include "plugin_header.h"

#define BUFFER_SIZE 128

struct Parameters{
    FLOAT_PARAM(0.0f, 1.0f) dummy;
};


struct State{
    real32 **buffer;
    u32 buffer_size;
    u32 cursor;   
    void *fft_context;  
    real32 *fft_out_real; 
    real32 *fft_out_im; 
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
    State initial_state  = {};
    float freq = 1000.0f;
    u32 buffer_size = (u32)(sample_rate / freq);
    initial_state.buffer_size = BUFFER_SIZE;
    initial_state.buffer = allocate_buffers(BUFFER_SIZE, num_channels, initialization_context);
    initial_state.cursor = 0;
    /*
    real32 phase = 0.0f;
    phasor_32_array(initial_state.buffer[0], 1.0f, 0.02f, BUFFER_SIZE, &phase);
    */
    
    set_array(2.0, initial_state.buffer[0], BUFFER_SIZE);
    real32 *db = (real32*)allocate_bytes(sizeof(real32) * BUFFER_SIZE, initialization_context);
    
    //to_db_32_array(initial_state.buffer[0], initial_state.buffer[0], BUFFER_SIZE);
    gain_32_array(initial_state.buffer[0], initial_state.buffer[0], 0.45f, BUFFER_SIZE);
    //from_db_32_array(db, initial_state.buffer[0], BUFFER_SIZE);
    
    initial_state.fft_out_real = (real32*)allocate_bytes(sizeof(real32) * BUFFER_SIZE, initialization_context);
    initial_state.fft_out_im = (real32*)allocate_bytes(sizeof(real32) * BUFFER_SIZE, initialization_context);
    
    for(i32 i = 0; i < BUFFER_SIZE; i++)
    {
        initial_state.fft_out_real[i] =  0.0f;
        initial_state.fft_out_im[i] =  0.0f;
    }
    
    initial_state.fft_context = fft_initialize(initialization_context);
    
    
    //windowing_hamming(initial_state.buffer[0], initial_state.buffer[0] , BUFFER_SIZE);
    
    fft_forward(initial_state.buffer[0], initial_state.fft_out_real, initial_state.fft_out_im, BUFFER_SIZE, initial_state.fft_context);
    
    for(i32 i = 0; i < BUFFER_SIZE; i++)
    {
        initial_state.buffer[0][i] =  0.0f;
    }
    
    fft_reverse( initial_state.fft_out_real, initial_state.fft_out_im, initial_state.buffer[0], BUFFER_SIZE, initial_state.fft_context);
    
    return initial_state;
}

void audio_callback(const Parameters& param,
                    State& state,
                    float** out_buffer, 
                    const u32  num_channels, 
                    const u32  num_samples,
                    const real32 sample_rate)
{
    for(int sample = 0; sample < BUFFER_SIZE && sample < num_samples; sample++)
    {
        for(int channel = 0; channel < num_channels; channel++)
        {
            out_buffer[channel][sample] =  state.buffer[0][state.cursor]; 
            //out_buffer[channel][sample] =  state.fft_out_real[sample] ;
        }
        state.cursor = (state.cursor + 1) % BUFFER_SIZE;
    }
    /*
    for(int sample = BUFFER_SIZE; sample < num_samples; sample++)
    {
        for(int channel = 0; channel < num_channels; channel++)
        { 
            out_buffer[channel][sample] = 0.0f;//state.buffer[channel][state.cursor]; 
        }
        state.cursor = (state.cursor + 1) % state.bu ffer_size;
    }*/
}

