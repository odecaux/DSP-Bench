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

//Plugin_Parameters_Ring_Buffer plugin_parameters_ring_buffer_initialize(u32 num_fields_by_plugin, u32 buffer_slot_count);


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

Plugin_Parameters_Ring_Buffer plugin_parameters_ring_buffer_initialize(u32 num_fields_by_plugin, u32 buffer_slot_count,
                                                                       Plugin_Allocator *allocator)
{
    return {
        .buffer = (Plugin_Parameter_Value*)plugin_allocate(allocator, sizeof(Plugin_Parameter_Value) * buffer_slot_count * num_fields_by_plugin),
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



#ifdef DEBUG
extern release_jit_t release_jit;
extern try_compile_t try_compile;
#endif
#ifdef RELEASE
void release_jit(Plugin *plugin);
void try_compile(const char* filename, void* clang_ctx_ptr, Plugin *plugin, Plugin_Allocator *allocator);
#endif

void plugin_reset_handle(Plugin *handle)
{
    handle->parameter_values_audio_side = nullptr;
    handle->parameter_values_ui_side = nullptr;
    handle->ring.buffer = nullptr;
    
    release_jit(handle);
    *handle = {};
}

DWORD compiler_thread_proc(void *void_param)
{
    Compiler_Thread_Param *param = (Compiler_Thread_Param*)void_param;
    if(compare_exchange_32(param->stage, Asset_File_State_BACKGROUND_LOADING, Asset_File_State_STAGE_BACKGROUND_LOADING) )
    {
        
        try_compile(param->source_filename, param->clang_ctx, param->handle, param->allocator);
        octave_assert(compare_exchange_32(param->stage, Asset_File_State_STAGE_VALIDATION, Asset_File_State_BACKGROUND_LOADING));
    }
    else if(compare_exchange_32(param->stage, 
                                Asset_File_State_HOT_RELOAD_BACKGROUND_LOADING, 
                                Asset_File_State_HOT_RELOAD_STAGE_BACKGROUND_LOADING) )
    {
        
        try_compile(param->source_filename, param->clang_ctx, param->handle, param->allocator);
        octave_assert(compare_exchange_32(param->stage, Asset_File_State_HOT_RELOAD_STAGE_VALIDATION, Asset_File_State_HOT_RELOAD_BACKGROUND_LOADING));
    }
    else
    {
        octave_assert(false && "someone touched plugin_state while I wasn't watching");
    }
    return 1;
}



Plugin_Allocator plugin_allocator_init(u64 size)
{
    Plugin_Allocator allocator = {
        .base = (char*)m_allocate(size),
        .current = allocator.base,
        .capacity = size,
    };
    return allocator;
}

void plugin_loading_manager_init(Plugin_Loading_Manager *m, void *clang_ctx, char *source_filename, Asset_File_State *plugin_state)
{
    *m = {
        .allocator_a = plugin_allocator_init(1024 * 1024),
        .allocator_b = plugin_allocator_init(1024 * 1204),
        
        .current_handle = &m->handle_a,
        .hot_reload_handle = &m->handle_b,
        .current_allocator = &m->allocator_a,
        .hot_reload_allocator = &m->allocator_b,
        
        .clang_ctx = clang_ctx,
        .plugin_state = plugin_state,
        .source_filename = source_filename,
        
        .gui_log = {
            .messages = m_allocate_array(Compiler_Gui_Message, 256),
            .message_count = 0,
            .message_capacity = 256,
            
            .holder_base = m_allocate_array(char, 1024*16),
            .holder_current = m->gui_log.holder_base,
            .holder_capacity = 1024*16
        }
    };
    
    if(m->source_filename[0] != 0)
    {
        m->compiler_thread_param = Compiler_Thread_Param {
            .source_filename = source_filename,
            .handle = m->current_handle,
            .allocator = m->current_allocator,
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


void plugin_populate_from_descriptor(Plugin *handle, Plugin_Allocator *allocator, Audio_Parameters audio_parameters)
{
    handle->parameter_values_audio_side = (Plugin_Parameter_Value*)plugin_allocate(allocator, sizeof(Plugin_Parameter_Value) *  handle->descriptor.num_parameters);
    
    handle->parameter_values_ui_side = (Plugin_Parameter_Value*)plugin_allocate(allocator, sizeof(Plugin_Parameter_Value) *  handle->descriptor.num_parameters);
    
    handle->parameters_holder = (char*) plugin_allocate(allocator, handle->descriptor.parameters_struct.size);
    handle->state_holder = (char*) plugin_allocate(allocator, handle->descriptor.state_struct.size);
    
    handle->default_parameters_f(handle->parameters_holder);
    handle->initialize_state_f(handle->parameters_holder, 
                               handle->state_holder, 
                               audio_parameters.num_channels,
                               audio_parameters.sample_rate, 
                               nullptr);
    
    plugin_set_parameter_values_from_holder(&handle->descriptor, handle->parameter_values_ui_side, handle->parameters_holder);
    plugin_set_parameter_values_from_holder(&handle->descriptor, handle->parameter_values_audio_side, handle->parameters_holder);
    handle->ring = plugin_parameters_ring_buffer_initialize(handle->descriptor.num_parameters, RING_BUFFER_SLOT_COUNT, allocator);
}



void plugin_loading_update(Plugin_Loading_Manager *m, Audio_Thread_Context *audio_context, Audio_Parameters audio_parameters, 
                           Plugin **handle_to_pull_ir_from)
{
    
    if(compare_exchange_32(m->plugin_state,
                           Asset_File_State_VALIDATING,
                           Asset_File_State_STAGE_VALIDATION))
    {
        if(m->current_handle->failure_stage == Compiler_Failure_Stage_No_Failure)
        {
            plugin_populate_from_descriptor(m->current_handle, m->current_allocator, audio_parameters);
            m->plugin_last_write_time = win32_get_last_write_time(m->source_filename);
            audio_context->plugin = m->current_handle;
            
            octave_assert(exchange_32(m->plugin_state, Asset_File_State_STAGE_USAGE)
                          == Asset_File_State_VALIDATING);
            
            *handle_to_pull_ir_from = m->current_handle;
        }
        else
        {
            printf("compilation failed, cleaning up\n");
            
            octave_assert(!m->current_handle->llvm_jit_engine);
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
        if(m->hot_reload_handle->failure_stage == Compiler_Failure_Stage_No_Failure)
        {
            printf("hot : compiler success\n");
            
            plugin_populate_from_descriptor(m->hot_reload_handle, m->hot_reload_allocator, audio_parameters);
            
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
            octave_assert(!m->hot_reload_handle->llvm_jit_engine);
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
        
        m->current_allocator->current = m->current_allocator->base;
        
        m->compiler_thread_param = {
            .source_filename = m->source_filename,
            .handle = m->current_handle,
            .allocator = m->current_allocator,
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
        
        Plugin_Allocator *temp_alloc = m->hot_reload_allocator;
        m->hot_reload_allocator = m->current_allocator;
        m->current_allocator = temp_alloc;
        
        plugin_reset_handle(m->hot_reload_handle);
        m->hot_reload_allocator->current = m->hot_reload_allocator->base;
        
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
            
            m->hot_reload_allocator->current = m->hot_reload_allocator->base;
            
            *m->hot_reload_handle = {};
            
            printf("source : %s\n", m->source_filename);
            m->compiler_thread_param = {
                .source_filename = m->source_filename,
                .handle = m->hot_reload_handle,
                .allocator = m->hot_reload_allocator,
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

#define CUSTOM_ERROR_FLAG(flag) case flag : return StringLit(#flag); break;
String compiler_error_flag_to_string(Compiler_Error_Flag flag)
{
    switch(flag)
    {
#include "errors.inc"
        default : {
            octave_assert(false && "why doesn't this switch cover all compiler errors ?\n");
            return {};
        }
    }
}
#undef CUSTOM_ERROR_FLAG


void maybe_append_error(Compiler_Gui_Log *log, Custom_Error error)
{
    if(error.flag == Compiler_Success) return;
    
}

void copy_message_to_log(Compiler_Gui_Log *log, String message)
{
    
}

void plugin_manager_print_errors(Plugin *handle, Compiler_Gui_Log *log)
{
    switch(handle->failure_stage)
    {
        case Compiler_Failure_Stage_Clang_Second_Pass :
        case Compiler_Failure_Stage_Clang_First_Pass :{
            Clang_Error_Log *error_log = &handle->clang_error_log;
            for(u32 i = 0; i < error_log->count; i++)
            {
                String message = error_log->errors[i].error_message;
                copy_message_to_log(log, message);
            }
        }break;
        
        case Compiler_Failure_Stage_Finding_Decls :{
            Decl_Search_Log *decls = &handle->decls_search_log;
            
            if(decls->audio_callback.flag == Compiler_Success && 
               decls->default_parameters.flag == Compiler_Success&& 
               decls->initialize_state.flag == Compiler_Success)
            {
                maybe_append_error(log, decls->parameters_struct);
                maybe_append_error(log, decls->state_struct);
            }
            else 
            {
                maybe_append_error(log, decls->audio_callback);
                maybe_append_error(log, decls->default_parameters);
                maybe_append_error(log, decls->initialize_state);
            }
        }break;
        
        case Compiler_Failure_Stage_Parsing_Parameters :{
            
            Plugin_Descriptor *descriptor = &handle->descriptor;
            octave_assert(descriptor->error.flag == Compiler_Error_Recurse);
            for(u32 param_idx = 0; param_idx < descriptor->num_parameters; param_idx++)
            {
                Plugin_Descriptor_Parameter *param = &descriptor->parameters[param_idx];
                maybe_append_error(log, param->error);
            }
        }break;
        
        case Compiler_Failure_Stage_No_Failure :{
            octave_assert(false && "don't call this function if there's no error\n");
        }break;
    }
}