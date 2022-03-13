#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "windows.h"
#include "math.h"

#include "base.h"
#include "structs.h"
#include "win32_helpers.h"
#include "memory.h"
#include "plugin.h"

#include "hardcoded_values.h"
bool plugin_descriptor_compare(Plugin_Descriptor *a, Plugin_Descriptor *b)
{
    if(a->parameters_struct.size            != b->parameters_struct.size) return false;
    if(a->parameters_struct.alignment       != b->parameters_struct.alignment) return false;
    if(a->state_struct.size                 != b->state_struct.size) return false;
    if(a->state_struct.alignment            != b->state_struct.alignment) return false;
    if(a->num_parameters                    != b->num_parameters) return false;
    
    for(i32 param_idx = 0; param_idx < a->num_parameters; param_idx++)
    {
        Plugin_Descriptor_Parameter *param_a = &a->parameters[param_idx];
        Plugin_Descriptor_Parameter *param_b = &b->parameters[param_idx];
        
        if(param_a->offset                    != param_b->offset) return false;
        if(param_a->type                      != param_b->type) return false;
        if(param_a->name.size                 != param_b->name.size) return false;
        if(strncmp(param_a->name.str, param_a->name.str, param_a->name.size) != 0) return false;
        
        switch(param_a->type)
        {
            case Int: {
                if(param_a->int_param.min         != param_b->int_param.min) return false;
                if(param_a->int_param.max         != param_b->int_param.max) return false;
            } break;
            
            case Float: {
                if(param_a->float_param.min        != param_b->float_param.min) return false;
                if(param_a->float_param.max        != param_b->float_param.max) return false;
            } break;
            
            case Enum: {
                if(param_a->enum_param.num_entries != param_b->enum_param.num_entries) return false;
                for(u32 entry_idx = 0; entry_idx < param_a->enum_param.num_entries; entry_idx++)
                {
                    Parameter_Enum_Entry *entry_a = &param_a->enum_param.entries[entry_idx];
                    Parameter_Enum_Entry *entry_b = &param_b->enum_param.entries[entry_idx];
                    
                    if(entry_a->value               != entry_b->value) return false;
                    if(entry_a->name.size           != entry_b->name.size) return false;
                    if(strncmp(entry_a->name.str, entry_b->name.str, entry_a->name.size)  != 0) return false;
                }
            } break;
        }
    }
    return true;
}

Plugin_Parameters_Ring_Buffer plugin_parameters_ring_buffer_initialize(u32 num_fields_by_plugin, u32 buffer_slot_count)
{
    return {
        .buffer = (Plugin_Parameter_Value*)malloc(sizeof(Plugin_Parameter_Value) * buffer_slot_count * num_fields_by_plugin),
        .head = nullptr,
        .writer_idx = 0,
        .buffer_size = buffer_slot_count * num_fields_by_plugin,
        .num_fields_by_plugin = num_fields_by_plugin
    };
}

void plugin_set_parameter_values_from_holder(Plugin_Descriptor *descriptor,
                                             Plugin_Parameter_Value *parameter_values_out,
                                             char* holder)
{
    for(auto param_idx = 0; param_idx < descriptor->num_parameters; param_idx++)
    {
        auto param_descriptor = descriptor->parameters[param_idx];
        auto offset = param_descriptor.offset;
        switch(param_descriptor.type){
            case Int :
            {
                parameter_values_out[param_idx].int_value = *(int*)(holder + offset);
            }break;
            case Float : 
            {
                parameter_values_out[param_idx].float_value = *(float*)(holder+ offset);
            }break;
            case Enum : 
            {
                parameter_values_out[param_idx].enum_value = *(int*)(holder + offset);
            }break;
        }
    }
}


void plugin_set_parameter_holder_from_values(Plugin_Descriptor* descriptor, 
                                             Plugin_Parameter_Value* new_values,
                                             char* plugin_parameters_holder)
{
    for(auto param_idx = 0; param_idx < descriptor->num_parameters ; param_idx++)
    {
        auto& param_descriptor = descriptor->parameters[param_idx];
        auto offset = param_descriptor.offset;
        
        switch(param_descriptor.type){
            case Int :
            {
                *(int*)(plugin_parameters_holder + offset) = new_values[param_idx].int_value;
            }break;
            case Float : 
            {
                *(float*)(plugin_parameters_holder + offset) = new_values[param_idx].float_value;
            }break;
            case Enum : 
            {
                *(int*)(plugin_parameters_holder + offset) = new_values[param_idx].enum_value;
            }break;
        }
    }
}


//TODO, ça marche pas, on sait pas qui c'est
void plugin_parameters_buffer_push(Plugin_Parameters_Ring_Buffer& ring, Plugin_Parameter_Value *new_parameters)
{
    Plugin_Parameter_Value *pointer_to_push = &ring.buffer[ring.writer_idx];
    
    for(u32 i = 0; i < ring.num_fields_by_plugin; i++)
        pointer_to_push[i] = new_parameters[i];
    
    ring.writer_idx += ring.num_fields_by_plugin;
    if(ring.writer_idx == ring.buffer_size) 
        ring.writer_idx = 0;
    
    exchange_ptr((void**)&ring.head, pointer_to_push);
}

Plugin_Parameter_Value* plugin_parameters_buffer_pull(Plugin_Parameters_Ring_Buffer& ring)
{
    Plugin_Parameter_Value *maybe_plugin_array =  (Plugin_Parameter_Value*) exchange_ptr((void**)&ring.head, nullptr);
    
    return maybe_plugin_array;
}



void plugin_populate_from_descriptor(Plugin *handle, Audio_Parameters audio_parameters)
{
    handle->parameter_values_audio_side = m_allocate_array(Plugin_Parameter_Value, handle->descriptor.num_parameters);
    
    handle->parameter_values_ui_side = m_allocate_array(Plugin_Parameter_Value, handle->descriptor.num_parameters);
    
    handle->parameters_holder = (char*) m_allocate(handle->descriptor.parameters_struct.size);
    handle->state_holder = (char*) m_allocate(handle->descriptor.state_struct.size);
    
    handle->default_parameters_f(handle->parameters_holder);
    handle->initialize_state_f(handle->parameters_holder, 
                               handle->state_holder, 
                               audio_parameters.num_channels,
                               audio_parameters.sample_rate, 
                               nullptr);
    
    plugin_set_parameter_values_from_holder(&handle->descriptor, handle->parameter_values_ui_side, handle->parameters_holder);
    plugin_set_parameter_values_from_holder(&handle->descriptor, handle->parameter_values_audio_side, handle->parameters_holder);
    handle->ring = plugin_parameters_ring_buffer_initialize(handle->descriptor.num_parameters, RING_BUFFER_SLOT_COUNT);
}

#ifdef DEBUG
extern release_jit_t release_jit;
extern try_compile_t try_compile;
#endif
#ifdef RELEASE
void release_jit(Plugin *plugin);
void try_compile(const char* filename, void* clang_ctx_ptr, Plugin *plugin);
#endif

void plugin_reset_handle(Plugin *handle)
{
    m_safe_free(handle->parameter_values_audio_side);
    m_safe_free(handle->parameter_values_ui_side);
    m_safe_free(handle->ring.buffer);
    Compiler_Error *old_error_log_buffer = handle->error_log.errors;
    
    release_jit(handle);
    *handle = {};
    
    handle->error_log = {
        old_error_log_buffer,
        0,
        1024
    };
}

DWORD compiler_thread_proc(void *void_param)
{
    Compiler_Thread_Param *param = (Compiler_Thread_Param*)void_param;
    if(compare_exchange_32(param->stage, Asset_File_State_BACKGROUND_LOADING, Asset_File_State_STAGE_BACKGROUND_LOADING) )
    {
        
        try_compile(param->source_filename, param->clang_ctx, param->handle);
        octave_assert(compare_exchange_32(param->stage, Asset_File_State_STAGE_VALIDATION, Asset_File_State_BACKGROUND_LOADING));
    }
    else if(compare_exchange_32(param->stage, 
                                Asset_File_State_HOT_RELOAD_BACKGROUND_LOADING, 
                                Asset_File_State_HOT_RELOAD_STAGE_BACKGROUND_LOADING) )
    {
        
        try_compile(param->source_filename, param->clang_ctx, param->handle);
        octave_assert(compare_exchange_32(param->stage, Asset_File_State_HOT_RELOAD_STAGE_VALIDATION, Asset_File_State_HOT_RELOAD_BACKGROUND_LOADING));
    }
    else
    {
        octave_assert(false && "someone touched plugin_state while I wasn't watching");
    }
    return 1;
}



void plugin_loading_manager_init(Plugin_Loading_Manager *m, void *clang_ctx, char *source_filename, Asset_File_State *plugin_state)
{
    *m = {
        .handle_a = {
            .error_log = {
                m_allocate_array(Compiler_Error, 1024),
                0,
                1024
            }
        }, 
        
        .handle_b = {
            .error_log = {
                m_allocate_array(Compiler_Error, 1024),
                0,
                1024
            }
        },
        
        .clang_ctx = clang_ctx,
        .plugin_state = plugin_state,
        .source_filename = source_filename
    };
    
    m->current_handle = &m->handle_a;
    m->hot_reload_handle = &m->handle_b;
    
    if(m->source_filename[0] != 0)
    {
        m->compiler_thread_param = Compiler_Thread_Param {
            .source_filename = source_filename,
            .handle = m->current_handle,
            .clang_ctx = m->clang_ctx,
            .stage = m->plugin_state
        };
        *m->plugin_state = Asset_File_State_STAGE_BACKGROUND_LOADING;
        MemoryBarrier();
        
        m->compiler_thread_handle = 
            CreateThread(0, 0,
                         &compiler_thread_proc,
                         &m->compiler_thread_param,
                         0, 0);
    }
}



void plugin_loading_update(Plugin_Loading_Manager *m, Audio_Thread_Context *audio_context, Audio_Parameters audio_parameters, 
                           Plugin **handle_to_pull_ir_from)
{
    
    if(compare_exchange_32(m->plugin_state,
                           Asset_File_State_VALIDATING,
                           Asset_File_State_STAGE_VALIDATION))
    {
        
        if(m->current_handle->error.flag == Compiler_Success)
        {
            plugin_populate_from_descriptor(m->current_handle, audio_parameters);
            m->plugin_last_write_time = win32_get_last_write_time(m->source_filename);
            audio_context->plugin = m->current_handle;
            
            octave_assert(exchange_32(m->plugin_state, Asset_File_State_STAGE_USAGE)
                          == Asset_File_State_VALIDATING);
            
            *handle_to_pull_ir_from = m->current_handle;
            
        }
        else
        {
            printf("compilation failed, cleaning up\n");
            octave_assert(!m->current_handle->parameter_values_audio_side);
            octave_assert(!m->current_handle->parameter_values_ui_side);
            octave_assert(!m->current_handle->ring.buffer);
            
            m->plugin_last_write_time = win32_get_last_write_time(m->source_filename);
            audio_context->plugin = m->current_handle;
            
            printf("done cleaning up\n");
            octave_assert(exchange_32(m->plugin_state, Asset_File_State_FAILED)
                          == Asset_File_State_VALIDATING);
        }
    }
    else if(compare_exchange_32(m->plugin_state,
                                Asset_File_State_HOT_RELOAD_VALIDATING,
                                Asset_File_State_HOT_RELOAD_STAGE_VALIDATION)) 
    {
        printf("hot : validating\n");
        if(m->hot_reload_handle->error.flag == Compiler_Success)
        {
            printf("hot : compiler success\n");
            
            plugin_populate_from_descriptor(m->hot_reload_handle, audio_parameters);
            
            m->plugin_last_write_time = win32_get_last_write_time(m->source_filename);
            audio_context->hot_reload_plugin = m->hot_reload_handle;
            
            *handle_to_pull_ir_from = m->hot_reload_handle;
            
            octave_assert(compare_exchange_32(m->plugin_state,
                                              Asset_File_State_HOT_RELOAD_STAGE_SWAP,
                                              Asset_File_State_HOT_RELOAD_VALIDATING));
            
            printf("hot reload validation done\n");
        }
        else 
        {
            
            printf("hot : compilation failed\n");
            release_jit(m->hot_reload_handle);
            octave_assert(!m->hot_reload_handle->parameter_values_audio_side);
            octave_assert(!m->hot_reload_handle->parameter_values_ui_side);
            octave_assert(!m->hot_reload_handle->ring.buffer);
            
            printf("hot : done cleaning up, back in use\n");
            
            if(m->current_handle->descriptor.error.flag != Compiler_Success){
                octave_assert(compare_exchange_32(m->plugin_state,
                                                  Asset_File_State_FAILED,
                                                  Asset_File_State_HOT_RELOAD_VALIDATING));
            }
            else{
                octave_assert(compare_exchange_32(m->plugin_state,
                                                  Asset_File_State_STAGE_USAGE,
                                                  Asset_File_State_HOT_RELOAD_VALIDATING));
            }
        }
    }
    else if(compare_exchange_32(m->plugin_state,
                                Asset_File_State_UNLOADING,
                                Asset_File_State_COLD_RELOAD_STAGE_UNLOAD)) 
    {
        plugin_reset_handle(m->current_handle);
        
        m->compiler_thread_param = {
            .source_filename = m->source_filename,
            .handle = m->current_handle,
            .clang_ctx = m->clang_ctx,
            .stage = m->plugin_state
        };
        
        octave_assert(exchange_32(m->plugin_state,
                                  Asset_File_State_STAGE_BACKGROUND_LOADING)
                      == Asset_File_State_UNLOADING);
        
        m->compiler_thread_handle = 
            CreateThread(0, 0,
                         &compiler_thread_proc,
                         &m->compiler_thread_param,
                         0, 0);
    }
    else if(compare_exchange_32(m->plugin_state,
                                Asset_File_State_HOT_RELOAD_DISPOSING,
                                Asset_File_State_HOT_RELOAD_STAGE_DISPOSE))
    {
        printf("hot reload disposing\n");
        
        Plugin *temp = m->hot_reload_handle;
        m->hot_reload_handle = m->current_handle;
        m->current_handle = temp;
        
        plugin_reset_handle(m->hot_reload_handle);
        
        compare_exchange_32(m->plugin_state,
                            Asset_File_State_STAGE_USAGE,
                            Asset_File_State_HOT_RELOAD_DISPOSING);
        printf("hot : in use\n");
    }
    
}


void plugin_load_button_was_clicked(Plugin_Loading_Manager *m)
{
    //Auto old_wave_state = 
    exchange_32(m->plugin_state, Asset_File_State_COLD_RELOAD_STAGE_UNUSE);
    //TODO octave_assert, dans quels états peut être old_wav_state qui nous foutraient dans la merde ?
}


void plugin_loading_check_and_stage_hot_reload(Plugin_Loading_Manager *m)
{
    bool was_in_use = compare_exchange_32(m->plugin_state, 
                                          Asset_File_State_HOT_RELOAD_CHECK_FILE_FOR_UPDATE,
                                          Asset_File_State_IN_USE); 
    bool was_failed  = compare_exchange_32(m->plugin_state, 
                                           Asset_File_State_HOT_RELOAD_CHECK_FILE_FOR_UPDATE,
                                           Asset_File_State_FAILED);
    if(was_in_use || was_failed)
    {
        if(win32_query_file_change(m->source_filename, &m->plugin_last_write_time))
        {
            printf("hot : file has changed\n");
            octave_assert(!m->hot_reload_handle->parameter_values_audio_side);
            octave_assert(!m->hot_reload_handle->parameter_values_ui_side);
            octave_assert(!m->hot_reload_handle->ring.buffer);
            
            Compiler_Error *old_error_log_buffer = m->hot_reload_handle->error_log.errors;
            
            *m->hot_reload_handle = {};
            
            m->hot_reload_handle->error_log = {
                old_error_log_buffer,
                0,
                1024
            };
            printf("source : %s\n", m->source_filename);
            m->compiler_thread_param = {
                .source_filename = m->source_filename,
                .handle = m->hot_reload_handle,
                .clang_ctx = m->clang_ctx,
                .stage = m->plugin_state
            };
            
            octave_assert(compare_exchange_32(m->plugin_state, Asset_File_State_HOT_RELOAD_STAGE_BACKGROUND_LOADING,
                                              Asset_File_State_HOT_RELOAD_CHECK_FILE_FOR_UPDATE));
            printf("hot : file stage backround loading\n");
            m->compiler_thread_handle = CreateThread(0, 0,
                                                     &compiler_thread_proc,
                                                     &m->compiler_thread_param,
                                                     0, 0);
            
            
        }
        else if(was_in_use)
        {
            octave_assert(compare_exchange_32(m->plugin_state, 
                                              Asset_File_State_IN_USE,
                                              Asset_File_State_HOT_RELOAD_CHECK_FILE_FOR_UPDATE));
        }
        else if(was_failed)
        {
            octave_assert(compare_exchange_32(m->plugin_state, 
                                              Asset_File_State_FAILED,
                                              Asset_File_State_HOT_RELOAD_CHECK_FILE_FOR_UPDATE));
        }
    }
    
}