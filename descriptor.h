/* date = December 11th 2021 1:40 am */

#ifndef DESCRIPTOR_H
#define DESCRIPTOR_H


#define CUSTOM_ERROR_FLAG(flag) case flag : return StringLit(#flag); break;


internal String compiler_error_flag_to_string(Compiler_Error_Flag flag)
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

bool plugin_descriptor_compare(Plugin_Descriptor *a, Plugin_Descriptor *b);

Plugin_Parameters_Ring_Buffer plugin_parameters_ring_buffer_initialize(u32 num_fields_by_plugin, u32 buffer_slot_count);

void plugin_set_parameter_values_from_holder(Plugin_Descriptor *descriptor,
                                             Plugin_Parameter_Value *parameter_values_out,
                                             char* holder);

void plugin_set_parameter_holder_from_values(Plugin_Descriptor* descriptor, 
                                             Plugin_Parameter_Value* new_values,
                                             char* plugin_parameters_holder);

//TODO, ça marche pas, on sait pas qui c'est
void plugin_parameters_buffer_push(Plugin_Parameters_Ring_Buffer& ring, Plugin_Parameter_Value *new_parameters);

Plugin_Parameter_Value* plugin_parameters_buffer_pull(Plugin_Parameters_Ring_Buffer& ring);


function real32 normalize_parameter_int_value(Parameter_Int param, real32 value)
{
    return real32(value - param.min)/real32(param.max - param.min);
}



function real32 normalize_parameter_float_value(Parameter_Float param, real32 value)
{
    value = octave_clamp(value, param.min, param.max);
    
    if(value == param.min)
        return 0.0f;
    else if(param.log == true)
        return log(value/param.min) / log(param.max/param.min);
    else
        return (value - param.min)/(param.max - param.min);
}


function float normalize_parameter_enum_index(Parameter_Enum param, i32 index)
{
    if(param.num_entries == 1)
        return 0.0f;
    else 
        return float(index)/(param.num_entries - 1);
}


function i32 denormalize_int_value(Parameter_Int& parameter, real32 normalized_value)
{
    return i32(normalized_value * (parameter.max - parameter.min) + parameter.min);
}

function real32 denormalize_float_value(Parameter_Float& parameter, real32 normalized_value)
{
    if(parameter.log == true)
        return parameter.min* exp(normalized_value * log(parameter.max/parameter.min));
    else
        return normalized_value * (parameter.max - parameter.min) + parameter.min;
}

function u32 denormalize_enum_index(Parameter_Enum& parameter, real32 normalized_value)
{
    return u32(normalized_value * (parameter.num_entries - 1));
}

function i64 enum_index_to_value(Parameter_Enum& parameter, u32 index)
{
    return parameter.entries[index].value;
}

function u32 enum_value_to_index(Parameter_Enum& parameter, i64 value)
{
    for(auto i = 0; i < parameter.num_entries; i++)
    {
        if(parameter.entries[i].value == value)
            return i;
    }
    octave_assert(false);
    return 0;
}

#endif //DESCRIPTOR_H
