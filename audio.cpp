#include "string.h"
#include "math.h"
#include "windows.h"
#include "stdio.h"

#include "base.h"
#include "structs.h"
#include "descriptor.h"
#include "audio.h"




void render_audio(real32** output_buffer, Audio_Parameters parameters, Audio_Context* ctx)
{
    
    for(auto channel = 0; channel < parameters.num_channels; channel++)
    {
        memset(output_buffer[channel], 0, parameters.num_samples * sizeof(real32));
    }
    
    if(InterlockedCompareExchange((LONG volatile *) ctx->audio_file_stage,
                                  Asset_File_Stage_IN_USE,
                                  Asset_File_Stage_STAGE_USAGE)
       == Asset_File_Stage_STAGE_USAGE)
    {
        ctx->audio_file_buffer = ctx->new_audio_file_buffer;
        ctx->audio_file_length = ctx->new_audio_file_length;
        ctx->audio_file_read_cursor = ctx->new_audio_file_read_cursor;
        ctx->audio_file_num_channels = ctx->new_audio_file_num_channels;
    }
    
    if(InterlockedCompareExchange((LONG volatile *) ctx->plugin_stage,
                                  Asset_File_Stage_IN_USE,
                                  Asset_File_Stage_STAGE_USAGE)
       == Asset_File_Stage_STAGE_USAGE)
    {
        
    }
    
    auto plugin_stage = *ctx->plugin_stage;
    auto audio_file_stage = *ctx->audio_file_stage;
    
    MemoryBarrier();
    
    u8 plugin_play = ctx->plugin_play;
    u8 audio_file_play = ctx->audio_file_play;
    u8 audio_file_loop = ctx->audio_file_loop;
    
    if(audio_file_stage == Asset_File_Stage_IN_USE &&
       audio_file_play)
    {
        real32** audio_file_buffer = ctx->audio_file_buffer;
        u64 channels_to_write = octave_min(ctx->audio_file_num_channels, parameters.num_channels);
        u64 audio_file_length = ctx->audio_file_length;
        u64 read_cursor = ctx->audio_file_read_cursor;
        u64 original_read_cursor = read_cursor;
        
        MemoryBarrier();
        
        if(audio_file_loop == 0)
        {
            u64 samples_left_in_file = audio_file_length - read_cursor;
            u64 samples_to_write  = octave_min(samples_left_in_file, parameters.num_samples);
            
            //copy audio
            for(auto channel = 0; channel < channels_to_write; channel++)
            {
                copy(audio_file_buffer[channel] + read_cursor, output_buffer[channel], samples_to_write);
            }
            
            //zero remaining of buffer
            for(auto channel = 0; channel < channels_to_write; channel++)
            {
                for(auto sample = samples_to_write; sample < parameters.num_samples; sample++)
                {
                    output_buffer[channel][sample] = 0.0f;
                }
            }
            
            read_cursor += samples_to_write;
            
            if(samples_to_write == samples_left_in_file)
            {
                ctx->audio_file_play = 0;
                read_cursor = 0;
            }
        }
        else
        {
            u64 written_samples = 0;
            
            while(written_samples < parameters.num_samples)
            {
                u64 samples_left_in_file = audio_file_length - read_cursor;
                u64 samples_left_in_buffer = parameters.num_samples - written_samples; 
                
                if(samples_left_in_file > samples_left_in_buffer)
                {
                    for(auto channel = 0; channel < channels_to_write; channel++)
                    {
                        copy(audio_file_buffer[channel] + read_cursor, 
                             output_buffer[channel] + written_samples, 
                             samples_left_in_buffer);
                    }
                    read_cursor += samples_left_in_buffer;
                    written_samples += samples_left_in_buffer;
                }
                else
                {
                    for(auto channel = 0; channel < channels_to_write; channel++)
                    {
                        copy(audio_file_buffer[channel] + read_cursor, 
                             output_buffer[channel] + written_samples, 
                             samples_left_in_file);
                    }
                    written_samples += samples_left_in_file;
                    read_cursor = 0;
                }
            }
        }
        
        InterlockedCompareExchange64((LONG64 volatile *)&ctx->audio_file_read_cursor,
                                     read_cursor,
                                     original_read_cursor);
        //zero buffers that don't map to a channel 
        for(auto channel = channels_to_write; channel < parameters.num_channels; channel++)
        {
            memset(output_buffer[channel], 0, parameters.num_samples * sizeof(real32));
        }
    }
    
    if(plugin_stage == Asset_File_Stage_IN_USE && 
       plugin_play == 1)
    {
        ctx->audio_callback_f(ctx->plugin_parameters_holder,
                              ctx->plugin_state_holder,
                              output_buffer,
                              parameters.num_channels,
                              parameters.num_samples,
                              parameters.sample_rate);
        
    }
    
    InterlockedCompareExchange((LONG volatile *) ctx->audio_file_stage,
                               Asset_File_Stage_OK_TO_UNLOAD,
                               Asset_File_Stage_STAGE_UNLOADING);
    InterlockedCompareExchange((LONG volatile *) ctx->plugin_stage,
                               Asset_File_Stage_OK_TO_UNLOAD,
                               Asset_File_Stage_STAGE_UNLOADING);
    
    InterlockedCompareExchange((LONG volatile *) ctx->audio_file_stage,
                               Asset_File_Stage_OK_TO_SWITCH,
                               Asset_File_Stage_STAGE_SWITCHING);
    InterlockedCompareExchange((LONG volatile *) ctx->plugin_stage,
                               Asset_File_Stage_OK_TO_SWITCH,
                               Asset_File_Stage_STAGE_SWITCHING);
    
}




