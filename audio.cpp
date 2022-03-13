#include "string.h"
#include "math.h"
#include "windows.h"
#include "stdio.h"

#include "base.h"
#include "win32_helpers.h"
#include "structs.h"
#include "plugin.h"
#include "audio.h"

void render_audio(real32** output_buffer, Audio_Parameters parameters, Audio_Thread_Context* ctx)
{
    for(auto channel = 0; channel < parameters.num_channels; channel++)
    {
        memset(output_buffer[channel], 0, parameters.num_samples * sizeof(real32));
    }
    
    if(compare_exchange_32(ctx->audio_file_state,
                           Asset_File_State_IN_USE,
                           Asset_File_State_STAGE_USAGE))
    {}
    
    if(compare_exchange_32(ctx->plugin_state,
                           Asset_File_State_IN_USE,
                           Asset_File_State_STAGE_USAGE))
    {}
    
    if(compare_exchange_32(ctx->plugin_state,
                           Asset_File_State_HOT_RELOAD_SWAPPING,
                           Asset_File_State_HOT_RELOAD_STAGE_SWAP))
    {
        ctx->hot_reload_old_plugin = ctx->plugin;
        ctx->plugin = ctx->hot_reload_plugin;
        ctx->hot_reload_plugin = nullptr;
        
        octave_assert(compare_exchange_32(ctx->plugin_state,
                                          Asset_File_State_HOT_RELOAD_STAGE_DISPOSE,
                                          Asset_File_State_HOT_RELOAD_SWAPPING));
    }
    
    
    
    if(compare_exchange_32(ctx->audio_file_state,
                           Asset_File_State_OK_TO_UNLOAD,
                           Asset_File_State_STAGE_UNLOADING))
    {
        //ctx->audio_file = nullptr;
    }
    if(compare_exchange_32(ctx->plugin_state,
                           Asset_File_State_OK_TO_UNLOAD,
                           Asset_File_State_STAGE_UNLOADING))
    {
        ctx->plugin = nullptr;
    }
    
    if(compare_exchange_32(ctx->audio_file_state,
                           Asset_File_State_COLD_RELOAD_STAGE_UNLOAD,
                           Asset_File_State_COLD_RELOAD_STAGE_UNUSE))
    {
        //ctx->audio_file = nullptr;
    }
    if(compare_exchange_32(ctx->plugin_state,
                           Asset_File_State_COLD_RELOAD_STAGE_UNLOAD,
                           Asset_File_State_COLD_RELOAD_STAGE_UNUSE))
    {
        ctx->plugin = nullptr;
    }
    
    MemoryBarrier();
    auto plugin_state = *ctx->plugin_state;
    auto audio_file_state = *ctx->audio_file_state;
    
    MemoryBarrier();
    
    u8 plugin_play = ctx->plugin_play;
    u8 audio_file_play = ctx->audio_file_play;
    u8 audio_file_loop = ctx->audio_file_loop;
    
    if(audio_file_state == Asset_File_State_IN_USE &&
       audio_file_play)
    {
        real32** audio_file_buffer = ctx->audio_file->deinterleaved_buffer;
        u64 channels_to_write = octave_min(ctx->audio_file->num_channels, parameters.num_channels);
        u64 audio_file_length = ctx->audio_file->samples_by_channel;
        u64 read_cursor = ctx->audio_file->read_cursor;
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
        
        compare_exchange_32(&ctx->audio_file->read_cursor,
                            read_cursor,
                            original_read_cursor);
        //zero buffers that don't map to a channel 
        for(auto channel = channels_to_write; channel < parameters.num_channels; channel++)
        {
            memset(output_buffer[channel], 0, parameters.num_samples * sizeof(real32));
        }
    }
    
    switch(plugin_state)
    {
        
        
        case Asset_File_State_IN_USE :
        case Asset_File_State_HOT_RELOAD_CHECK_FILE_FOR_UPDATE :
        case Asset_File_State_HOT_RELOAD_STAGE_BACKGROUND_LOADING :
        case Asset_File_State_HOT_RELOAD_BACKGROUND_LOADING :
        case Asset_File_State_HOT_RELOAD_STAGE_VALIDATION :
        case Asset_File_State_HOT_RELOAD_STAGE_SWAP :
        case Asset_File_State_HOT_RELOAD_VALIDATING :
        case Asset_File_State_HOT_RELOAD_STAGE_DISPOSE :
        case Asset_File_State_HOT_RELOAD_DISPOSING :
        {
            Plugin_Parameter_Value* maybe_new_parameter_values = plugin_parameters_buffer_pull(ctx->plugin->ring);
            if(maybe_new_parameter_values)
            {
                for(auto param_idx = 0; param_idx < ctx->plugin->ring.num_fields_by_plugin; param_idx++)
                {
                    ctx->plugin->parameter_values_audio_side[param_idx] = maybe_new_parameter_values[param_idx];
                }
                
                MemoryBarrier();
                octave_assert(ctx->plugin->ring.num_fields_by_plugin == ctx->plugin->descriptor.num_parameters);
                
                plugin_set_parameter_holder_from_values(&ctx->plugin->descriptor, 
                                                        ctx->plugin->parameter_values_audio_side, ctx->plugin->parameters_holder);
            }
            
            if(plugin_play == 1)
            {
                ctx->plugin->audio_callback_f(ctx->plugin->parameters_holder,
                                              ctx->plugin->state_holder,
                                              output_buffer,
                                              parameters.num_channels,
                                              parameters.num_samples,
                                              parameters.sample_rate);
            }
        }break;
        case Asset_File_State_HOT_RELOAD_SWAPPING :
        {
            octave_assert(false);
        }break;
    }
    
}




