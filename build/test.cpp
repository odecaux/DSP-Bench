
#define INT_ARAM(min_value, max_value) __attribute__(( annotate( "Int " #min_value " " #max_value ) )) int

#define FLOAT_ARAM(min_value, max_value) __attribute__(( annotate( "Float " #min_value " " #max_value ) )) float

#define ENUM_ARAM(enum_name)  __attribute__(( annotate( "Enum" ) )) enum_name

/*
enum random_enum{
    A, B, C, D
};
*/
// TODO(octave): courbe exponentielle, power factor

typedef void*(*allocator_t)(unsigned int);

struct Parameters{
    INT_ARAM(0, 4) ish;
    FLOAT_ARAM(0.0f, 1.0f) gain;
    //ENUM_ARAM(random_enum) truc;
};

struct State{
    double current_gain;
};


Parameters default_parameters()
{
    Parameters initial_state = {0, 0.9f};
    return initial_state;
}

State initialize_state(const Parameters& param, 
                       const unsigned int num_channels, 
                       const float sample_rate,
                       allocator_t allocator)
{
    State initial_state = {param.gain};
    return initial_state;
}

typedef float real32;
typedef unsigned int u32;

void audio_callback(const Parameters& param,
                    State& state,
                    float** out_buffer, 
                    const u32  num_channels, 
                    const u32  num_samples,
                    const real32 sample_rate)
{
    float real_gain = data.gain * data.gain;
    for(int channel = 0; channel < num_channels; channel++)
    {
        for(int sample = 0; sample < num_samples; sample++)
        {
            out_buffer[channel][sample] *= real_gain;
        }
    }
}

