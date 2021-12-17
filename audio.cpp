
#include "audio.h"
#include "base.h"
#include "structs.h"
#include "string.h"

#include "math.h"
#include "windows.h"
#include "stdio.h"

#define M_2I (3.1415926535897932384626433 * 2.0)


void render_audio(real32** output_buffer, Audio_Parameters parameters, Audio_Context* ctx)
{
    u8 file_is_valid = ctx->file_is_valid;
    u8 plugin_is_valid = ctx->plugin_is_valid;
    MemoryBarrier();
    
    if(!file_is_valid)
    {
        for(auto channel = 0; channel < parameters.num_channels; channel++)
            for(auto sample = 0; sample < parameters.num_samples; sample++)
            output_buffer[channel][sample] = 0.0f;
        return;
    }
    
    real32** audio_file_buffer = ctx->audio_file_buffer;
    u64 samples_to_write = octave_min(ctx->audio_file_length - ctx->audio_file_read_cursor, 
                                      parameters.num_samples);
    
    u64 channels_to_write = octave_min(ctx->audio_file_num_channels, 
                                       parameters.num_channels);
    
    u64 *read_cursor = &ctx->audio_file_read_cursor;
    
    for(auto channel = 0; channel < channels_to_write; channel++)
    {
        copy(audio_file_buffer[channel] + *read_cursor, output_buffer[channel], samples_to_write);
    }
    
    
    for(auto channel = 0; channel < channels_to_write; channel++)
    {
        for(auto sample = samples_to_write; sample < parameters.num_samples; sample++)
        {
            output_buffer[channel][sample] = 0.0f;
        }
    }
    
    for(auto channel = channels_to_write; channel < parameters.num_channels; channel++)
    {
        memset(output_buffer[channel], 0, parameters.num_samples * sizeof(real32));
    }
    
    if(plugin_is_valid)
    {
        ctx->audio_callback_f(ctx->plugin_parameters_holder, ctx->plugin_state_holder, output_buffer, parameters.num_channels, parameters.num_samples, parameters.sample_rate);
    }
    
    *read_cursor += samples_to_write;
}




