#include "hardcoded_values.h"

#include "stdlib.h"
#include "stdio.h"
#include "math.h"
#include "string.h"

#include "ipp.h"
#include "ipps.h"
#include <ippdefs.h>

#include "memory.h"
#include "base.h"
#include "structs.h"
#include "fft.h"

void ipp_octave_assert_impl(IppStatus status, const char* file, u32 line)
{
    if(status != ippStsNoErr) {
        printf("%s:%lu\n", file, line);
        printf("%s\n", ippGetStatusString(status));
        exit(1);
    }
}

FFT fft_initialize(u32 ir_sample_count, u32 num_channels)
{
    real32** IR_buffer = m_allocate_array(real32*, num_channels);
    real32** windowed_zero_padded_buffer = m_allocate_array(real32*, num_channels);
    
    for(u32 channel = 0; channel < num_channels; channel++)
    {
        IR_buffer[channel] = m_allocate_array(real32, ir_sample_count);
        windowed_zero_padded_buffer[channel] = m_allocate_array(real32, ir_sample_count * 4);
        memset(windowed_zero_padded_buffer[channel], 0, ir_sample_count * 4 * sizeof(real32));
    }
    
    return {
        .ir_sample_count = ir_sample_count,
        .ir_num_channels = num_channels,
        .ipp_context = ipp_initialize(),
        .IR_buffer = IR_buffer,
        .windowed_zero_padded_buffer = windowed_zero_padded_buffer,
        .fft_out = m_allocate_array(Vec2, ir_sample_count * 4),
        .magnitudes  = m_allocate_array(real32, ir_sample_count * 4)
    };
}

void fft_perform(FFT *fft)
{
    
    for(u32 channel = 0; channel < fft->ir_num_channels; channel++)
    {
        windowing_hamming(fft->IR_buffer[channel], fft->windowed_zero_padded_buffer[channel], IR_BUFFER_LENGTH);
    }
    
    fft_forward(fft->windowed_zero_padded_buffer[0], 
                fft->fft_out, 
                fft->ir_sample_count * 4, 
                &fft->ipp_context);
    
    for(i32 i = 0; i < IR_BUFFER_LENGTH * 4; i++)
        fft->magnitudes[i] = sqrt(fft->fft_out[i].a * fft->fft_out[i].a + 
                                  fft->fft_out[i].b * fft->fft_out[i].b);
}
#define ipp_octave_assert(status) ipp_octave_assert_impl(status, __FILE__, __LINE__); 

void fft_test_generate_tone(real32 frequency, real32 magnitude, real32 *buffer, i32 sample_count)
{
    real32 phase = 0.0f;
    ippsTone_32f(buffer, sample_count, magnitude, frequency, &phase, ippAlgHintFast);
}

void windowing_hamming(real32 *in_buffer, real32 *out_buffer, i32 sample_count)
{
    ipp_octave_assert(ippsWinHamming_32f(in_buffer, out_buffer, sample_count));
}

void fft_forward(real32 *in, Vec2 *out, i32 input_sample_count, Ipp_Context *ipp_ctx)
{
    octave_assert((input_sample_count & (input_sample_count - 1)) == 0);
    real32 r_s = log((real32)input_sample_count);
    real32 l = log(2.0);
    i32 order = (i32)(log((real32)input_sample_count)/log(2.0));
    
    Ipp_Order_Context *ctx = &ipp_ctx->order_to_ctx[order];
    ipp_octave_assert(ippsFFTFwd_RToCCS_32f(in, ctx->temp_perm_buffer, (IppsFFTSpec_R_32f*)ctx->spec, ctx->work_buffer));
    
    //NOTE il faut que Ipp32fc et Vec2 aient le même layout
    ipp_octave_assert(ippsConjCcs_32fc(ctx->temp_perm_buffer, (Ipp32fc*)out, input_sample_count));
}

Ipp_Order_Context ipp_create_spec_for_order(i32 fft_order)
{
    i32 buffer_size = 2 << fft_order;
    
    i32 spec_size;
    i32 spec_buffer_size;
    i32 work_buffer_size;
    ipp_octave_assert(ippsFFTGetSize_R_32f(fft_order, IPP_FFT_DIV_BY_SQRTN, ippAlgHintFast ,&spec_size, &spec_buffer_size, &work_buffer_size));
    
    u8 *spec_holder = (u8*) malloc(spec_size);
    u8 *spec_initialization_buffer = (u8*) malloc(spec_buffer_size);
    u8 *work_buffer = (u8*) malloc(work_buffer_size);
    
    IppsFFTSpec_R_32f *spec; 
    ipp_octave_assert(ippsFFTInit_R_32f(&spec, fft_order, IPP_FFT_DIV_BY_SQRTN, ippAlgHintFast, spec_holder, spec_initialization_buffer));
    
    
    free(spec_initialization_buffer);
    
    return Ipp_Order_Context{
        .fft_order = fft_order,
        .buffer_size = buffer_size,
        .spec_holder = spec_holder,
        .spec = spec, 
        .work_buffer = work_buffer,
        .temp_perm_buffer = (real32*) malloc(sizeof(real32) * (buffer_size + 2))
    };
}

Ipp_Context ipp_initialize()
{
    IppLibraryVersion *libVersion;
    u64 cpuFeatures;
    u64 enabledFeatures;
    
    ipp_octave_assert(ippInit());
    ipp_octave_assert(ippSetFlushToZero(1, 0));
    ipp_octave_assert(ippSetDenormAreZeros(1));
    ipp_octave_assert(ippGetCpuFeatures(&cpuFeatures, 0));/* Get CPU features and features enabled with selected library level */
    
    
    Ipp_Order_Context *order_to_ctx = (Ipp_Order_Context*) malloc( sizeof(Ipp_Order_Context) *  (MAX_FFT_ORDER + 1));
    for(i32 i = 0; i < MAX_FFT_ORDER + 1; i++)
    {
        order_to_ctx[i] = ipp_create_spec_for_order(i);
    }
    
    return Ipp_Context{
        .highest_order = MAX_FFT_ORDER,
        .order_to_ctx = order_to_ctx
    };
}