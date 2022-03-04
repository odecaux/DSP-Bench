#include "hardcoded_values.h"

#include "stdio.h"
#include "math.h"
#include "assert.h"
#include "windows.h"
#include "string.h"


#include "memory.h"
#include "base.h"
#include "structs.h"
#include "descriptor.h"
#include "win32_helpers.h"
#include "audio.h"
#include "wav_reader.h"
#include "font.h"
#include "fft.h"
#include "app.h"

#include "win32_platform.h"
#include "opengl.h"


typedef struct {
    const char* source_filename;
    Plugin_Handle *handle;
    Compiler_Errors *errors;
    void *clang_ctx;
    try_compile_t try_compile_f;
    
    bool *compilation_is_over;
} Compiler_Thread_Param;

DWORD compiler_thread_proc(void *void_param)
{
    Compiler_Thread_Param *param = (Compiler_Thread_Param*)void_param;
    *param->handle = param->try_compile_f(param->source_filename, param->clang_ctx, param->errors);
    MemoryBarrier();
    *param->compilation_is_over = true;
    return 1;
}

typedef struct {
    const char *filename;
    WavData *file;
    bool *loading_is_over;
} Wav_Loader_Thread_Param;

DWORD wav_loader_thread_proc(void *void_param)
{
    Wav_Loader_Thread_Param *param = (Wav_Loader_Thread_Param*)void_param;
    
    *param->file = windows_load_wav(param->filename);
    MemoryBarrier();
    *param->loading_is_over = true;
    return 1;
}


bool check_extension(const char* filename, const char* extension)
{
    const char *maybe_ext = filename + strlen(filename) - 3; 
    return maybe_ext > filename && (strncmp(maybe_ext, extension, 3) == 0);
}

i32 main(i32 argc, char** argv)
{
    
    i64 time_program_begin = win32_get_time();
    
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
    
    //~ Graphics Init
    
    CoInitializeEx(NULL, COINIT_MULTITHREADED); 
    
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
    
    
    win32_print_elapsed(time_program_begin, "time to graphics");
    
    //~ Audio Init
    
    void* platform_audio_context;
    Audio_Context audio_context = {};
    audio_context.audio_file_play = 1;
    Audio_Parameters audio_parameters = {};
    MemoryBarrier();
    
    if(!audio_initialize(&platform_audio_context, &audio_parameters, &audio_context))
    {
        printf("failed to initialize audio\n");
        return -1;
    }
    
    win32_print_elapsed(time_program_begin, "time to audio");
    
    //~ Wav Init
    
    WavData wav_file;
    bool wav_loading_is_over = false;
    bool wav_is_already_loaded = false;
    Wav_Loader_Thread_Param wav_thread_param = {
        .filename = audio_filename,
        .file = &wav_file,
        .loading_is_over = &wav_loading_is_over
    };
    HANDLE wav_loader_thread_handle;
    
    if(audio_filename != nullptr)
    {
        wav_loader_thread_handle = CreateThread(0, 0,
                                                &wav_loader_thread_proc,
                                                &wav_thread_param,
                                                0, 0);
    }
    
    win32_print_elapsed(time_program_begin, "time to wav");
    
    //~ Compiler Init
    
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
    
    bool compilation_is_over = false;
    bool plugin_is_loaded = false;
    Plugin_Handle handle;
    
    Plugin_Parameter_Value *parameter_values_audio_side;
    Plugin_Parameter_Value *parameter_values_ui_side;
    Plugin_Parameters_Ring_Buffer ring;
    
    Compiler_Thread_Param compiler_thread_param = {
        .source_filename = source_filename,
        .handle = &handle,
        .errors = &errors,
        .clang_ctx = clang_ctx,
        .try_compile_f = try_compile_f,
        .compilation_is_over = &compilation_is_over
    };
    MemoryBarrier();
    
    HANDLE compiler_thread_handle = 
        CreateThread(0, 0,
                     &compiler_thread_proc,
                     &compiler_thread_param,
                     0, 0);
    
    
    win32_print_elapsed(time_program_begin, "time to thread start");
    
    //~ IR/FFT
    
    graphics_ctx.ir = {
        .IR_buffer = m_allocate_array(real32, IR_BUFFER_LENGTH),
        .IR_sample_count = IR_BUFFER_LENGTH
    };
    graphics_ctx.fft = {
        .fft_buffer = m_allocate_array(real32, IR_BUFFER_LENGTH * 2),
        .fft_sample_count = IR_BUFFER_LENGTH * 2
    };
    
    FFT fft = fft_initialize(IR_BUFFER_LENGTH, audio_parameters.num_channels);
    
    IO frame_io = io_initial_state();
    UI_State ui_state = {-1};
    i64 last_time = win32_get_time();
    bool done = false;
    
    win32_print_elapsed(time_program_begin, "time to loop");
    while(!done)
    {
        win32_message_dispatch(&window, &frame_io, &done);
        if(done) break;
        
        graphics_ctx.atlas.draw_vertices_count = 0;
        graphics_ctx.atlas.draw_indices_count = 0;
        
        frame_io = io_state_advance(frame_io);
        frame_io.mouse_position = win32_get_mouse_position(&window);
        
        MemoryBarrier(); //TODO ???
        
        if(wav_loading_is_over && !wav_is_already_loaded)
        {
            if(wav_file.error == Wav_Success)
            {
                audio_context.audio_file_buffer = wav_file.deinterleaved_buffer;
                audio_context.audio_file_length = wav_file.samples_by_channel;
                audio_context.audio_file_read_cursor = 0;
                audio_context.audio_file_num_channels = wav_file.num_channels;
                MemoryBarrier();
                audio_context.audio_file_valid = 1;
            }
            wav_is_already_loaded = true;
        }
        
        if(compilation_is_over && !plugin_is_loaded)
        {
            win32_print_elapsed(time_program_begin, "time to compilation end");
            
            parameter_values_audio_side = m_allocate_array(Plugin_Parameter_Value, handle.descriptor.num_parameters);
            
            parameter_values_ui_side = m_allocate_array(Plugin_Parameter_Value, handle.descriptor.num_parameters);
            
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
            ring = plugin_parameters_ring_buffer_initialize(handle.descriptor.num_parameters, RING_BUFFER_SLOT_COUNT);
            
            audio_context.ring = &ring;
            audio_context.audio_callback_f = handle.audio_callback_f;
            audio_context.plugin_state_holder = plugin_state_holder;
            audio_context.plugin_parameters_holder = plugin_parameters_holder;
            audio_context.parameter_values_audio_side = parameter_values_audio_side;
            audio_context.descriptor = &handle.descriptor;
            InterlockedExchange8(&audio_context.plugin_valid, 1);
            
            //~ IR initialization
            
            compute_IR(handle, fft.IR_buffer, IR_BUFFER_LENGTH, audio_parameters, parameter_values_ui_side);
            
            fft_perform(&fft);
            
            memcpy(graphics_ctx.ir.IR_buffer, fft.IR_buffer[0], sizeof(real32) * IR_BUFFER_LENGTH); 
            memcpy(graphics_ctx.fft.fft_buffer, fft.magnitudes, sizeof(real32) * IR_BUFFER_LENGTH * 2); 
            
            plugin_is_loaded = true;
            
            win32_print_elapsed(time_program_begin, "time to loaded");
        }
        else if(plugin_is_loaded)
        {
            
            bool parameters_were_tweaked = false;
            
            frame(handle.descriptor, 
                  &graphics_ctx, 
                  ui_state, 
                  frame_io, 
                  parameter_values_ui_side, 
                  &audio_context, 
                  parameters_were_tweaked);
            
            if(parameters_were_tweaked)
            {
                plugin_parameters_buffer_push(ring, parameter_values_ui_side);
                compute_IR(handle, fft.IR_buffer, 
                           IR_BUFFER_LENGTH, 
                           audio_parameters, 
                           parameter_values_ui_side);
                fft_perform(&fft);
                
                memcpy(graphics_ctx.ir.IR_buffer, fft.IR_buffer[0], sizeof(real32) * IR_BUFFER_LENGTH); 
                memcpy(graphics_ctx.fft.fft_buffer, fft.magnitudes, sizeof(real32) * IR_BUFFER_LENGTH * 2); 
            }
        }
        
        //TODO hack
        if(ui_state.selected_parameter_id != -1 && ui_state.selected_parameter_id < 255 && frame_io.mouse_clicked)
        {
            ShowCursor(FALSE);
        }
        else if(ui_state.previous_selected_parameter_id != -1 && frame_io.mouse_released && ui_state.previous_selected_parameter_id < 255)
        {
            ShowCursor(TRUE);
        }
        
        opengl_render_ui(&opengl_ctx, &graphics_ctx);
        i64 current_time;
        win32_get_elapsed_ms_since(last_time, &current_time, &frame_io.delta_time);
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