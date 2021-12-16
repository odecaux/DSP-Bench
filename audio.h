/* date = November 15th 2021 1:54 pm */

#ifndef AUDIO_H
#define AUDIO_H

#include "assert.h"
#include "math.h"
#include "string.h"

#include "base.h"
#include "structs.h"

#define M_2I (3.1415926535897932384626433 * 2.0)

internal u64 next_power_of_two(u64 in)
{
    in--;
    in |= in >> 1;
    in |= in >> 2;
    in |= in >> 4;
    in |= in >> 8;
    in |= in >> 16;
    in |= in >> 32;
    in++;
    return in;
}


internal void copy(real32* source, real32* destination, u32 sample_count)
{
    memcpy(destination, source, sample_count * sizeof(real32));
}


internal void copy_add(real32** in, real32** out, Audio_Parameters p)
{
    for(u64 channel = 0; channel < p.num_channels; channel++)
        for(u64 sample = 0; sample < p.num_samples; sample++)
        out[channel][sample] += in[channel][sample];
}


internal void convertInt32ToFloat (const void* source, real32* dest, i32 numSamples)
{
    assert(source != (void*) dest);
    
    const unsigned char* data = static_cast<const unsigned char*> (source);
    
    for (i32 i = 0; i < numSamples; ++i)
    {
        i32 value = data[0] | data[1] << 8 | data[2] << 16 | data[3] << 24;
        dest[i] = value / (float) ((1 << 31) - 1);
        data += 4;
    }
}

internal void convertInt24ToFloat (const void* source, real32* dest, i32 numSamples)
{
    assert(source != (void*) dest);
    
    const unsigned char* data = static_cast<const unsigned char*> (source);
    
    for (i32 i = 0; i < numSamples; ++i)
    {
        i32 value = (data[2] << 24 | data[1] << 16 | data[0] << 8);
        
        /*if (value & 0x800000)
            value|= ~0xffffff;
        */
        float fl =  double(value ) / (double)((1 << 31) - 1);
        dest[i] = fl;
        data += 3;
    }
}

internal void convertInt16ToFloat (const void* source, real32* dest, i32 numSamples)
{
    assert(source != (void*) dest);
    
    const unsigned char* data = static_cast<unsigned const char*> (source);
    
    for (i32 i = 0; i < numSamples; ++i)
    {
        i32 value = data[0] << 16 | data[1] << 24;
        
        dest[i] =  (value) / (float)((1 << 31) - 1);
        data += 2;
    }
}


internal void deinterleave(real32** destination, real32* source, i32 samples_by_channel, i32 num_channels)
{
    for(u32 sample = 0; sample < samples_by_channel; sample++)
    {
        for(u32 channel = 0; channel < num_channels; channel++)
        {
            destination[channel][sample] = source[num_channels * sample + channel];
        }
    }
}

internal void interleave(real32** in, real32* out, 
                         u64 channel_count, u64 input_offset, u64 num_samples)
{
    for(i32 sample = 0; sample < num_samples; sample++)
    {
        for(i32 channel = 0; channel < channel_count; channel++)
        {
            out[sample * channel_count + channel] = in[channel][sample + input_offset];
        }
    }
}

bool audio_initialize(void** ctx, Audio_Parameters* parameters, Audio_Context* audio_context);
void audio_uninitialize(void* ctx);

void render_audio(real32** output_buffer, Audio_Parameters audio_parameters, Audio_Context *audio_context);

#endif //AUDIO_H
