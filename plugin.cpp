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
                                             char* holder)
{
    for(auto param_idx = 0; param_idx < descriptor->num_parameters ; param_idx++)
    {
        auto& param_descriptor = descriptor->parameters[param_idx];
        auto offset = param_descriptor.offset;
        
        switch(param_descriptor.type){
            case Int :
            {
                *(int*)(holder + offset) = new_values[param_idx].int_value;
            }break;
            case Float : 
            {
                *(float*)(holder + offset) = new_values[param_idx].float_value;
            }break;
            case Enum : 
            {
                *(int*)(holder + offset) = new_values[param_idx].enum_value;
            }break;
        }
    }
}


//TODO, ça marche pas, on sait pas qui c'est
void plugin_parameters_push_to_ring(Plugin_Parameters_Ring_Buffer& ring, Plugin_Parameter_Value *new_parameters)
{
    Plugin_Parameter_Value *pointer_to_push = &ring.buffer[ring.writer_idx];
    
    for(u32 i = 0; i < ring.num_fields_by_plugin; i++)
        pointer_to_push[i] = new_parameters[i];
    
    ring.writer_idx += ring.num_fields_by_plugin;
    if(ring.writer_idx == ring.buffer_size) 
        ring.writer_idx = 0;
    
    exchange_ptr((void**)&ring.head, pointer_to_push);
}

Plugin_Parameter_Value* plugin_parameters_pull_from_ring(Plugin_Parameters_Ring_Buffer& ring)
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
Plugin try_compile(const char* filename, void* clang_ctx_ptr, Plugin_Allocator *allocator);
#endif

void plugin_reset_handle(Plugin *handle, Plugin_Allocator *allocator)
{
    handle->parameter_values_audio_side = nullptr;
    handle->parameter_values_ui_side = nullptr;
    handle->ring.buffer = nullptr;
    
    release_jit(handle);
    *handle = {};
    allocator->current = allocator->base;
}



DWORD compiler_thread_proc(void *void_param)
{
    Compiler_Thread_Param *param = (Compiler_Thread_Param*)void_param;
    if(compare_exchange_32(param->stage, Asset_File_State_BACKGROUND_LOADING, Asset_File_State_STAGE_BACKGROUND_LOADING) )
    {
        
        *param->handle = try_compile(param->source_filename, param->clang_ctx, param->allocator);
        octave_assert(compare_exchange_32(param->stage, Asset_File_State_STAGE_VALIDATION, Asset_File_State_BACKGROUND_LOADING));
    }
    else if(compare_exchange_32(param->stage, 
                                Asset_File_State_HOT_RELOAD_BACKGROUND_LOADING, 
                                Asset_File_State_HOT_RELOAD_STAGE_BACKGROUND_LOADING) )
    {
        
        *param->handle = try_compile(param->source_filename, param->clang_ctx, param->allocator);
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
        .base = (char*)m_allocate(size, "plugin : allocator"),
        .current = allocator.base,
        .capacity = size,
    };
    return allocator;
}


HANDLE launch_compiler_thread(Compiler_Thread_Param *thread_parameters)
{
    return CreateThread(0, 0,
                        &compiler_thread_proc,
                        thread_parameters,
                        0, 0);
}

void plugin_reloader_stage_cold_compilation(Plugin_Reloading_Manager *m)
{
    octave_assert(m->source_filename[0] != 0);
    
    m->compiler_thread_param = Compiler_Thread_Param {
        .source_filename = m->source_filename,
        .handle = m->front_handle,
        .allocator = m->front_allocator,
        .clang_ctx = m->clang_ctx,
        .stage = m->plugin_state
    };
    auto old_state = exchange_32(m->plugin_state, Asset_File_State_STAGE_BACKGROUND_LOADING);
    
    octave_assert(old_state == Asset_File_State_NONE || old_state == Asset_File_State_UNLOADING);
    
    m->compiler_thread_handle = launch_compiler_thread(&m->compiler_thread_param);
}

void plugin_reloader_stage_hot_compilation(Plugin_Reloading_Manager *m)
{
    octave_assert(m->source_filename[0] != 0);
    
    m->compiler_thread_param = Compiler_Thread_Param {
        .source_filename = m->source_filename,
        .handle = m->back_handle,
        .allocator = m->back_allocator,
        .clang_ctx = m->clang_ctx,
        .stage = m->plugin_state
    };
    octave_assert(compare_exchange_32(m->plugin_state, Asset_File_State_HOT_RELOAD_STAGE_BACKGROUND_LOADING,
                                      Asset_File_State_HOT_RELOAD_CHECK_FILE_FOR_UPDATE));
    
    m->compiler_thread_handle = launch_compiler_thread(&m->compiler_thread_param);
}

void plugin_reloading_manager_init(Plugin_Reloading_Manager *m, 
                                   void *clang_ctx, 
                                   char *source_filename, 
                                   Asset_File_State *plugin_state)
{
    *m = {
        .allocator_a = plugin_allocator_init(1024 * 1024),
        .allocator_b = plugin_allocator_init(1024 * 1204),
        
        .front_handle = &m->handle_a,
        .back_handle = &m->handle_b,
        
        .front_handle_audio_side = &m->handle_a,
        .back_handle_audio_side = &m->handle_b,
        
        .front_allocator = &m->allocator_a,
        .back_allocator = &m->allocator_b,
        
        .clang_ctx = clang_ctx,
        .plugin_state = plugin_state,
        .source_filename = source_filename,
        
        .gui_log = {
            .messages = m_allocate_array(String, 256, "gui error log : messages"),
            .message_count = 0,
            .message_capacity = 256,
            
            .holder_base = m_allocate_array(char, 1024*16, "gui error log : holder"),
            .holder_current = m->gui_log.holder_base,
            .holder_capacity = 1024*16
        }
    };
}


void plugin_populate_from_descriptor(Plugin *handle, 
                                     Plugin_Allocator *allocator, 
                                     Audio_Parameters audio_parameters)
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



void plugin_reloading_update_gui_side(Plugin_Reloading_Manager *m, 
                                      Audio_Thread_Context *audio_context, 
                                      Audio_Parameters audio_parameters,
                                      Plugin **handle_to_pull_ir_from)
{
    
    if(compare_exchange_32(m->plugin_state,
                           Asset_File_State_VALIDATING,
                           Asset_File_State_STAGE_VALIDATION))
    {
        if(m->front_handle->failure_stage == Compiler_Failure_Stage_No_Failure)
        {
            plugin_populate_from_descriptor(m->front_handle, m->front_allocator, audio_parameters);
            m->plugin_last_write_time = win32_get_last_write_time(m->source_filename);
            
            octave_assert(exchange_32(m->plugin_state, Asset_File_State_STAGE_USAGE)
                          == Asset_File_State_VALIDATING);
            
            *handle_to_pull_ir_from = m->front_handle;
        }
        else
        {
            printf("compilation failed, cleaning up\n");
            
            octave_assert(!m->front_handle->llvm_jit_engine);
            octave_assert(!m->front_handle->parameter_values_audio_side);
            octave_assert(!m->front_handle->parameter_values_ui_side);
            octave_assert(!m->front_handle->ring.buffer);
            plugin_write_all_errors_on_log(m->front_handle, &m->gui_log);
            
            m->plugin_last_write_time = win32_get_last_write_time(m->source_filename);
            
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
        if(m->back_handle->failure_stage == Compiler_Failure_Stage_No_Failure)
        {
            printf("hot : compiler success\n");
            
            plugin_populate_from_descriptor(m->back_handle, m->back_allocator, audio_parameters);
            
            m->plugin_last_write_time = win32_get_last_write_time(m->source_filename);
            *handle_to_pull_ir_from = m->back_handle;
            
            octave_assert(compare_exchange_32(m->plugin_state,
                                              Asset_File_State_HOT_RELOAD_STAGE_SWAP,
                                              Asset_File_State_HOT_RELOAD_VALIDATING));
            
            printf("hot reload validation done\n");
        }
        else 
        {
            
            printf("hot : compilation failed\n");
            octave_assert(!m->back_handle->llvm_jit_engine);
            octave_assert(!m->back_handle->parameter_values_audio_side);
            octave_assert(!m->back_handle->parameter_values_ui_side);
            octave_assert(!m->back_handle->ring.buffer);
            
            plugin_write_all_errors_on_log(m->back_handle, &m->gui_log);
            
            printf("hot : done cleaning up, back in use\n");
            
            if(m->front_handle->descriptor.error.flag != Compiler_Success){
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
        plugin_reset_handle(m->front_handle, m->front_allocator);
        plugin_reloader_stage_cold_compilation(m);
    }
    else if(compare_exchange_32(m->plugin_state,
                                Asset_File_State_HOT_RELOAD_DISPOSING,
                                Asset_File_State_HOT_RELOAD_STAGE_DISPOSE))
    {
        printf("hot reload disposing\n");
        
        Plugin *temp = m->back_handle;
        m->back_handle = m->front_handle;
        m->front_handle = temp;
        
        Plugin_Allocator *temp_alloc = m->back_allocator;
        m->back_allocator = m->front_allocator;
        m->front_allocator = temp_alloc;
        
        plugin_reset_handle(m->back_handle, m->back_allocator);
        
        compare_exchange_32(m->plugin_state,
                            Asset_File_State_STAGE_USAGE,
                            Asset_File_State_HOT_RELOAD_DISPOSING);
        printf("hot : in use\n");
    }
    
}

void plugin_reloading_update_audio_side(Plugin_Reloading_Manager *m)
{
    MemoryBarrier();
    auto plugin_state = *m->plugin_state;
    MemoryBarrier();
    
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
            Plugin_Parameter_Value* maybe_new_parameter_values = plugin_parameters_pull_from_ring(m->front_handle_audio_side->ring);
            if(maybe_new_parameter_values)
            {
                for(auto param_idx = 0; param_idx < m->front_handle_audio_side->ring.num_fields_by_plugin; param_idx++)
                {
                    m->front_handle_audio_side->parameter_values_audio_side[param_idx] = maybe_new_parameter_values[param_idx];
                }
                
                MemoryBarrier();
                octave_assert(m->front_handle_audio_side->ring.num_fields_by_plugin == m->front_handle_audio_side->descriptor.num_parameters);
                
                plugin_set_parameter_holder_from_values(&m->front_handle_audio_side->descriptor, 
                                                        m->front_handle_audio_side->parameter_values_audio_side, m->front_handle_audio_side->parameters_holder);
            }
        }break;
        case Asset_File_State_HOT_RELOAD_SWAPPING :
        {
            octave_assert(false);
        }break;
    }
    
    
    if(compare_exchange_32(m->plugin_state,
                           Asset_File_State_IN_USE,
                           Asset_File_State_STAGE_USAGE))
    {}
    
    if(compare_exchange_32(m->plugin_state,
                           Asset_File_State_HOT_RELOAD_SWAPPING,
                           Asset_File_State_HOT_RELOAD_STAGE_SWAP))
    {
        Plugin * temp = m->back_handle_audio_side;
        m->back_handle_audio_side = m->front_handle_audio_side;
        m->front_handle_audio_side = temp;
        
        octave_assert(compare_exchange_32(m->plugin_state,
                                          Asset_File_State_HOT_RELOAD_STAGE_DISPOSE,
                                          Asset_File_State_HOT_RELOAD_SWAPPING));
    }
    
    
    
    if(compare_exchange_32(m->plugin_state,
                           Asset_File_State_OK_TO_UNLOAD,
                           Asset_File_State_STAGE_UNLOADING))
    {
        //m->front_handle_audio_side = nullptr;
    }
    
    if(compare_exchange_32(m->plugin_state,
                           Asset_File_State_COLD_RELOAD_STAGE_UNLOAD,
                           Asset_File_State_COLD_RELOAD_STAGE_UNUSE))
    {
        //m->front_handle_audio_side = nullptr;
    }
    
}

void plugin_load_button_was_clicked(Plugin_Reloading_Manager *m)
{
    auto old_wave_state = 
        exchange_32(m->plugin_state, Asset_File_State_COLD_RELOAD_STAGE_UNUSE);
    //TODO octave_assert, dans quels états peut être old_wav_state qui nous foutraient dans la merde ?
}


void plugin_check_for_save_and_stage_hot_reload(Plugin_Reloading_Manager *m)
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
            octave_assert(!m->back_handle->parameter_values_audio_side);
            octave_assert(!m->back_handle->parameter_values_ui_side);
            octave_assert(!m->back_handle->ring.buffer);
            octave_assert(!m->back_handle->llvm_jit_engine);
            
            plugin_reset_handle(m->back_handle, m->back_allocator);
            plugin_reloader_stage_hot_compilation(m);
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

//~ Error messages


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

void maybe_append_fun_error(Compiler_Gui_Log *log, String function_name, Custom_Error error)
{
    if(error.flag == Compiler_Success) return;
    octave_assert(log->message_count < log->message_capacity);
    
    String *new_message = &log->messages[log->message_count++];
    new_message->str = log->holder_current;
    
    String flag_string = compiler_error_flag_to_string(error.flag);
    i32 written_chars;
    
    switch(error.flag)
    {
        case Compiler_Too_Many_Fun : {
            written_chars = sprintf(new_message->str, "%s : %s between %lu:%lu and %lu:%lu", function_name.str, flag_string.str, error.location.line, error.location.column, error.location_2.line, error.location_2.column);
        }break;
        case Compiler_No_Fun :{
            written_chars = sprintf(new_message->str, "%s : %s", function_name.str, flag_string.str);
        }break;
        case Compiler_Wrong_Signature_Fun : {
            written_chars = sprintf(new_message->str, "%s : %s at %lu:%lu", function_name.str, flag_string.str, error.location.line, error.location.column); 
        }break;
        default : assert(false && "invalid function decl flag"); break;
    }
    
    //TODO written_chars = sprintf only valid because C string literals are null terminated
    
    assert(written_chars > 0);
    
    new_message->size = written_chars + 1;
    log->holder_current += align(new_message->size);
}



void maybe_append_struct_error(Compiler_Gui_Log *log, String struct_name, Custom_Error error)
{
    if(error.flag == Compiler_Success) return;
    octave_assert(log->message_count < log->message_capacity);
    
    String *new_message = &log->messages[log->message_count++];
    new_message->str = log->holder_current;
    
    String flag_string = compiler_error_flag_to_string(error.flag);
    i32 written_chars;
    
    switch(error.flag)
    {
        case Compiler_Types_Mismatch : {
            written_chars = sprintf(new_message->str, "%s : %s between %lu:%lu, %lu:%lu and %lu:%lu", struct_name.str, flag_string.str, error.location.line, error.location.column, error.location_2.line, error.location_2.column, error.location_3.line, error.location_3.column);
            
        }break;
        case Compiler_Not_Record_Type :
        case Compiler_Polymorphic : {
            written_chars = sprintf(new_message->str, "%s : %s at %lu.%lu", struct_name.str, flag_string.str, error.location.line, error.location.column); 
        }break;
        default : assert(false && "invalid function decl flag"); break;
    }
    
    assert(written_chars > 0);
    
    new_message->size = written_chars + 1;
    log->holder_current += align(new_message->size);
}


void maybe_append_parameter_error(Compiler_Gui_Log *log, u32 parameter_idx, Plugin_Descriptor_Parameter *param)
{
    Custom_Error error = param->error;
    if(error.flag == Compiler_Success) return;
    octave_assert(log->message_count < log->message_capacity);
    
    String *new_message = &log->messages[log->message_count++];
    new_message->str = log->holder_current;
    
    String flag_string = compiler_error_flag_to_string(error.flag);
    i32 written_chars;
    
    switch(error.flag)
    {
        case Compiler_Empty_Annotation : 
        case Compiler_Invalid_Annotation : 
        case Compiler_Annotation_Type_Mismatch :
        case Compiler_Missing_Min_Max : 
        case Compiler_Invalid_Min_Value : 
        case Compiler_Invalid_Max_Value : 
        case Compiler_Min_Greater_Than_Max : {
            written_chars = sprintf(new_message->str, "parameter %lu : %s at %lu.%lu", parameter_idx, flag_string.str, error.location.line, error.location.column); 
        }break;
        default : assert(false && "invalid function decl flag"); break;
    }
    
    assert(written_chars > 0);
    
    new_message->size = written_chars + 1;
    log->holder_current += align(new_message->size);
}


void append_clang_message_to_log(Compiler_Gui_Log *log, Clang_Error *error)
{
    octave_assert(log->message_count < log->message_capacity);
    String *new_message = &log->messages[log->message_count++];
    new_message->str = log->holder_current;
    
    i32 location_char_length = sprintf(new_message->str, "%lu:%lu : ", error->location.line, error->location.column);
    
    strncpy(new_message->str + location_char_length, error->message.str, error->message.size);
    
    new_message->size = error->message.size + location_char_length;
    log->holder_current += align(new_message->size);
}

void plugin_write_all_errors_on_log(Plugin *handle, Compiler_Gui_Log *log)
{
    log->message_count = 0;
    log->holder_current = log->holder_base;
    
    switch(handle->failure_stage)
    {
        case Compiler_Failure_Stage_Clang_Second_Pass :
        case Compiler_Failure_Stage_Clang_First_Pass :{
            Clang_Error_Log *error_log = &handle->clang_error_log;
            for(u32 i = 0; i < error_log->count; i++)
            {
                append_clang_message_to_log(log, &error_log->errors[i]);
            }
        }break;
        
        case Compiler_Failure_Stage_Finding_Decls :{
            Decl_Search_Log *decls = &handle->decls_search_log;
            
            if(decls->audio_callback.flag == Compiler_Success && 
               decls->default_parameters.flag == Compiler_Success&& 
               decls->initialize_state.flag == Compiler_Success)
            {
                maybe_append_struct_error(log, StringLit("Parameter struct"), decls->parameters_struct);
                maybe_append_struct_error(log, StringLit("State struct"), decls->state_struct);
            }
            else 
            {
                maybe_append_fun_error(log, StringLit("audio_callback"), decls->audio_callback);
                maybe_append_fun_error(log, StringLit("default_parameters"), decls->default_parameters);
                maybe_append_fun_error(log, StringLit("initalize_state"), decls->initialize_state);
            }
        }break;
        
        case Compiler_Failure_Stage_Parsing_Parameters :{
            
            Plugin_Descriptor *descriptor = &handle->descriptor;
            octave_assert(descriptor->error.flag == Compiler_Error_Recurse);
            for(u32 param_idx = 0; param_idx < descriptor->num_parameters; param_idx++)
            {
                Plugin_Descriptor_Parameter *param = &descriptor->parameters[param_idx];
                maybe_append_parameter_error(log, param_idx, param);
            }
        }break;
        
        case Compiler_Failure_Stage_No_Failure :{
            octave_assert(false && "don't call this function if there's no error\n");
        }break;
    }
}