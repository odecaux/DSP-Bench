/* date = November 29th 2021 9:48 am */

#ifndef STRUCTS_H
#define STRUCTS_H

#include "base.h"
#include "descriptor.h"

typedef void(*callback_t)(void*, float**, unsigned int, unsigned int);
typedef void(*initializer_t)(void*);

typedef struct {
    bool worked;
    callback_t audio_callback;
    initializer_t initializer;
    Plugin_Descriptor descriptor;
} Plugin_Handle;

typedef Plugin_Handle(*try_compile_t)(const char*);

typedef struct 
{
    real32 sample_rate;
    u32 num_channels;
    u32 num_samples;
    u32 bit_depth;
} Audio_Parameters;

typedef struct 
{
    real32** audio_file_buffer;
    u64 audio_file_length;
    u64 audio_file_read_cursor;
    u64 audio_file_num_channels;
    volatile i8 file_is_valid;
    
    volatile i8 plugin_is_valid;
    callback_t callback;
    char* plugin_state_holder;
    Plugin_Descriptor* descriptor; //TODO il se passe quoi si le ui thread s'arrête et que l'audio thread continue ?
    Plugin_Parameter_Value* parameter_values_audio_side;
    Plugin_Parameters_Ring_Buffer* ring;
} Audio_Context;


#endif //STRUCTS_H
