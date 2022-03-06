/* date = December 11th 2021 1:40 am */

#ifndef DESCRIPTOR_H
#define DESCRIPTOR_H

bool plugin_descriptor_compare(Plugin_Descriptor *a, Plugin_Descriptor *b);

Plugin_Parameters_Ring_Buffer plugin_parameters_ring_buffer_initialize(u32 num_fields_by_plugin, u32 buffer_slot_count);

//TODO, ça marche pas, on sait pas qui c'est
void plugin_parameters_buffer_push(Plugin_Parameters_Ring_Buffer& ring, Plugin_Parameter_Value *new_parameters);

Plugin_Parameter_Value* plugin_parameters_buffer_pull(Plugin_Parameters_Ring_Buffer& ring);

//TODO trouver un meilleur nom
function void update_parameters_holder(Plugin_Descriptor* descriptor, 
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
    assert(false);
    return 0;
}

#endif //DESCRIPTOR_H
