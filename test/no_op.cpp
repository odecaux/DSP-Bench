struct Parameters {};
struct State {};

typedef void*(*allocator_t)(unsigned int);


Parameters default_parameters() { return Parameters(); }
State initialize_state(Parameters& param, 
                       const unsigned int num_channels, 
                       const float sample_rate,
                       allocator_t allocator) { return State(); }


void audio_callback(const Parameters& param,
                    State& state,
                    float** out_buffer, 
                    unsigned int num_channels, 
                    unsigned int num_samples,
                    float sample_rate)
{
}
