/* date = November 15th 2021 1:54 pm */

#ifndef AUDIO_H
#define AUDIO_H


enum Wav_Reading_Error{
    Wav_Success,
    Wav_Could_Not_Open_File,
    Wav_Not_A_RIFF,
    Wav_File_Reading_Error,
    Wav_Invalid_Format
};

typedef struct {
    Wav_Reading_Error error;
    u32 num_channels;
    u32 samples_by_channel;
    real32** deinterleaved_buffer;
    u32 read_cursor;
} Audio_File;


typedef struct 
{
    real32 sample_rate;
    u32 num_channels;
    u32 num_samples;
    u32 bit_depth;
} Audio_Parameters;

struct Plugin_Reloading_Manager;

typedef struct 
{
    i8 audio_file_play;
    i8 audio_file_loop;
    i8 plugin_play;
    
    Asset_File_State *plugin_state;
    Asset_File_State *audio_file_state;
    
    Plugin_Reloading_Manager *m;
    Audio_File *audio_file;
} Audio_Thread_Context;


//TODO what if in == 0
internal u32 next_power_of_two(u32 in)
{
    if(in == 0) return 0;
    
    in--;
    in |= in >> 1;
    in |= in >> 2;
    in |= in >> 4;
    in |= in >> 8;
    in |= in >> 16;
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
    ensure(source != (void*) dest);
    const u8* data = (const u8*)(source);
    
    for (i32 i = 0; i < numSamples; ++i)
    {
        i32 value = data[0] | data[1] << 8 | data[2] << 16 | data[3] << 24;
        dest[i] = value / (float) ((1 << 31) - 1);
        data += 4;
    }
}

internal void convertInt24ToFloat (const void* source, real32* dest, i32 numSamples)
{
    ensure(source != (void*) dest);
    const u8* data = (const u8*)(source);
    
    for (i32 i = 0; i < numSamples; ++i)
    {
        i32 value = (data[2] << 24 | data[1] << 16 | data[0] << 8);
        
        /*if (value & 0x800000)
            value|= ~0xffffff;
        */
        real32 fl =  real32(double(value ) / (double)((1 << 31) - 1));
        dest[i] = fl;
        data += 3;
    }
}

internal void convertInt16ToFloat (const void* source, real32* dest, i32 numSamples)
{
    ensure(source != (void*) dest);
    const u8* data = (const u8*)(source);
    
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

bool audio_initialize(void** ctx, Audio_Parameters* parameters, Audio_Thread_Context* audio_context);
void audio_uninitialize(void* ctx);

void render_audio(real32** output_buffer, Audio_Parameters audio_parameters, Audio_Thread_Context *audio_context);

#endif //AUDIO_H
