#include "hardcoded_values.h"

#include "stdlib.h"
#include "stdio.h"
#include "math.h"

#include "ipp.h"
#include "ipps.h"
#include <ippdefs.h>

#include "base.h"
#include "fft.h"

void ipp_assert_impl(IppStatus status, const char* file, u32 line)
{
    if(status != ippStsNoErr) {
        printf("%s:%lu\n", file, line);
        printf("%s\n", ippGetStatusString(status));
        exit(1);
    }
}

#define ipp_assert(status) ipp_assert_impl(status, __FILE__, __LINE__); 

void fft_test_generate_tone(real32 frequency, real32 magnitude, real32 *buffer, i32 sample_count)
{
    real32 phase = 0.0f;
    ippsTone_32f(buffer, sample_count, magnitude, frequency, &phase, ippAlgHintFast);
}

void windowing_hamming(real32 *in_buffer, real32 *out_buffer, i32 sample_count)
{
    ipp_assert(ippsWinHamming_32f(in_buffer, out_buffer, sample_count));
}

void fft_forward(real32 *in, Vec2 *out, i32 input_sample_count, Ipp_Context *ipp_ctx)
{
    assert((input_sample_count & (input_sample_count - 1)) == 0);
    real32 r_s = log((real32)input_sample_count);
    real32 l = log(2.0);
    i32 order = (i32)(log((real32)input_sample_count)/log(2.0));
    
    Ipp_Order_Context *ctx = &ipp_ctx->order_to_ctx[order];
    ipp_assert(ippsFFTFwd_RToCCS_32f(in, ctx->temp_perm_buffer, (IppsFFTSpec_R_32f*)ctx->spec, ctx->work_buffer));
    
    //NOTE il faut que Ipp32fc et Vec2 aient le même layout
    ipp_assert(ippsConjCcs_32fc(ctx->temp_perm_buffer, (Ipp32fc*)out, input_sample_count));
}

Ipp_Order_Context ipp_create_spec_for_order(i32 fft_order)
{
    i32 buffer_size = 2 << fft_order;
    
    i32 spec_size;
    i32 spec_buffer_size;
    i32 work_buffer_size;
    ipp_assert(ippsFFTGetSize_R_32f(fft_order, IPP_FFT_DIV_BY_SQRTN, ippAlgHintFast ,&spec_size, &spec_buffer_size, &work_buffer_size));
    
    u8 *spec_holder = (u8*) malloc(spec_size);
    u8 *spec_initialization_buffer = (u8*) malloc(spec_buffer_size);
    u8 *work_buffer = (u8*) malloc(work_buffer_size);
    
    IppsFFTSpec_R_32f *spec; 
    ipp_assert(ippsFFTInit_R_32f(&spec, fft_order, IPP_FFT_DIV_BY_SQRTN, ippAlgHintFast, spec_holder, spec_initialization_buffer));
    
    
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
    
    ipp_assert(ippInit());
    ipp_assert(ippSetFlushToZero(1, 0));
    ipp_assert(ippSetDenormAreZeros(1));
    ipp_assert(ippGetCpuFeatures(&cpuFeatures, 0));/* Get CPU features and features enabled with selected library level */
    
    
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