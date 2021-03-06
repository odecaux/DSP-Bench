#include "hardcoded_values.h"

#include "stdio.h"
#include "math.h"
#include "assert.h"
#include "windows.h"
#include "string.h"


#include "base.h"
#include "structs.h"
#include "memory.h"
#include "audio.h"
#include "dsp.h"
#include "plugin.h"
#include "win32_helpers.h"
#include "wav_reader.h"
#include "draw.h"
#include "font.h"
#include "app.h"

#include "win32_platform.h"
#include "opengl.h"


#ifdef DEBUG
typedef Plugin(*try_compile_t)(const char*, const void*, Arena *allocator, void *previous_clang_ctx);
typedef void(*release_jit_t)(Plugin*);
typedef void*(*create_llvm_context_t)();
typedef void(*release_llvm_context_t)(void* llvm_context_void);

try_compile_t try_compile = nullptr;
release_jit_t release_jit = nullptr;
create_llvm_context_t create_llvm_context = nullptr;
release_llvm_context_t release_llvm_context = nullptr; 

typedef void(*frame_t)(
                       Plugin_Descriptor&,
                       Graphics_Context *graphics_ctx, 
                       UI_State& ui_state, 
                       IO frame_io, 
                       Plugin_Parameter_Value* current_parameter_values, 
                       Audio_Thread_Context *audio_ctx, 
                       Compiler_Gui_Log *error_log,
                       Device_List *audio_device_list,
                       Analysis *analysis, 
                       bool *parameters_were_tweaked, 
                       bool *load_was_clicked, 
                       bool *load_plugin_was_clicked);

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
    ensure(exchange_32(param->stage, Asset_File_State_BACKGROUND_LOADING)
           == Asset_File_State_STAGE_BACKGROUND_LOADING);
    
    *param->file = windows_load_wav(param->filename);
    
    ensure(exchange_32(param->stage, Asset_File_State_STAGE_VALIDATION)
           == Asset_File_State_BACKGROUND_LOADING);
    return 1;
}


bool check_extension(const char* filename, const char* extension)
{
    const char *maybe_ext = filename + strlen(filename) - 3; 
    return maybe_ext > filename && (strncmp(maybe_ext, extension, 3) == 0);
}

i32 WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR  pCmdLine, int nCmdShow)
{
    
    auto argc = __argc;
    auto argv = __argv;
    
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
    
    Arena app_allocator = allocator_init(4 * 1024 * 1204);
    Arena scratch_allocator = allocator_init(1024 * 1204);
    
    //~ Graphics Init
    Graphics_Context graphics_ctx = {
        .window_dim = { INITIAL_WINDOW_WIDTH, INITIAL_WINDOW_HEIGHT},
        .font = load_fonts(DEFAULT_FONT_FILENAME, &app_allocator, &scratch_allocator),
        .command_list = {
            .draw_vertices = (Vertex*) arena_allocate(&app_allocator, sizeof(Vertex) * ATLAS_MAX_VERTEX_COUNT),
            .draw_vertices_count = 0,
            .draw_indices = (u32*) arena_allocate(&app_allocator, sizeof(u32) * ATLAS_MAX_VERTEX_COUNT), 
            .draw_indices_count = 0,
            .draw_commands = (Draw_Command*) arena_allocate(&app_allocator, sizeof(Draw_Command) * 1000), 
            .draw_command_count = 0,
        },
        .popup_command_list = {
            .draw_vertices = (Vertex*) arena_allocate(&app_allocator, sizeof(Vertex) * ATLAS_MAX_VERTEX_COUNT),
            .draw_vertices_count = 0,
            .draw_indices = (u32*) arena_allocate(&app_allocator, sizeof(u32) * ATLAS_MAX_VERTEX_COUNT), 
            .draw_indices_count = 0,
            .draw_commands = (Draw_Command*) arena_allocate(&app_allocator, sizeof(Draw_Command) * 1000), 
            .draw_command_count = 0,
        },
        .FIXME_zoom_state = 1.0f
    };
    
    IO frame_io = io_initial_state();
    Window_Backend_Context window;
    win32_init_window(&window, &graphics_ctx.window_dim, &frame_io);
    OpenGL_Context opengl_ctx = opengl_initialize(&window, &graphics_ctx.font);
    
    win32_print_elapsed(time_program_begin, "time to graphics");
    
    //~ Audio Thread Init
    
    Asset_File_State wav_state = Asset_File_State_NONE;
    Asset_File_State plugin_state = Asset_File_State_NONE;
    
    void* platform_audio_context;
    Audio_Thread_Context audio_context = {};
    audio_context.audio_file_play = 0;
    audio_context.audio_file_state = &wav_state;
    audio_context.plugin_state = &plugin_state;
    audio_context.m = nullptr;
    Audio_Format audio_format = {};
    MemoryBarrier();
    
    
    auto audio_device_list = get_device_list();
    
    Audio_Device *default_output = &audio_device_list.outputs[audio_device_list.default_output_idx];
    
    if(!audio_initialize(&platform_audio_context, &audio_format, &audio_context, &app_allocator, default_output))
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
    ensure(compiler_dll != NULL && "couldn't find compiler.dll");
    try_compile = (try_compile_t)GetProcAddress(compiler_dll, "try_compile");
    release_jit = (release_jit_t)GetProcAddress(compiler_dll, "release_jit");
    create_llvm_context = (create_llvm_context_t)GetProcAddress(compiler_dll, "create_llvm_context");
    release_llvm_context = (release_llvm_context_t)GetProcAddress(compiler_dll, "release_llvm_context");
    
#endif
    
    void* llvm_ctx = create_llvm_context();
    
    IPP_FFT_Context audio_side_ipp_context = ipp_initialize(&app_allocator);
    Plugin_Reloading_Manager plugin_reloading_manager;
    plugin_reloading_manager_init(&plugin_reloading_manager, llvm_ctx, source_filename, &plugin_state, &audio_side_ipp_context);
    
    audio_context.m = &plugin_reloading_manager;
    
    if(source_filename[0] != 0)
        plugin_reloader_stage_cold_compilation(&plugin_reloading_manager);
    
    //~ IR/FFT
    
    Arena gui_IR_allocator = allocator_init(100 * 1204);
    IPP_FFT_Context gui_ipp_context = ipp_initialize(&app_allocator);
    Initializer gui_initializer = { &gui_IR_allocator, &gui_ipp_context};
    Analysis analysis = analysis_initialize(IR_BUFFER_LENGTH, audio_format.num_channels, &app_allocator, &gui_ipp_context);
    
    //~ UI init
#ifdef DEBUG
    ensure(CopyFile("gui.dll", "gui_temp.dll", FALSE) != 0);
    HMODULE gui_dll = LoadLibraryA("gui_temp.dll");
    ensure(gui_dll != NULL && "couldn't find gui.dll");
    frame = (frame_t)GetProcAddress(gui_dll, "frame");
    ensure(frame != 0);
    u64 gui_dll_last_write_time = win32_get_last_write_time("gui.dll");
#endif
    
    UI_State ui_state = {-1, -1, Panel_Plugin};
    i64 last_time = win32_get_time();
    bool done = false;
    
    win32_print_elapsed(time_program_begin, "time to loop");
    
    ShowWindow(window.window, 1);
    //UpdateWindow(window.window);
    
    while(!done)
    {
        win32_message_dispatch(&window, &done);
        if(done) break;
        
        frame_io = io_state_advance(frame_io);
        frame_io.mouse_position = win32_get_mouse_position(&window);
        
        
        if(compare_exchange_32(&wav_state,
                               Asset_File_State_VALIDATING,
                               Asset_File_State_STAGE_VALIDATION))
        {
            if(wav_file.error == Wav_Success)
            {
                ensure(exchange_32(&wav_state, Asset_File_State_STAGE_USAGE)
                       == Asset_File_State_VALIDATING);
            }
            else
            {
                ensure(exchange_32(&wav_state, Asset_File_State_FAILED)
                       == Asset_File_State_VALIDATING);
                //TODO y a pas ?? cleanup un buffer ici ?
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
            ensure(exchange_32(&wav_state,
                               Asset_File_State_STAGE_BACKGROUND_LOADING)
                   == Asset_File_State_COLD_RELOAD_UNLOADING);
            
            wav_loader_thread_handle = CreateThread(0, 0,
                                                    &wav_loader_thread_proc,
                                                    &wav_thread_param,
                                                    0, 0);
        }
        
        Plugin *plugin_to_pull_ir_from = nullptr;
        plugin_reloading_update_gui_side(&plugin_reloading_manager, &audio_context, audio_format, &plugin_to_pull_ir_from);
        
        if(plugin_to_pull_ir_from)
        {
            gui_IR_allocator.current = gui_IR_allocator.base;
            
            compute_IR(*plugin_to_pull_ir_from, 
                       analysis.IR_buffer, 
                       IR_BUFFER_LENGTH, 
                       audio_format, 
                       plugin_to_pull_ir_from->parameter_values_ui_side, 
                       &scratch_allocator, 
                       &gui_initializer);
            
            fft_perform_and_get_magnitude(&analysis);
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
              &audio_device_list,
              &analysis,
              &parameters_were_tweaked,
              &load_wav_was_clicked,
              &load_plugin_was_clicked );
        
        ui_state.previous_selected_parameter_id = ui_state.selected_parameter_id;
        
        if(parameters_were_tweaked)
        {
            plugin_parameters_push_to_ring(plugin_reloading_manager.front_handle->ring, plugin_reloading_manager.front_handle->parameter_values_ui_side);
            
            gui_IR_allocator.current = gui_IR_allocator.base;
            compute_IR(*plugin_reloading_manager.front_handle, analysis.IR_buffer, 
                       IR_BUFFER_LENGTH, 
                       audio_format, 
                       plugin_reloading_manager.front_handle->parameter_values_ui_side, 
                       &scratch_allocator,
                       &gui_initializer);
            
            fft_perform_and_get_magnitude(&analysis);
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
                    ensure(compare_exchange_32(&wav_state, 
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
                    ensure(compare_exchange_32(&wav_state,
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
        
        opengl_render(&opengl_ctx, &graphics_ctx);
        
        i64 current_time;
        win32_get_elapsed_ms_since(last_time, &current_time, &frame_io.delta_time);
        last_time = current_time;
        
#ifdef DEBUG
        {
            u64 temp_gui_write_time = gui_dll_last_write_time;
            if(win32_query_file_change("gui.dll", &temp_gui_write_time))
            {
                printf("reload gui\n");
                FreeLibrary(gui_dll);
                if(CopyFile("gui.dll", "gui_temp.dll", FALSE) == 0)
                    win32_exit_on_error_code();
                HMODULE gui_dll = LoadLibraryA("gui_temp.dll");
                ensure(gui_dll != NULL);
                frame = (frame_t)GetProcAddress(gui_dll, "frame");
                ensure(frame != nullptr);
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