/* date = December 11th 2021 1:40 am */

#ifndef DESCRIPTOR_H
#define DESCRIPTOR_H

#include "stdlib.h"

enum Plugin_Parameter_Type{
    Int,
    Float,
    Enum
};

typedef struct{
    i32 min;
    i32 max;
} Parameter_Int;

typedef struct {
    real32 min;
    real32 max;
} Parameter_Float;

typedef struct{
    i32 value;
    String name;
} Parameter_Enum_Entry;


typedef struct {
    Parameter_Enum_Entry *entries;
    u32 num_entries;
} Parameter_Enum;

typedef struct {
    String name;
    u32 offset;
    
    Plugin_Parameter_Type type;
    union {
        Parameter_Int int_param;
        Parameter_Float float_param;
        Parameter_Enum enum_param;
    };
    
} Plugin_Descriptor_Parameter;

typedef struct {
    //String name;
    //le nom c'est le filename ?
    struct { 
        u32 size;
        u32 alignment;
    } parameters_struct;
    
    struct { 
        u32 size;
        u32 alignment;
    } state_struct;
    
    Plugin_Descriptor_Parameter *parameters;
    u32 num_parameters;
} Plugin_Descriptor;

typedef struct {
    union{
        int int_value;
        float float_value;
        int enum_value;
    };
} Plugin_Parameter_Value;


//TODO pour l'instant il y a un bug : si l'ui arrive à emettre 4096 blocs pendant que le thread audio fait 1 copie
typedef struct{
    Plugin_Parameter_Value *buffer;
    volatile Plugin_Parameter_Value **head;
    u32 writer_idx;
    u32 buffer_size;
    u32 num_fields_by_plugin;
} Plugin_Parameters_Ring_Buffer;


Plugin_Parameters_Ring_Buffer plugin_parameters_ring_buffer_initialize(u32 num_fields_by_plugin, u32 buffer_slot_count);

//TODO, ça marche pas, on sait pas qui c'est
void plugin_parameters_buffer_push(Plugin_Parameters_Ring_Buffer& ring, Plugin_Parameter_Value *new_parameters);

Plugin_Parameter_Value* plugin_parameters_buffer_pull(Plugin_Parameters_Ring_Buffer& ring);

//TODO trouver un meilleur nom
internal void update_parameters_holder(Plugin_Descriptor* descriptor, 
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
                //printf("%d\n", parameter_values_audio_side[param_idx].int_value);
                *(int*)(plugin_parameters_holder + offset) = new_values[param_idx].int_value;
            }break;
            case Float : 
            {
                //printf("%f\n", new_values[param_idx].float_value);
                *(float*)(plugin_parameters_holder + offset) = new_values[param_idx].float_value;
            }break;
            case Enum : 
            {
                //printf("%d\n", new_values[param_idx].enum_value);
                *(int*)(plugin_parameters_holder + offset) = new_values[param_idx].enum_value;
            }break;
        }
    }
}


internal float normalize_parameter_int_value(Parameter_Int param, int value)
{
    return float(value - param.min)/float(param.max - param.min);
}

internal float normalize_parameter_float_value(Parameter_Float param, float value)
{
    if(value == param.min)
        return 0.0f;
    else
        return (value - param.min)/(param.max - param.min);
}


internal float normalize_parameter_enum_index(Parameter_Enum param, i32 index)
{
    if(param.num_entries == 1)
        return 0.0f;
    else 
        return float(index)/(param.num_entries - 1);
}


internal int denormalize_int_value(Parameter_Int& parameter, float normalized_value)
{
    return normalized_value * (parameter.max - parameter.min) + parameter.min;
}

internal float denormalize_float_value(Parameter_Float& parameter, float normalized_value)
{
    return normalized_value * (parameter.max - parameter.min) + parameter.min;
}

internal u32 denormalize_enum_index(Parameter_Enum& parameter, float normalized_value)
{
    return normalized_value * (parameter.num_entries - 1);
}

internal int enum_index_to_value(Parameter_Enum& parameter, u32 index)
{
    return parameter.entries[index].value;
}

internal u32 enum_value_to_index(Parameter_Enum& parameter, int value)
{
    for(auto i = 0; i < parameter.num_entries; i++)
    {
        if(parameter.entries[i].value == value)
            return i;
    }
    assert(false);
    return 0;
}

#endif //DESCRIPTOR_H
