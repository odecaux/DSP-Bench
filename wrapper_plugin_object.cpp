#include "setjmp.h"
static jmp_buf jump_buf;
static int error_flag;

void audio_callback_type_wrapper(void* param_ptr, void* state_ptr,float** out_buffer, unsigned int num_channels, unsigned int num_samples, float sample_rate);
void default_parameters_type_wrapper(void* out_parameters_ptr);
int initialize_state_type_wrapper(void* parameters_ptr, void* out_initial_state_ptr, unsigned int num_channels, float sample_rate, void *allocator);


void audio_callback_error_wrapper(void* param_ptr, void* state_ptr,float** out_buffer, unsigned int num_channels, unsigned int num_samples, float sample_rate)
{
    if(!setjmp(jump_buf))
    {
        audio_callback_type_wrapper(param_ptr, state_ptr, out_buffer, num_channels, num_samples, sample_rate);
    }
    else 
    {
        
    }
}

void default_parameters_error_wrapper(void* out_parameters_ptr)
{
    if(!setjmp(jump_buf))
    {
        default_parameters_type_wrapper(out_parameters_ptr);
    }
    else 
    {
        
    }
}

int initialize_state_error_wrapper(void* parameters_ptr, void* out_initial_state_ptr, unsigned int num_channels, float sample_rate, void *allocator)
{if(!setjmp(jump_buf))
    {
        return initialize_state_type_wrapper(parameters_ptr, out_initial_state_ptr, num_channels, sample_rate, allocator);
    }
    else 
    {
        
    }
}


































)