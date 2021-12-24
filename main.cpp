
#include "stdio.h"
#include <iostream>
#include "assert.h"

#include "base.h"

#include "windows.h"

#include "wav_reader.h"
#include "audio.h"



i32 main(i32 argc, char** argv)
{
    
    *(char*)0;
    if(argc != 3)
    {
        printf("wrong argument count\n");
        return -1;
    }
    
    
    //~
    // Audio 
    const char* audio_filename = argv[1];
    CoInitializeEx(NULL, COINIT_MULTITHREADED); 
    
    void* platform_audio_context;
    Audio_Parameters audio_parameters;
    
    Audio_Context audio_context = {};
    audio_context.file_is_valid = 0;
    
    
    if(!audio_initialize(&platform_audio_context, &audio_parameters, &audio_context))
        return -1;
    
    
    auto wav_data = windows_load_wav(audio_filename);
    
    if(wav_data.header.AudioFormat != 1)
    {
        printf("wrong audio format\n");
        return -1;
    }
    
    printf("wav loaded\n");
    
    
    auto num_samples = 8 * wav_data.audio_data_length / wav_data.header.bit_depth;
    real32* float_buffer = (real32*)malloc(num_samples * sizeof(real32)); 
    
    if(wav_data.header.bit_depth == 16)
    {
        convertInt16ToFloat(wav_data.data, float_buffer, num_samples);
    }
    else if(wav_data.header.bit_depth == 24)
    {
        convertInt24ToFloat(wav_data.data, float_buffer, num_samples);
    }
    else if(wav_data.header.bit_depth == 32)
    {
        convertInt32ToFloat(wav_data.data, float_buffer, num_samples);
    }
    
    printf("real32 convrsion done\n");
    
    u32 num_channels = wav_data.header.num_channels;
    u32 samples_by_channel = num_samples / num_channels;
    
    real32** deinterleaved_buffer = (real32**)malloc(sizeof(real32*) * num_channels);
    for(u32 channel = 0; channel < num_channels; channel++)
    {
        deinterleaved_buffer[channel] = (real32*) malloc(sizeof(real32) * samples_by_channel);
        assert(deinterleaved_buffer[channel] != NULL);
    }
    
    deinterleave(deinterleaved_buffer, float_buffer, samples_by_channel, num_channels);
    
    
    audio_context.audio_file_buffer = deinterleaved_buffer;
    audio_context.audio_file_length = samples_by_channel;
    audio_context.audio_file_read_cursor = 0;
    audio_context.audio_file_num_channels = num_channels;
    
    InterlockedExchange8(&audio_context.file_is_valid, 1);
    
    printf("initialized audio\n");
    
    
    
    
    
    //~
    //compiler stuff
    
    
    HMODULE gui_dll = LoadLibraryA("gui.dll");
    if(gui_dll == NULL)
    {
        printf("couldn't find gui.dll\n");
        return -1;
    }
    
    HMODULE compiler_dll = LoadLibraryA("compiler.dll");
    if(compiler_dll == NULL)
    {
        printf("couldn't find compiler.dll\n");
        return -1;
    }
    
    auto try_compile_f = (try_compile_t)GetProcAddress(compiler_dll, "try_compile");
    if(try_compile_f == NULL)
    {
        printf("couldnt find try_compile\n");
        return -1;
    }
    
    typedef void*(*create_clang_context_t)();
    
    void* clang_ctx = (create_clang_context_t)GetProcAddress(compiler_dll, "create_clang_context")();
    
    auto handle = try_compile_f(argv[2], clang_ctx);
    
    
    if(handle.worked)
    {
        
        //~ Application
        
        Plugin_Parameter_Value *parameter_values_audio_side = (Plugin_Parameter_Value*) 
            malloc(sizeof(Plugin_Parameter_Value) * handle.descriptor.num_parameters);
        
        Plugin_Parameter_Value *parameter_values_ui_side = (Plugin_Parameter_Value*) 
            malloc(sizeof(Plugin_Parameter_Value) * handle.descriptor.num_parameters);
        
        char* plugin_parameters_holder = (char*) malloc(handle.descriptor.parameters_struct.size);
        char* plugin_state_holder = (char*) malloc(handle.descriptor.state_struct.size);
        
        handle.default_parameters_f(plugin_parameters_holder);
        handle.initialize_state_f(plugin_parameters_holder, 
                                  plugin_state_holder, 
                                  audio_parameters.num_channels,
                                  audio_parameters.sample_rate, 
                                  &malloc_allocator
                                  );
        
        for(auto param_idx = 0; param_idx < handle.descriptor.num_parameters; param_idx++)
        {
            auto param_descriptor = handle.descriptor.parameters[param_idx];
            auto offset = param_descriptor.offset;
            switch(param_descriptor.type){
                case Int :
                {
                    parameter_values_ui_side[param_idx].int_value = *(int*)(plugin_parameters_holder + offset);
                    parameter_values_audio_side[param_idx].int_value = *(int*)(plugin_parameters_holder + offset);
                }break;
                case Float : 
                {
                    parameter_values_ui_side[param_idx].float_value = *(float*)(plugin_parameters_holder + offset);
                    parameter_values_audio_side[param_idx].float_value = *(float*)(plugin_parameters_holder + offset);
                }break;
                case Enum : 
                {
                    parameter_values_ui_side[param_idx].enum_value = *(int*)(plugin_parameters_holder + offset);
                    parameter_values_audio_side[param_idx].enum_value = *(int*)(plugin_parameters_holder + offset);
                    
                }break;
            }
        }
        Plugin_Parameters_Ring_Buffer ring = plugin_parameters_ring_buffer_initialize(handle.descriptor.num_parameters,4096);
        
        audio_context.ring = &ring;
        audio_context.audio_callback_f = handle.audio_callback_f;
        audio_context.plugin_state_holder = plugin_state_holder;
        audio_context.parameter_values_audio_side = parameter_values_audio_side;
        audio_context.descriptor = &handle.descriptor;
        InterlockedExchange8(&audio_context.plugin_is_valid, 1);
        
        
        
        
        
        typedef void(*initialize_gui_t)(Plugin_Descriptor, 
                                        Plugin_Parameter_Value*,
                                        Plugin_Parameters_Ring_Buffer*);
        
        auto initialize_gui_f  = (initialize_gui_t)GetProcAddress(gui_dll, "initialize_gui");
        if(initialize_gui_f == NULL)
        {
            printf("couldnt find initialize_gui()\n");
            return -1;
        }
        
        initialize_gui_f(handle.descriptor, parameter_values_ui_side, &ring);
        
    }
    else
    {
        printf("compilation failed\n");
    }
    
    audio_uninitialize(platform_audio_context);
    printf("done\n");
    return 0;
}
