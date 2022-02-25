#include "windows.h"
#include "math.h"
#include "base.h"
#include "structs.h"
#include "descriptor.h"

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
