#include "string.h"
#include "windows.h"
#include "math.h"
#include "base.h"
#include "structs.h"
#include "descriptor.h"


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

//TODO, ça marche pas, on sait pas qui c'est
void plugin_parameters_buffer_push(Plugin_Parameters_Ring_Buffer& ring, Plugin_Parameter_Value *new_parameters)
{
    Plugin_Parameter_Value *pointer_to_push = &ring.buffer[ring.writer_idx];
    
    for(u32 i = 0; i < ring.num_fields_by_plugin; i++)
        pointer_to_push[i] = new_parameters[i];
    
    ring.writer_idx += ring.num_fields_by_plugin;
    if(ring.writer_idx == ring.buffer_size) 
        ring.writer_idx = 0;
    
    InterlockedExchangePointer((void**)&ring.head, pointer_to_push);
}

Plugin_Parameter_Value* plugin_parameters_buffer_pull(Plugin_Parameters_Ring_Buffer& ring)
{
    Plugin_Parameter_Value *maybe_plugin_array =  (Plugin_Parameter_Value*) InterlockedExchangePointer((void**)&ring.head, nullptr);
    
    return maybe_plugin_array;
}
