#include "hardcoded_values.h"

#include "stdio.h"
#include "math.h"
#include "assert.h"
#include "windows.h"
#include "string.h"

#include "ippdefs.h"

#include "memory.h"
#include "base.h"
#include "structs.h"
#include "descriptor.h"
#include "win32_helpers.h"
#include "wav_reader.h"
#include "font.h"
#include "audio.h"
#include "fft.h"
#include "app.h"

#include "win32_platform.h"
#include "opengl.h"


bool check_extension(const char* filename, const char* extension)
{
    const char *maybe_ext = filename + strlen(filename) - 3; 
    return maybe_ext > filename && (strncmp(maybe_ext, extension, 3) == 0);
}

i32 main(i32 argc, char** argv)
{
    
    const char *source_filename = nullptr;
    const char *audio_filename = nullptr;
    if(argc == 1)
    {
        printf("please specify a wav file and a cpp file");
        return -1;
    }
    else if(argc > 3)
    {
        printf("too many arguments\n");
        return -1;
    }
    else if(argc == 2)
    {
        if(check_extension(argv[1], "cpp"))
        {
            source_filename = argv[1];
        }
        else 
        {
            printf("not a cpp file\n");
            return -1;
        }
    }
    else if(argc == 3)
    {
        if(check_extension(argv[1], "wav") && check_extension(argv[2], "cpp"))
        {
            audio_filename = argv[1];
            source_filename = argv[2];
        }
        else if(check_extension(argv[1], "cpp") && check_extension(argv[2], "wav"))
        {
            source_filename = argv[1];
            audio_filename = argv[2];
        }
        else
        {
            printf("specify a cpp and a wav file\n");
            return -1;
        }
    }
    
    if(source_filename == nullptr)
    {
        return -1;
    }
    
    CoInitializeEx(NULL, COINIT_MULTITHREADED); 
    
    void* platform_audio_context;
    Audio_Context audio_context = {};
    audio_context.audio_file_play = 0;
    Audio_Parameters audio_parameters = {};
    MemoryBarrier();
    
    if(!audio_initialize(&platform_audio_context, &audio_parameters, &audio_context))
    {
        printf("failed to initialize audio\n");
        return -1;
    }
    
    
    if(audio_filename != nullptr)
    {
        const char* audio_filename = argv[1];
        WavData wav_data = windows_load_wav(audio_filename);
        
        if(wav_data.error == Wav_Success)
        {
            auto num_samples = 8 * wav_data.audio_data_length / wav_data.header.bit_depth;
            real32* float_buffer = m_allocate_array(real32, num_samples); 
            
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
            
            free(wav_data.data);
            
            u32 num_channels = wav_data.header.num_channels;
            u32 samples_by_channel = num_samples / num_channels;
            
            real32** deinterleaved_buffer = m_allocate_array(real32*,num_channels);
            for(u32 channel = 0; channel < num_channels; channel++)
            {
                deinterleaved_buffer[channel] = m_allocate_array(real32, samples_by_channel);
            }
            
            deinterleave(deinterleaved_buffer, float_buffer, samples_by_channel, num_channels);
            free(float_buffer);
            
            audio_context.audio_file_buffer = deinterleaved_buffer;
            audio_context.audio_file_length = samples_by_channel;
            audio_context.audio_file_read_cursor = 0;
            audio_context.audio_file_num_channels = num_channels;
            audio_context.audio_file_valid = 1;
            MemoryBarrier();
        }
        else
        {
            printf("failed to read .wav file\n");
        }
    }
    
    
    //~
    //compiler stuff
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
    
    Compiler_Errors errors = {
        m_allocate_array(Compiler_Error, 1024),
        0,
        1024
    };
    Plugin_Handle handle = try_compile_f(source_filename, clang_ctx, &errors);
    
    if(handle.error != Compiler_Success)
    {
        printf("compilation errors\n");
        return -1;
    }
    
    
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
    Plugin_Parameters_Ring_Buffer ring = plugin_parameters_ring_buffer_initialize(handle.descriptor.num_parameters, RING_BUFFER_SLOT_COUNT);
    
    audio_context.ring = &ring;
    audio_context.audio_callback_f = handle.audio_callback_f;
    audio_context.plugin_state_holder = plugin_state_holder;
    audio_context.plugin_parameters_holder = plugin_parameters_holder;
    audio_context.parameter_values_audio_side = parameter_values_audio_side;
    audio_context.descriptor = &handle.descriptor;
    InterlockedExchange8(&audio_context.plugin_valid, 1);
    
    
    
    
    Graphics_Context graphics_ctx = {};
    graphics_ctx.window_dim = { INITIAL_WINDOW_WIDTH, INITIAL_WINDOW_HEIGHT, }; 
    
    Window_Context window = win32_init_window(&graphics_ctx.window_dim);
    
    graphics_ctx.atlas = {
        .font = load_fonts(DEFAULT_FONT_FILENAME),
        .draw_vertices = m_allocate_array(Vertex, ATLAS_MAX_VERTEX_COUNT),
        .draw_vertices_count = 0,
        .draw_indices = m_allocate_array(u32, ATLAS_MAX_VERTEX_COUNT), 
        .draw_indices_count = 0
    };
    
    OpenGL_Context opengl_ctx = opengl_initialize(&window, &graphics_ctx.atlas.font);
    
    //~ IR initialization
    
    real32** IR_buffer = m_allocate_array(real32*, audio_parameters.num_channels);
    for(u32 channel = 0; channel < audio_parameters.num_channels; channel++)
    {
        IR_buffer[channel] = m_allocate_array(real32, IR_BUFFER_LENGTH);
    }
    
    compute_IR(handle, IR_buffer, IR_BUFFER_LENGTH, audio_parameters, parameter_values_ui_side);
    
    
    real32** windowed_zero_padded_buffer = m_allocate_array(real32*, audio_parameters.num_channels);
    for(u32 channel = 0; channel < audio_parameters.num_channels; channel++)
    {
        windowed_zero_padded_buffer[channel] = m_allocate_array(real32, IR_BUFFER_LENGTH * 4);
        memset(windowed_zero_padded_buffer[channel], 0, IR_BUFFER_LENGTH * 4 * sizeof(real32));
        windowing_hamming(IR_buffer[channel], windowed_zero_padded_buffer[channel], IR_BUFFER_LENGTH);
    }
    
    Ipp_Context ipp_ctx = ipp_initialize();
    
    Vec2* fft_out = m_allocate_array(Vec2, IR_BUFFER_LENGTH * 4);
    fft_forward(windowed_zero_padded_buffer[0], fft_out, IR_BUFFER_LENGTH * 4, &ipp_ctx);
    
    real32 *magnitudes  = m_allocate_array(real32, IR_BUFFER_LENGTH * 4);
    for(i32 i = 0; i < IR_BUFFER_LENGTH * 4; i++)
        magnitudes[i] = sqrt(fft_out[i].a * fft_out[i].a + fft_out[i].b * fft_out[i].b);
    
    graphics_ctx.ir = {
        .IR_buffer = m_allocate_array(real32, IR_BUFFER_LENGTH),
        .IR_sample_count = IR_BUFFER_LENGTH
    };
    memcpy(graphics_ctx.ir.IR_buffer, IR_buffer[0], sizeof(real32) * IR_BUFFER_LENGTH); 
    
    graphics_ctx.fft = {
        .fft_buffer = m_allocate_array(real32, IR_BUFFER_LENGTH * 2),
        .fft_sample_count = IR_BUFFER_LENGTH * 2
    };
    memcpy(graphics_ctx.fft.fft_buffer, magnitudes, sizeof(real32) * IR_BUFFER_LENGTH * 2); 
    
    
    IO frame_io = io_initial_state();
    UI_State ui_state = {-1};
    
    bool done = false;
    
    i64 last_time = win32_init_timer();
    
    while(!done)
    {
        win32_message_dispatch(&window, &frame_io, &done);
        
        // TODO(octave): hack ?
        if(done)
            break;
        
        //~
        //frame
        graphics_ctx.atlas.draw_vertices_count = 0;
        graphics_ctx.atlas.draw_indices_count = 0;
        
        frame_io = io_state_advance(frame_io);
        frame_io.mouse_position = win32_get_mouse_position(&window);
        
        bool parameters_were_tweaked = false;
        
        frame(handle.descriptor, &graphics_ctx, ui_state, frame_io, parameter_values_ui_side, &audio_context, parameters_were_tweaked);
        
        
        if(parameters_were_tweaked)
        {
            plugin_parameters_buffer_push(ring, parameter_values_ui_side);
            compute_IR(handle, IR_buffer, IR_BUFFER_LENGTH, audio_parameters, parameter_values_ui_side);
            for(u32 channel = 0; channel < audio_parameters.num_channels; channel++)
            {
                windowing_hamming(IR_buffer[channel], windowed_zero_padded_buffer[channel], IR_BUFFER_LENGTH);
            }
            
            fft_forward(windowed_zero_padded_buffer[0], fft_out, IR_BUFFER_LENGTH * 4, &ipp_ctx);
            
            for(i32 i = 0; i < IR_BUFFER_LENGTH * 4; i++)
                magnitudes[i] = sqrt(fft_out[i].x * fft_out[i].x + fft_out[i].y * fft_out[i].y);
            
            memcpy(graphics_ctx.ir.IR_buffer, IR_buffer[0], sizeof(real32) * IR_BUFFER_LENGTH); 
            memcpy(graphics_ctx.fft.fft_buffer, magnitudes, sizeof(real32) * IR_BUFFER_LENGTH * 2);
        }
        
        opengl_render_ui(&opengl_ctx, &graphics_ctx);
        
        i64 current_time;
        win32_pace_60_fps(last_time, &current_time, &frame_io.delta_time);
        last_time = current_time;
    }
    
    
    opengl_uninitialize(&opengl_ctx);
    
    
    typedef void(*release_jit_t)(Plugin_Handle*);
    release_jit_t release_jit = (release_jit_t)GetProcAddress(compiler_dll, "release_jit");
    
    InterlockedExchange8(&audio_context.plugin_valid, 0);
    
    release_jit(&handle);
    
    audio_uninitialize(platform_audio_context);
    printf("done\n");
    return 0;
}