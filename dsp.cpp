#include "hardcoded_values.h"

#include "stdlib.h"
#include "stdio.h"
#include "math.h"
#include "string.h"

#include <ippdefs.h>
#include <ipp.h>
#include <ipps.h>

#include "base.h"
#include "structs.h"
#include "memory.h"
#include "dsp.h"



void ipp_ensure_impl(IppStatus status, const char* file, u32 line)
{
    if(status != ippStsNoErr) {
        printf("%s:%lu\n", file, line);
        printf("%s\n", ippGetStatusString(status));
        exit(1);
    }
}


Analysis analysis_initialize(u32 ir_sample_count, u32 num_channels, Arena *allocator, IPP_FFT_Context *ipp_context)
{
    real32** IR_buffer = (real32**)arena_allocate(allocator, sizeof(real32*) *num_channels);
    real32** windowed_zero_padded_buffer = (real32**)arena_allocate(allocator, sizeof(real32*) * num_channels);
    
    for(u32 channel = 0; channel < num_channels; channel++)
    {
        IR_buffer[channel] = (real32*)arena_allocate(allocator, sizeof(real32) * ir_sample_count);
        windowed_zero_padded_buffer[channel] = (real32*)arena_allocate(allocator, sizeof(real32) * ir_sample_count * 4);
        memset(windowed_zero_padded_buffer[channel], 0, ir_sample_count * 4 * sizeof(real32));
    }
    Analysis analysis {
        .ir_sample_count = ir_sample_count,
        .ir_num_channels = num_channels,
        .ipp_context = ipp_context,
        .IR_buffer = IR_buffer,
        .windowed_zero_padded_buffer = windowed_zero_padded_buffer,
        .fft_out_real = (real32*)arena_allocate(allocator, sizeof(real32) * ir_sample_count * 4),
        .fft_out_im = (real32*)arena_allocate(allocator, sizeof(real32) * ir_sample_count * 4),
        .magnitudes  = (real32*)arena_allocate(allocator, sizeof(real32) * ir_sample_count * 4)
    };
    return analysis;
}

void fft_perform_and_get_magnitude(Analysis *analysis)
{
    
    for(u32 channel = 0; channel < analysis->ir_num_channels; channel++)
    {
        windowing_hamming(analysis->IR_buffer[channel], analysis->windowed_zero_padded_buffer[channel], IR_BUFFER_LENGTH);
    }
    
    fft_forward(analysis->windowed_zero_padded_buffer[0], 
                analysis->fft_out_real, analysis->fft_out_im, 
                analysis->ir_sample_count * 4, 
                analysis->ipp_context);
    pythagore_array(analysis->fft_out_real, analysis->fft_out_im, analysis->magnitudes, IR_BUFFER_LENGTH * 4);
}
#define ipp_ensure(status) ipp_ensure_impl(status, __FILE__, __LINE__); 

void windowing_hamming(real32 *in_buffer, real32 *out_buffer, i32 sample_count)
{
    ipp_ensure(ippsWinHamming_32f(in_buffer, out_buffer, sample_count));
}

void fft_forward(real32 *in, 
                 real32 *out_real, real32 *out_im, 
                 i32 input_sample_count, 
                 IPP_FFT_Context *ipp_ctx)
{
    ensure((input_sample_count & (input_sample_count - 1)) == 0);
    real32 r_s = log((real32)input_sample_count);
    real32 l = log(2.0);
    i32 order = (i32)(log((real32)input_sample_count)/log(2.0));
    
    if(order != ipp_ctx->current_order)
    {
        ensure(order <= MAX_FFT_ORDER);
        ipp_ensure(ippsFFTInit_C_32f((IppsFFTSpec_C_32f**)&ipp_ctx->spec, 
                                     order, 
                                     IPP_FFT_DIV_BY_SQRTN, 
                                     ippAlgHintFast, 
                                     ipp_ctx->spec_holder, 
                                     ipp_ctx->spec_initialization_buffer));
        
        ipp_ctx->current_order = order;
    }
    
    memset(ipp_ctx->temp_im_buffer, 0, sizeof(real32) * input_sample_count); 
    
    ipp_ensure(ippsFFTFwd_CToC_32f(in, ipp_ctx->temp_im_buffer,
                                   out_real, out_im,
                                   (IppsFFTSpec_C_32f*)ipp_ctx->spec, 
                                   ipp_ctx->work_buffer));
}


void fft_reverse(real32 *in_real, real32 *in_im, real32 *out, i32 input_sample_count, IPP_FFT_Context *ipp_ctx)
{
    ensure((input_sample_count & (input_sample_count - 1)) == 0);
    real32 r_s = log((real32)input_sample_count);
    real32 l = log(2.0);
    i32 order = (i32)(log((real32)input_sample_count)/log(2.0));
    
    if(order != ipp_ctx->current_order)
    {
        ensure(order <= MAX_FFT_ORDER);
        ipp_ensure(ippsFFTInit_C_32f((IppsFFTSpec_C_32f**)&ipp_ctx->spec, 
                                     order, 
                                     IPP_FFT_DIV_BY_SQRTN, 
                                     ippAlgHintFast, 
                                     ipp_ctx->spec_holder, 
                                     ipp_ctx->spec_initialization_buffer));
        
        ipp_ctx->current_order = order;
    }
    
    ipp_ensure(ippsFFTInv_CToC_32f(in_real, in_im,
                                   out, ipp_ctx->temp_im_buffer,
                                   (IppsFFTSpec_C_32f*)ipp_ctx->spec, 
                                   ipp_ctx->work_buffer));
    
    //pythagore_array(ipp_ctx->temp_real_buffer, ipp_ctx->temp_im_buffer, out, input_sample_count);
}

IPP_FFT_Context ipp_initialize(Arena *allocator)
{
    IppLibraryVersion *libVersion;
    u64 cpuFeatures;
    u64 enabledFeatures;
    
    ipp_ensure(ippInit());
    ipp_ensure(ippSetFlushToZero(1, 0));
    ipp_ensure(ippSetDenormAreZeros(1));
    ipp_ensure(ippGetCpuFeatures(&cpuFeatures, 0));/* Get CPU features and features enabled with selected library level */
    
    i32 spec_size;
    i32 spec_buffer_size;
    i32 work_buffer_size;
    ipp_ensure(ippsFFTGetSize_C_32f(MAX_FFT_ORDER, IPP_FFT_DIV_BY_SQRTN, ippAlgHintFast ,&spec_size, &spec_buffer_size, &work_buffer_size));
    
    u8 *work_buffer = (u8*) arena_allocate(allocator, work_buffer_size);
    u8 *spec_holder = (u8*) arena_allocate(allocator, spec_size);
    u8 *spec_initialization_buffer = (u8*) arena_allocate(allocator, spec_buffer_size);
    real32 *temp_im_buffer = (real32*) arena_allocate(allocator, sizeof(real32) * ((2 << MAX_FFT_ORDER)));
    real32 *temp_real_buffer = (real32*) arena_allocate(allocator, sizeof(real32) * ((2 << MAX_FFT_ORDER)));
    
    return {
        .current_order = -1,
        .work_buffer = work_buffer,
        .temp_real_buffer = temp_real_buffer,
        .temp_im_buffer = temp_im_buffer,
        .spec_holder = spec_holder,
        .spec_initialization_buffer = spec_initialization_buffer,
        .spec = nullptr
    };
}
void pythagore_array(real32 *in_x, real32 *in_y, real32* out, i32 sample_count){
    ippsMagnitude_32f(in_x, in_y, out, sample_count);
}


void copy_array(real32 *in, real32 *out, i32 sample_count){
    ipp_ensure(ippsCopy_32f(in, out, sample_count));
}

void set_array(real32 val, real32 *out, i32 sample_count){
    ipp_ensure(ippsSet_32f(val, out, sample_count));
}

void zero_array(real32 *out, i32 sample_count){
    ipp_ensure(ippsZero_32f(out, sample_count));
}



void phasor_32_array(real32 *out, real32 ampl, real32 freq, i32 sample_count, real32 *phase_in_out){
    if(*phase_in_out == 0.0f)
    {
        //NOTE est-ce que ça va faire un click qq part ?
        *phase_in_out += 0.000005f;
    }
    ipp_ensure(ippsTriangle_32f(out, sample_count, ampl, freq, - M_PI + 0.00000004f, phase_in_out));
}

void sin_32_array(real32 *out, real32 ampl, real32 freq, i32 sample_count, real32 *phase_in_out){
    ipp_ensure(ippsTone_32f(out, sample_count, ampl, freq, phase_in_out, ippAlgHintFast));
}

void triangle_32_array(real32 *out, real32 ampl, real32 freq, i32 sample_count, real32 *phase_in_out){
    ipp_ensure(ippsTriangle_32f(out, sample_count, ampl, freq, 0.0f, phase_in_out));
}


void random_uniform_32_array(real32 *out, i32 sample_count, void *rng){
    //ipp_ensure(ippsRandUniform_32f(out, sample_count, ampl, freq, phase_in_out, ippAlgHintFast));
}


void gain_32_array(real32 *in, real32 *out, real32 gain, i32 sample_count){
    ippsMulC_32f(in, gain, out, sample_count);
}
void dc_offset_32_array(real32 *in, real32 *out, real32 offset, i32 sample_count){
    ippsAddC_32f(in, offset, out, sample_count);
}
void sqrt_32_array(real32 *in, real32 *out, i32 sample_count){
    ippsSqrt_32f(in, out, sample_count);
}

void abs_32_array(real32 *in, real32 *out, i32 sample_count){
    ippsAbs_32f(in, out, sample_count);
}

void ln_32_array(real32 *in, real32 *out, i32 sample_count){
    ippsLn_32f(in, out,sample_count);
}

void log2_32_array(real32 *in, real32 *out, i32 sample_count){
    ippsLn_32f(in, out, sample_count);
    ippsMulC_32f_I(1.0f / (real32)log(2.0), out, sample_count);
}

void log10_32_array(real32 *in, real32 *out, i32 sample_count){
    ippsLn_32f(in, out, sample_count);
    ippsMulC_32f_I(1.0f / (real32)log(10.0), out, sample_count);
}

void to_db_32_array(real32 *in, real32 *out, i32 sample_count){
    ippsLn_32f(in, out, sample_count);
    ippsMulC_32f_I(20.0f / (real32)log(10.0), out, sample_count);
}

void from_db_32_array(real32 *in, real32 *out, i32 sample_count){
    ippsMulC_32f(in, (real32)log(10.0) / 20.0f, out, sample_count);
    ippsExp_32f_I(out, sample_count);
}



void gain_ip_32_array(real32 *in_out, real32 gain, i32 sample_count){
    ippsMulC_32f_I(gain, in_out, sample_count);
}
void dc_offset_ip_32_array(real32 *in_out, real32 offset, i32 sample_count){
    ippsAddC_32f_I(offset, in_out, sample_count);
}
void sqrt_ip_32_array(real32 *in_out, i32 sample_count){
    ippsSqrt_32f_I(in_out, sample_count);
}

void abs_ip_32_array(real32 *in_out, i32 sample_count){
    ippsAbs_32f_I(in_out, sample_count);
}

void ln_ip_32_array(real32 *in_out, i32 sample_count){
    ippsLn_32f_I(in_out,sample_count);
}

void log2_ip_32_array(real32 *in_out, i32 sample_count){
    ippsLn_32f_I(in_out, sample_count);
    ippsMulC_32f_I(1.0f / (real32)log(2.0), in_out, sample_count);
}

void log10_ip_32_array(real32 *in_out, i32 sample_count){
    ippsLn_32f_I(in_out, sample_count);
    ippsMulC_32f_I(1.0f / (real32)log(10.0), in_out, sample_count);
}
