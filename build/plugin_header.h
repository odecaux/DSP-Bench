/* date = December 26th 2021 11:05 pm */

#ifndef PLUGIN_HEADER_H
#define PLUGIN_HEADER_H


#define INT_PARAM(min_value, max_value) __attribute__(( annotate( "Int " #min_value " " #max_value ) )) int

#define FLOAT_PARAM(min_value, max_value) __attribute__(( annotate( "Float " #min_value " " #max_value ) )) float

#define FLOAT_PARAM_LOG(min_value, max_value) __attribute__(( annotate( "Float " #min_value " " #max_value " " "log") )) float

#define ENUM_PARAM(enum_name)  __attribute__(( annotate( "Enum" ) )) enum_name


typedef float real32;
typedef unsigned int u32;
typedef int i32;

typedef struct {
    union{
        struct {
            real32 x;
            real32 y;
        };
        struct {
            real32 a;
            real32 b;
        };
    };
} Vec2;

const float inv_two_pi = 0.1591549f;
const float two_pi = 6.283185f;
const float three_half_pi = 4.7123889f;
const float pi = 3.141593f;
const float half_pi = 1.570796f;
const float quarter_pi = 0.7853982f;

extern "C" { 
    
    float *allocate_buffer(int num_sample, void* allocator);
    float **allocate_buffers(int num_samples, int num_channels, void* allocator);
    void *allocate_bytes(int num_bytes, void* allocator);
    
    double sin_64(double d1);
    double cos_64(double d1);
    double tan_64(double d1);
    double fabs_64(double d1);
    double pow_64(double d1, double d2);
    double fmod_64(double d1, double d2);
    double ceil_64(double d1);
    double floor_64(double d1);
    double sqrt_64(double d1);
    double exp_64(double d1);
    double log10_64(double d1);
    double log_64(double d1);
    double asin_64(double d1); 
    double acos_64(double d1);
    double atan_64(double d1);
    double atan2_64(double d1, double d2);
    double sinh_64(double d1);
    double cosh_64(double d1);
    double tanh_64(double d1);
    
    
    float sin_32(float d1);
    float cos_32(float d1);
    float tan_32(float d1);
    float fabs_32(float d1);
    float pow_32(float d1, float d2);
    float fmod_32(float d1, float d2);
    float ceil_32(float d1);
    float floor_32(float d1);
    float sqrt_32(float d1);
    float exp_32(float d1);
    float log10_32(float d1);
    float log_32(float d1);
    float asin_32(float d1);
    float acos_32(float d1);
    float atan_32(float d1);
    float atan2_32(float d1, float d2);
    float sinh_32(float d1);
    float cosh_32(float d1);
    float tanh_32(float d1);
    
    void phasor_32_array(real32 *out, real32 freq, i32 sample_count, real32 *phase_in_out);
    void sin_32_array(real32 *out, real32 freq, i32 sample_count, real32 *phase_in_out);
    void cos_32_array(real32 *out, real32 freq, i32 sample_count, real32 *phase_in_out);
    void tan_32_array(real32 *out, real32 freq, i32 sample_count, real32 *phase_in_out);
    void cos_32_array(real32 *out, real32 freq, i32 sample_count, real32 *phase_in_out);
    
    void gain_32_array(real32 *in, real32 *out, real32 gain, i32 sample_count);
    void sqrt_32_array(real32 *in, real32 *out, i32 sample_count);
    void abs_32_array(real32 *in, real32 *out, i32 sample_count);
    void to_db_32_array(real32 *in, real32 *out, i32 sample_count);
    void from_db_32_array(real32 *in, real32 *out, i32 sample_count);
    void pythagore_array(Vec2 *in, real32* out, i32 sample_count);
    void log2_32_array(Vec2 *in, real32* out, i32 sample_count);
    void log10_32_array(Vec2 *in, real32* out, i32 sample_count);
    
    
    void* initialize_fft();
    void windowing_hamming(real32 *in_buffer, real32 *out_buffer, i32 sample_count);
    void fft_forward(real32 *in, Vec2 *out, i32 input_sample_count, void *fft_context);
}

#endif //PLUGIN_HEADER_H
