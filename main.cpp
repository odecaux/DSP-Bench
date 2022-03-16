#include "hardcoded_values.h"

#include "stdio.h"
#include "math.h"
#include "assert.h"
#include "windows.h"
#include "string.h"


#include "memory.h"
#include "base.h"
#include "structs.h"
#include "plugin.h"
#include "audio.h"
#include "win32_helpers.h"
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
frame_t frame = nullptr;
#endif
#ifdef RELEASE
#include "compiler.h"
#include "gui.h"
#endif

typedef struct {
    const char *filename;
    Audio_File *file;
    Asset_File_State *stage;
} Wav_Loader_Thread_Param;

DWORD wav_loader_thread_proc(void *void_param)
{
    Wav_Loader_Thread_Param *param = (Wav_Loader_Thread_Param*)void_param;
    octave_assert(exchange_32(param->stage, Asset_File_State_BACKGROUND_LOADING)
                  == Asset_File_State_STAGE_BACKGROUND_LOADING);
    
    *param->file = windows_load_wav(param->filename);
    
    octave_assert(exchange_32(param->stage, Asset_File_State_STAGE_VALIDATION)
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
    char source_filename[1024] = {0};
    char audio_filename[1024] = {0};
    
    i64 time_program_begin = win32_get_time();
    
    if(argc > 3)
    {
        printf("too many arguments\n");
        return -1;
    }
    else if(argc == 2)
    {
        if(check_extension(argv[1], "cpp"))
        {
            strncpy(source_filename, argv[1], sizeof(source_filename)/sizeof(*source_filename));
        }
        else 
        {
            printf("not a cpp file\n");
            return -1;
        }
    }
    else if(argc == 3)
    {
        if(check_extension(argv[1], "cpp") && check_extension(argv[2], "wav"))
        {
            strncpy(source_filename, argv[1], sizeof(source_filename)/sizeof(*source_filename));
            strncpy(audio_filename, argv[2], sizeof(audio_filename)/sizeof(*audio_filename));
            
        }
        else if(check_extension(argv[1], "wav") && check_extension(argv[2], "cpp"))
        {
            strncpy(audio_filename, argv[1], sizeof(audio_filename)/sizeof(*audio_filename));
            strncpy(source_filename, argv[2], sizeof(source_filename)/sizeof(*source_filename));
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
    Graphics_Context graphics_ctx = {};
    graphics_ctx.window_dim = { INITIAL_WINDOW_WIDTH, INITIAL_WINDOW_HEIGHT}; 
    
    Window_Context window = win32_init_window(&graphics_ctx.window_dim);
    
    graphics_ctx.atlas = {
        .font = load_fonts(DEFAULT_FONT_FILENAME),
        .draw_vertices = m_allocate_array(Vertex, ATLAS_MAX_VERTEX_COUNT, "graphics : draw vertices"),
        .draw_vertices_count = 0,
        .draw_indices = m_allocate_array(u32, ATLAS_MAX_VERTEX_COUNT, "graphics : draw indices"), 
        .draw_indices_count = 0
    };
    
    OpenGL_Context opengl_ctx = opengl_initialize(&window, &graphics_ctx.atlas.font);
    
    win32_print_elapsed(time_program_begin, "time to graphics");
    
    //~ Audio Thread Init
    
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
    
    Audio_File wav_file;
    Wav_Loader_Thread_Param wav_thread_param;
    HANDLE wav_loader_thread_handle;
    
    audio_context.audio_file = &wav_file;
    if(audio_filename[0] != 0)
    {
        wav_thread_param = {
            .filename = audio_filename,
            .file = &wav_file,
            .stage = &wav_state
        };
        wav_state = Asset_File_State_STAGE_BACKGROUND_LOADING;
        MemoryBarrier();
        wav_loader_thread_handle = CreateThread(0, 0,
                                                &wav_loader_thread_proc,
                                                &wav_thread_param,
                                                0, 0);
        win32_print_elapsed(time_program_begin, "time to wav");
    }
    
    
    //~ Compiler Init
#ifdef DEBUG
    HMODULE compiler_dll = LoadLibraryA("compiler.dll");
    octave_assert(compiler_dll != NULL && "couldn't find compiler.dll");
    try_compile = (try_compile_t)GetProcAddress(compiler_dll, "try_compile");
    release_jit = (release_jit_t)GetProcAddress(compiler_dll, "release_jit");
    create_clang_context = (create_clang_context_t)GetProcAddress(compiler_dll, "create_clang_context");
    release_clang_context = (release_clang_context_t)GetProcAddress(compiler_dll, "release_clang_context");
    
#endif
    
    void* clang_ctx = create_clang_context();
    
    Plugin_Reloading_Manager plugin_reloading_manager;
    plugin_reloading_manager_init(&plugin_reloading_manager, clang_ctx, source_filename, &plugin_state);
    
    if(source_filename[0] != 0)
        plugin_reloader_stage_cold_compilation(&plugin_reloading_manager);
    //~ IR/FFT
    
    graphics_ctx.ir = {
        .IR_buffer = m_allocate_array(real32, IR_BUFFER_LENGTH, "graphics : ir buffer"),
        .IR_sample_count = IR_BUFFER_LENGTH,
        .zoom_state = 1.0f
    };
    graphics_ctx.fft = {
        .fft_buffer = m_allocate_array(real32, IR_BUFFER_LENGTH * 2, "graphics : fft buffer"),
        .fft_sample_count = IR_BUFFER_LENGTH * 2
    };
    
    FFT fft = fft_initialize(IR_BUFFER_LENGTH, audio_parameters.num_channels);
    
    //~ UI init
#ifdef DEBUG
    octave_assert(CopyFile("gui.dll", "gui_temp.dll", FALSE) != 0);
    HMODULE gui_dll = LoadLibraryA("gui_temp.dll");
    octave_assert(gui_dll != NULL && "couldn't find gui.dll");
    frame = (frame_t)GetProcAddress(gui_dll, "frame");
    octave_assert(frame != 0);
    u64 gui_dll_last_write_time = win32_get_last_write_time("gui.dll");
#endif
    
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
                octave_assert(exchange_32(&wav_state, Asset_File_State_STAGE_USAGE)
                              == Asset_File_State_VALIDATING);
            }
            else
            {
                octave_assert(exchange_32(&wav_state, Asset_File_State_FAILED)
                              == Asset_File_State_VALIDATING);
                //TODO y a pas à cleanup un buffer ici ?
            }
        }
        else if(compare_exchange_32(&wav_state,
                                    Asset_File_State_COLD_RELOAD_UNLOADING,
                                    Asset_File_State_COLD_RELOAD_STAGE_UNLOAD)) 
        {
            for(i32 channel = 0; channel < wav_file.num_channels; channel++)
                m_free(wav_file.deinterleaved_buffer[channel], "wav : deinterleaved buffer");
            m_free(wav_file.deinterleaved_buffer, "wav : deinterleaved buffers[]");
            
            wav_thread_param = {
                .filename = audio_filename,
                .file = &wav_file,
                .stage = &wav_state
            };
            octave_assert(exchange_32(&wav_state,
                                      Asset_File_State_STAGE_BACKGROUND_LOADING)
                          == Asset_File_State_COLD_RELOAD_UNLOADING);
            
            wav_loader_thread_handle = CreateThread(0, 0,
                                                    &wav_loader_thread_proc,
                                                    &wav_thread_param,
                                                    0, 0);
        }
        
        Plugin *plugin_to_pull_ir_from = nullptr;
        plugin_reloading_update(&plugin_reloading_manager, &audio_context, audio_parameters, &plugin_to_pull_ir_from);
        
        if(plugin_to_pull_ir_from)
        {
            compute_IR(*plugin_to_pull_ir_from, fft.IR_buffer, IR_BUFFER_LENGTH, audio_parameters, plugin_to_pull_ir_from->parameter_values_ui_side);
            fft_perform(&fft);
            
            memcpy(graphics_ctx.ir.IR_buffer, fft.IR_buffer[0], sizeof(real32) * IR_BUFFER_LENGTH); 
            memcpy(graphics_ctx.fft.fft_buffer, fft.magnitudes, sizeof(real32) * IR_BUFFER_LENGTH * 2); 
        }
        
        bool parameters_were_tweaked = false;
        bool load_wav_was_clicked = false;
        bool load_plugin_was_clicked = false;
        frame(plugin_reloading_manager.front_handle->descriptor, 
              &graphics_ctx, 
              ui_state, 
              frame_io, 
              plugin_reloading_manager.front_handle->parameter_values_ui_side, 
              &audio_context, 
              &plugin_reloading_manager.gui_log,
              &parameters_were_tweaked,
              &load_wav_was_clicked,
              &load_plugin_was_clicked
              );
        
        if(parameters_were_tweaked)
        {
            plugin_parameters_push_to_ring(plugin_reloading_manager.front_handle->ring, plugin_reloading_manager.front_handle->parameter_values_ui_side);
            compute_IR(*plugin_reloading_manager.front_handle, fft.IR_buffer, 
                       IR_BUFFER_LENGTH, 
                       audio_parameters, 
                       plugin_reloading_manager.front_handle->parameter_values_ui_side);
            fft_perform(&fft);
            
            memcpy(graphics_ctx.ir.IR_buffer, fft.IR_buffer[0], sizeof(real32) * IR_BUFFER_LENGTH); 
            memcpy(graphics_ctx.fft.fft_buffer, fft.magnitudes, sizeof(real32) * IR_BUFFER_LENGTH * 2); 
        }
        
        if(load_wav_was_clicked)
        {
            char filter[] = "Wav File\0*.wav\0";
            if(win32_open_file(audio_filename, sizeof(audio_filename), filter))
            {
                MemoryBarrier();
                auto old_wave_state = wav_state;
                MemoryBarrier();
                if(old_wave_state == Asset_File_State_IN_USE)
                {
                    octave_assert(compare_exchange_32(&wav_state, 
                                                      Asset_File_State_COLD_RELOAD_STAGE_UNUSE,
                                                      Asset_File_State_IN_USE));
                }
                else if(old_wave_state == Asset_File_State_NONE)
                {
                    wav_thread_param = {
                        .filename = audio_filename,
                        .file = &wav_file,
                        .stage = &wav_state
                    };
                    octave_assert(compare_exchange_32(&wav_state,
                                                      Asset_File_State_STAGE_BACKGROUND_LOADING, 
                                                      Asset_File_State_NONE));
                    
                    wav_loader_thread_handle = CreateThread(0, 0,
                                                            &wav_loader_thread_proc,
                                                            &wav_thread_param,
                                                            0, 0);
                }
                else 
                {
                    assert(false && "button should only be clicked when wav is in use or none\n");
                }
            }
            frame_io.mouse_down = false;
        }
        else if(load_plugin_was_clicked)
        {
            char filter[] = "C++ File\0*.cpp\0";
            if(win32_open_file(source_filename, sizeof(source_filename), filter))
            {
                plugin_load_button_was_clicked(&plugin_reloading_manager);
            }
            frame_io.mouse_down = false;
        }
        else {
            plugin_check_for_save_and_stage_hot_reload(&plugin_reloading_manager);
        }
        
        //TODO les id c'est un hack
        if(ui_state.selected_parameter_id != -1 && 
           ui_state.selected_parameter_id < 255 && 
           frame_io.mouse_clicked)
            ShowCursor(FALSE);
        else if(ui_state.previous_selected_parameter_id != -1 && frame_io.mouse_released && ui_state.previous_selected_parameter_id < 255)
            ShowCursor(TRUE);
        
        
        if(plugin_state == Asset_File_State_IN_USE)
            opengl_render_ui(&opengl_ctx, &graphics_ctx);
        else
            opengl_render_generic(&opengl_ctx, &graphics_ctx);
        
        i64 current_time;
        win32_get_elapsed_ms_since(last_time, &current_time, &frame_io.delta_time);
        last_time = current_time;
        
#ifdef DEBUG
        {
            u64 temp_gui_write_time = gui_dll_last_write_time;
            if(win32_query_file_change("gui.dll", &temp_gui_write_time))
            {
                FreeLibrary(gui_dll);
                HMODULE gui_dll = LoadLibraryA("gui_temp.dll");
                octave_assert(gui_dll != NULL);
                frame = (frame_t)GetProcAddress(gui_dll, "frame");
                octave_assert(frame != nullptr);
                gui_dll_last_write_time = temp_gui_write_time;
            }
        }
#endif
    }
    
    opengl_uninitialize(&opengl_ctx);
    
    return 0;
    exchange_32(&plugin_state, Asset_File_State_STAGE_UNLOADING);
    
    do{
        Sleep(4);
    } while(!compare_exchange_32(&plugin_state,
                                 Asset_File_State_UNLOADING,
                                 Asset_File_State_OK_TO_UNLOAD));
    
    release_jit(plugin_reloading_manager.front_handle);
    release_jit(plugin_reloading_manager.back_handle);
    
    audio_uninitialize(platform_audio_context);
    printf("done\n");
    return 0;
}