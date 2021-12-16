#define INT_ARAM(min_value, max_value) __attribute__(( annotate( "Int " #min_value " " #max_value ) )) int

#define FLOAT_ARAM(min_value, max_value) __attribute__(( annotate( "Float " #min_value " " #max_value ) )) float

#define ENUM_ARAM(enum_name)  __attribute__(( annotate( "Enum" ) )) enum_name

struct AudioData;

struct NotAudioData{
    int not_ish;
};

enum random_enum{
    A, B, C, D
};

struct AudioData{
    INT_ARAM(0, 4) ish;
    FLOAT_ARAM(0.1, 2.3) ish2;
    ENUM_ARAM(random_enum) ish3;
};




void initialize_wrapper(void* void_out_initial_parameters);
AudioData initialize()
{
    return {1, 0.4f, A};
}
void initialize_wrapper(void* void_out_initial_parameters)
{
    AudioData* out_initial_parameters = (AudioData*)void_out_initial_parameters;
    *out_initial_parameters = initialize();
}


typedef unsigned int u32;


void audio_callback_wrapper(void* data_ptr, float** out_buffer, const u32 num_channels, const unsigned int num_samples);

void audio_callback(AudioData& data, float** out_buffer, const u32 num_channels, const unsigned int num_samples)
{
    for(int channel = 0; channel < num_channels; channel++)
    {
        for(int sample = 0; sample < num_samples; sample++)
        {
            out_buffer[channel][sample] *= 2;
        }
    }
}
void audio_callback_wrapper(void* data_ptr, float** out_buffer, const u32 num_channels,const unsigned int num_samples)
{
    AudioData* data = (AudioData*)data_ptr;
    audio_callback(*data, out_buffer, num_channels, num_samples);
}
