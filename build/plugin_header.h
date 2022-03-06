/* date = December 26th 2021 11:05 pm */

#ifndef PLUGIN_HEADER_H
#define PLUGIN_HEADER_H


#define INT_ARAM(min_value, max_value) __attribute__(( annotate( "Int " #min_value " " #max_value ) )) int

#define FLOAT_ARAM(min_value, max_value) __attribute__(( annotate( "Float " #min_value " " #max_value ) )) float

#define FLOAT_ARAM_LOG(min_value, max_value) __attribute__(( annotate( "Float " #min_value " " #max_value " " "log") )) float

#define ENUM_ARAM(enum_name)  __attribute__(( annotate( "Enum" ) )) enum_name



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
}

#endif //PLUGIN_HEADER_H
