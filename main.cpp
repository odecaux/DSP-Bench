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
#include "draw.h"

#include "win32_platform.h"
#include "opengl.h"

#ifdef DEBUG
try_compile_t try_compile = nullptr;
release_jit_t release_jit = nullptr;
create_clang_context_t create_clang_context = nullptr;
release_clang_context_t release_clang_context = nullptr; 

#endif
#ifdef RELEASE
#include "compiler.h"
#endif

typedef struct {
    const char* source_filename;
    Plugin *handle;
    void *clang_ctx;
    
    Asset_File_State *stage;
} Compiler_Thread_Param;

DWORD compiler_thread_proc(void *void_param)
{
    Compiler_Thread_Param *param = (Compiler_Thread_Param*)void_param;
    assert(exchange_32(param->stage, Asset_File_State_BACKGROUND_LOADING)
           == Asset_File_State_STAGE_BACKGROUND_LOADING);
    
    try_compile(param->source_filename, param->clang_ctx, param->handle);
    assert(exchange_32(param->stage, Asset_File_State_STAGE_VALIDATION)
           == Asset_File_State_BACKGROUND_LOADING);
    return 1;
}

typedef struct {
    const char *filename;
    WavData *file;
    Asset_File_State *stage;
} Wav_Loader_Thread_Param;


//TODO should not be a global
char new_file_string[1024];

DWORD wav_loader_thread_proc(void *void_param)
{
    Wav_Loader_Thread_Param *param = (Wav_Loader_Thread_Param*)void_param;
    assert(exchange_32(param->stage, Asset_File_State_BACKGROUND_LOADING)
           == Asset_File_State_STAGE_BACKGROUND_LOADING);
    
    *param->file = windows_load_wav(param->filename);
    
    assert(exchange_32(param->stage, Asset_File_State_STAGE_VALIDATION)
           == Asset_File_State_BACKGROUND_LOADING);
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
    graphics_ctx.window_dim = { INITIAL_WINDOW_WIDTH, INITIAL_WINDOW_HEIGHT}; 
    
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
    Asset_File_State wav_state = Asset_File_State_NONE;
    Asset_File_State plugin_state = Asset_File_State_NONE;
    
    void* platform_audio_context;
    Audio_Thread_Context audio_context = {};
    audio_context.audio_file_play = 0;
    audio_context.audio_file_state = &wav_state;
    audio_context.plugin_state = &plugin_state;
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
    Wav_Loader_Thread_Param wav_thread_param = {
        .filename = audio_filename,
        .file = &wav_file,
        .stage = &wav_state
    };
    HANDLE wav_loader_thread_handle;
    
    if(audio_filename != nullptr)
    {
        wav_state = Asset_File_State_STAGE_BACKGROUND_LOADING;
        MemoryBarrier();
        wav_loader_thread_handle = CreateThread(0, 0,
                                                &wav_loader_thread_proc,
                                                &wav_thread_param,
                                                0, 0);
    }
    
    win32_print_elapsed(time_program_begin, "time to wav");
    
    //~ Compiler Init
#ifdef DEBUG
    HMODULE compiler_dll = LoadLibraryA("compiler.dll");
    assert(compiler_dll && "couldn't find compiler.dll");
    
    try_compile = (try_compile_t)GetProcAddress(compiler_dll, "try_compile");
    release_jit = (release_jit_t)GetProcAddress(compiler_dll, "release_jit");
    create_clang_context = (create_clang_context_t)GetProcAddress(compiler_dll, "create_clang_context");
    release_clang_context = (release_clang_context_t)GetProcAddress(compiler_dll, "release_clang_context");
    
#endif
    void* clang_ctx = create_clang_context();
    
    Plugin handle {
        .error_log = {
            m_allocate_array(Compiler_Error, 1024),
            0,
            1024
        }
    }; 
    
    Compiler_Thread_Param compiler_thread_param = {
        .source_filename = source_filename,
        .handle = &handle,
        .clang_ctx = clang_ctx,
        .stage = &plugin_state
    };
    
    plugin_state = Asset_File_State_STAGE_BACKGROUND_LOADING;
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
        .IR_sample_count = IR_BUFFER_LENGTH,
        .zoom_state = 1.0f
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
    
    ShowWindow(window.window, 1);
    //UpdateWindow(window.window);
    
    while(!done)
    {
        win32_message_dispatch(&window, &frame_io, &done);
        if(done) break;
        
        graphics_ctx.atlas.draw_vertices_count = 0;
        graphics_ctx.atlas.draw_indices_count = 0;
        
        frame_io = io_state_advance(frame_io);
        frame_io.mouse_position = win32_get_mouse_position(&window);
        
        
        if(compare_exchange_32(&wav_state,
                               Asset_File_State_VALIDATING,
                               Asset_File_State_STAGE_VALIDATION))
        {
            if(wav_file.error == Wav_Success)
            {
                /*
                audio_context.new_audio_file_buffer = wav_file.deinterleaved_buffer;
                audio_context.new_audio_file_length = wav_file.samples_by_channel;
                audio_context.new_audio_file_read_cursor = 0;
                audio_context.new_audio_file_num_channels = wav_file.num_channels;
                */
                assert(exchange_32(&wav_state, Asset_File_State_STAGE_USAGE)
                       == Asset_File_State_VALIDATING);
            }
            else
            {
                assert(exchange_32(&wav_state, Asset_File_State_FAILED)
                       == Asset_File_State_VALIDATING);
                //TODO y a pas à cleanup un buffer ici ?
            }
        }
        else if(compare_exchange_32(&wav_state,
                                    Asset_File_State_COLD_RELOAD_UNLOADING,
                                    Asset_File_State_COLD_RELOAD_STAGE_UNLOAD)) 
        {
            for(i32 channel = 0; channel < wav_file.num_channels; channel++)
                m_free(wav_file.deinterleaved_buffer[channel]);
            m_free(wav_file.deinterleaved_buffer);
            
            wav_thread_param = {
                .filename = new_file_string,
                .file = &wav_file,
                .stage = &wav_state
            };
            assert(exchange_32(&wav_state,
                               Asset_File_State_STAGE_BACKGROUND_LOADING)
                   == Asset_File_State_COLD_RELOAD_UNLOADING);
            
            wav_loader_thread_handle = CreateThread(0, 0,
                                                    &wav_loader_thread_proc,
                                                    &wav_thread_param,
                                                    0, 0);
            
        }
        
        if(compare_exchange_32(&plugin_state,
                               Asset_File_State_VALIDATING,
                               Asset_File_State_STAGE_VALIDATION))
        {
            win32_print_elapsed(time_program_begin, "time to compilation end");
            
            if(handle.error.flag == Compiler_Success)
            {
                handle.parameter_values_audio_side = m_allocate_array(Plugin_Parameter_Value, handle.descriptor.num_parameters);
                
                handle.parameter_values_ui_side = m_allocate_array(Plugin_Parameter_Value, handle.descriptor.num_parameters);
                
                handle.parameters_holder = (char*) malloc(handle.descriptor.parameters_struct.size);
                handle.state_holder = (char*) malloc(handle.descriptor.state_struct.size);
                
                handle.default_parameters_f(handle.parameters_holder);
                handle.initialize_state_f(handle.parameters_holder, 
                                          handle.state_holder, 
                                          audio_parameters.num_channels,
                                          audio_parameters.sample_rate, 
                                          nullptr);
                
                plugin_set_parameter_values_from_holder(&handle.descriptor, handle.parameter_values_ui_side, handle.parameters_holder);
                plugin_set_parameter_values_from_holder(&handle.descriptor, handle.parameter_values_audio_side, handle.parameters_holder);
                handle.ring = plugin_parameters_ring_buffer_initialize(handle.descriptor.num_parameters, RING_BUFFER_SLOT_COUNT);
                
                audio_context.plugin = &handle;
                
                assert(exchange_32(&plugin_state, Asset_File_State_STAGE_USAGE)
                       == Asset_File_State_VALIDATING);
                
                //~ IR initialization
                compute_IR(handle, fft.IR_buffer, IR_BUFFER_LENGTH, audio_parameters, handle.parameter_values_ui_side);
                
                fft_perform(&fft);
                
                memcpy(graphics_ctx.ir.IR_buffer, fft.IR_buffer[0], sizeof(real32) * IR_BUFFER_LENGTH); 
                memcpy(graphics_ctx.fft.fft_buffer, fft.magnitudes, sizeof(real32) * IR_BUFFER_LENGTH * 2); 
                
                win32_print_elapsed(time_program_begin, "time to loaded");
            }
            else
            {
                assert(exchange_32(&plugin_state, Asset_File_State_FAILED)
                       == Asset_File_State_VALIDATING);
            }
        }
        else if(compare_exchange_32(&plugin_state,
                                    Asset_File_State_UNLOADING,
                                    Asset_File_State_COLD_RELOAD_STAGE_UNLOAD)) 
        {
            
            m_safe_free(handle.parameter_values_audio_side);
            m_safe_free(handle.parameter_values_ui_side);
            m_safe_free(handle.ring.buffer);
            Compiler_Error *old_error_log_buffer = handle.error_log.errors;
            
            release_jit(&handle);
            handle = {};
            
            handle.error_log = {
                old_error_log_buffer,
                0,
                1024
            };
            
            compiler_thread_param = {
                .source_filename = new_file_string,
                .handle = &handle,
                .clang_ctx = clang_ctx,
                .stage = &plugin_state
            };
            
            assert(exchange_32(&plugin_state,
                               Asset_File_State_STAGE_BACKGROUND_LOADING)
                   == Asset_File_State_UNLOADING);
            
            compiler_thread_handle = 
                CreateThread(0, 0,
                             &compiler_thread_proc,
                             &compiler_thread_param,
                             0, 0);
            
            
        }
        
        
        bool parameters_were_tweaked = false;
        bool load_wav_was_clicked = false;
        bool load_plugin_was_clicked = false;
        frame(handle.descriptor, 
              &graphics_ctx, 
              ui_state, 
              frame_io, 
              handle.parameter_values_ui_side, 
              &audio_context, 
              &handle.error_log,
              &parameters_were_tweaked,
              &load_wav_was_clicked,
              &load_plugin_was_clicked
              );
        
        if(parameters_were_tweaked)
        {
            plugin_parameters_buffer_push(handle.ring, handle.parameter_values_ui_side);
            compute_IR(handle, fft.IR_buffer, 
                       IR_BUFFER_LENGTH, 
                       audio_parameters, 
                       handle.parameter_values_ui_side);
            fft_perform(&fft);
            
            memcpy(graphics_ctx.ir.IR_buffer, fft.IR_buffer[0], sizeof(real32) * IR_BUFFER_LENGTH); 
            memcpy(graphics_ctx.fft.fft_buffer, fft.magnitudes, sizeof(real32) * IR_BUFFER_LENGTH * 2); 
        }
        
        if(load_wav_was_clicked)
        {
            char filter[] = "Wav File\0*.wav\0";
            if(win32_open_file(new_file_string, sizeof(new_file_string), filter))
            {
                //Auto old_wave_state = 
                exchange_32(&wav_state, Asset_File_State_COLD_RELOAD_STAGE_UNUSE);
            }
            frame_io.mouse_down = false;
        }
        else if(load_plugin_was_clicked)
        {
            char filter[] = "C++ File\0*.cpp\0";
            if(win32_open_file(new_file_string, sizeof(new_file_string), filter))
            {
                //Auto old_wave_state = 
                exchange_32(&plugin_state, Asset_File_State_COLD_RELOAD_STAGE_UNUSE);
                //TODO assert, dans quels états peut être old_wav_state qui nous foutraient dans la merde ?
            }
            frame_io.mouse_down = false;
        }
        
        //TODO les id c'est un hack
        if(ui_state.selected_parameter_id != -1 && 
           ui_state.selected_parameter_id < 255 && 
           frame_io.mouse_clicked)
        {
            ShowCursor(FALSE);
        }
        else if(ui_state.previous_selected_parameter_id != -1 && frame_io.mouse_released && ui_state.previous_selected_parameter_id < 255)
        {
            ShowCursor(TRUE);
        }
        
        if(plugin_state == Asset_File_State_IN_USE)
            opengl_render_ui(&opengl_ctx, &graphics_ctx);
        else
            opengl_render_generic(&opengl_ctx, &graphics_ctx);
        
        i64 current_time;
        win32_get_elapsed_ms_since(last_time, &current_time, &frame_io.delta_time);
        last_time = current_time;
    }
    
    exit(1);
    
    opengl_uninitialize(&opengl_ctx);
    
    
    exchange_32(&plugin_state, Asset_File_State_STAGE_UNLOADING);
    
    do{
        Sleep(4);
    } while(!compare_exchange_32(&plugin_state,
                                 Asset_File_State_UNLOADING,
                                 Asset_File_State_OK_TO_UNLOAD));
    
    release_jit(&handle);
    
    audio_uninitialize(platform_audio_context);
    printf("done\n");
    return 0;
}