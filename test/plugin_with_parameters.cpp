
#define INT_ARAM(min_value, max_value) __attribute__(( annotate( "Int " #min_value " " #max_value ) )) int

#define FLOAT_ARAM(min_value, max_value) __attribute__(( annotate( "Float " #min_value " " #max_value ) )) float

#define ENUM_ARAM(enum_name)  __attribute__(( annotate( "Enum" ) )) enum_name


enum random_enum{
    A, B, C, D
};

// TODO(octave): courbe exponentielle, power factor

typedef void*(*allocator_t)(unsigned int);

struct Parameters{
    INT_ARAM(0, 4) ish;
    FLOAT_ARAM(0.0f, 1.0f) gain;
    ENUM_ARAM(random_enum) truc;
};

struct State {
    float gain;
};

typedef void*(*allocator_t)(unsigned int);


Parameters default_parameters() { 
    Parameters ish = {0, 0.9f, A}; 
    return ish; 
}
State initialize_state(Parameters& param, 
                       const unsigned int num_channels, 
                       const float sample_rate,
                       void *allocator) {
    State state = {0.1f};
    return state; 
}


void audio_callback(const Parameters& param,
                    State& state,
                    float** out_buffer, 
                    const unsigned int num_channels, 
                    const unsigned int num_samples,
                    const float sample_rate)
{
}