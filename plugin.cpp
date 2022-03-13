#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "windows.h"
#include "math.h"

#include "base.h"
#include "win32_helpers.h"
#include "structs.h"
#include "plugin.h"

#include "memory.h"
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
#endif
#ifdef RELEASE
void release_jit(Plugin *plugin);
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