/* date = November 29th 2021 9:48 am */

#ifndef STRUCTS_H
#define STRUCTS_H

#include "base.h"
#include "descriptor.h"

typedef void*(*buffer_allocator_t)(unsigned int);

typedef void(*audio_callback_t)(void*, void*, float**, unsigned int, unsigned int, float);
typedef void(*default_parameters_t)(void*);
typedef void(*initialize_state_t)(void*, void*, unsigned int, float, buffer_allocator_t);

internal void* malloc_allocator(u32 byte_size)
{
    return malloc(byte_size);
}

typedef struct {
    bool worked;
    
    void* llvm_jit_engine;
    audio_callback_t audio_callback_f;
    default_parameters_t default_parameters_f;
    initialize_state_t initialize_state_f;
    Plugin_Descriptor descriptor;
} Plugin_Handle;

typedef Plugin_Handle(*try_compile_t)(const char*, const void*);

typedef struct 
{
    real32 sample_rate;
    u32 num_channels;
    u32 num_samples;
    u32 bit_depth;
} Audio_Parameters;

typedef struct 
{
    volatile i8 file_is_valid;
    
    real32** audio_file_buffer;
    u64 audio_file_length;
    u64 audio_file_read_cursor;
    u64 audio_file_num_channels;
    
    volatile i8 plugin_is_valid;
    audio_callback_t audio_callback_f;
    
    char* plugin_parameters_holder;
    char* plugin_state_holder;
    
    //TODO il se passe quoi si le ui thread s'arrête et que l'audio thread continue ?
    Plugin_Descriptor* descriptor; 
    Plugin_Parameter_Value* parameter_values_audio_side;
    Plugin_Parameters_Ring_Buffer* ring;
} Audio_Context;


#endif //STRUCTS_H
