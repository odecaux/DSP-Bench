/* date = December 11th 2021 1:40 am */

#ifndef DESCRIPTOR_H
#define DESCRIPTOR_H




typedef struct {
    const char* source_filename;
    Plugin *handle;
    Plugin_Allocator *allocator;
    void *clang_ctx;
    
    Asset_File_State *stage;
} Compiler_Thread_Param;


#define word_size (sizeof(void*))
#define align(n) ((n + word_size - 1) & ~(word_size - 1))


internal void *plugin_allocate(Plugin_Allocator *allocator, u64 size)
{
    char *begin = allocator->current;
    u64 aligned = align(size);
    if(begin + aligned <= allocator->base + allocator->capacity)
    {
        allocator->current += aligned;
        return begin;
    }
    else return nullptr;
}

struct Plugin_Loading_Manager
{
    Plugin handle_a;
    Plugin handle_b;
    Plugin_Allocator allocator_a;
    Plugin_Allocator allocator_b;
    
    Plugin *current_handle;
    Plugin *hot_reload_handle;
    
    Plugin_Allocator *current_allocator;
    Plugin_Allocator *hot_reload_allocator;
    
    Compiler_Thread_Param compiler_thread_param;
    HANDLE compiler_thread_handle;
    u64 plugin_last_write_time;
    void *clang_ctx;
    Asset_File_State *plugin_state;
    char *source_filename;
    
    Compiler_Gui_Log gui_log;
};


void plugin_loading_manager_init(Plugin_Loading_Manager *m, void *clang_ctx, char *source_filename, Asset_File_State *plugin_state);

void plugin_loading_update(Plugin_Loading_Manager *m, Audio_Thread_Context *audio_context, Audio_Parameters audio_parameters, Plugin **handle_to_pull_ir_from);

void plugin_load_button_was_clicked(Plugin_Loading_Manager *m);

void plugin_loading_check_and_stage_hot_reload(Plugin_Loading_Manager *m);


void plugin_manager_print_errors(Plugin *handle, Compiler_Gui_Log *log);

bool plugin_descriptor_compare(Plugin_Descriptor *a, Plugin_Descriptor *b);


void plugin_set_parameter_values_from_holder(Plugin_Descriptor *descriptor,
                                             Plugin_Parameter_Value *parameter_values_out,
                                             char* holder);

void plugin_set_parameter_holder_from_values(Plugin_Descriptor* descriptor, 
                                             Plugin_Parameter_Value* new_values,
                                             char* plugin_parameters_holder);

//TODO, ça marche pas, on sait pas qui c'est
void plugin_parameters_buffer_push(Plugin_Parameters_Ring_Buffer& ring, Plugin_Parameter_Value *new_parameters);


void plugin_reset_handle(Plugin *plugin);

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
    octave_assert(false);
    return 0;
}

#endif //DESCRIPTOR_H
