/* date = December 11th 2021 1:40 am */

#ifndef DESCRIPTOR_H
#define DESCRIPTOR_H



internal float *plugin_allocate_buffer(int num_sample, Arena* allocator)
{
    return (real32*)arena_allocate(allocator, sizeof(real32) * num_sample);
}

internal float **plugin_allocate_buffers(int num_samples, int num_channels, Arena* allocator) 
{
    real32** channels = (real32**) arena_allocate(allocator, sizeof(real32*) * num_channels);
    for(int i = 0; i < num_channels; i++)
    {
        channels[i] = (real32*) arena_allocate(allocator, sizeof(real32) * num_samples);
    }
    return channels;
}

internal void *plugin_allocate_bytes(int num_bytes, Arena* allocator)
{
    return arena_allocate(allocator, num_bytes);
}


void plugin_reset_handle(Plugin *plugin);

bool plugin_descriptor_compare(Plugin_Descriptor *a, Plugin_Descriptor *b);

void plugin_write_all_errors_on_log(Plugin *handle, Compiler_Gui_Log *log);


//~ Can't name this stuff, everything tying the ui representation of parameter states with the actual memory blob passed to the plugin


void plugin_set_parameter_values_from_holder(Plugin_Descriptor *descriptor,
                                             Plugin_Parameter_Value *parameter_values_out,
                                             char* plugin_parameters_holder);

void plugin_set_parameter_holder_from_values(Plugin_Descriptor* descriptor, 
                                             Plugin_Parameter_Value* new_values,
                                             char* plugin_parameters_holder);

//TODO, ça marche pas, on sait pas qui c'est
void plugin_parameters_push_to_ring(Plugin_Parameters_Ring_Buffer& ring, Plugin_Parameter_Value *new_parameters);


Plugin_Parameter_Value* plugin_parameters_pull_from_ring(Plugin_Parameters_Ring_Buffer& ring);



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
    //TODO si y a zero entries ça crash
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
    ensure(false);
    return 0;
}

//~ Reloading Manager

typedef struct {
    const char* source_filename;
    Plugin *handle;
    Arena *allocator;
    void *clang_ctx;
    
    Asset_File_State *stage;
} Compiler_Thread_Param;


struct Plugin_Reloading_Manager
{
    Plugin handle_a;
    Plugin handle_b;
    Arena allocator_a;
    Arena allocator_b;
    
    Plugin *front_handle;
    Plugin *back_handle;
    
    Plugin *front_handle_audio_side;
    Plugin *back_handle_audio_side;
    
    Arena *front_allocator;
    Arena *back_allocator;
    
    Compiler_Thread_Param compiler_thread_param;
    HANDLE compiler_thread_handle;
    u64 plugin_last_write_time;
    void *clang_ctx;
    Asset_File_State *plugin_state;
    char *source_filename;
    
    Compiler_Gui_Log gui_log;
};


void plugin_reloading_manager_init(Plugin_Reloading_Manager *m, void *clang_ctx, char *source_filename, Asset_File_State *plugin_state);

void plugin_reloader_stage_cold_compilation(Plugin_Reloading_Manager *m);

void plugin_reloader_stage_hot_compilation(Plugin_Reloading_Manager *m);

void plugin_reloading_update_gui_side(Plugin_Reloading_Manager *m, Audio_Thread_Context *audio_context, Audio_Parameters audio_parameters, Plugin **handle_to_pull_ir_from);

void plugin_reloading_update_audio_side(Plugin_Reloading_Manager *m);


void plugin_load_button_was_clicked(Plugin_Reloading_Manager *m);

void plugin_check_for_save_and_stage_hot_reload(Plugin_Reloading_Manager *m);


#endif //DESCRIPTOR_H
