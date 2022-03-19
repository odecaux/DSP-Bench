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

//TODO cette API a aucun sens ?????
Analysis analysis_initialize(Arena *allocator, u32 ir_sample_count, u32 num_channels)
{
    real32** IR_buffer = m_allocate_array(real32*, num_channels, "rename");
    real32** windowed_zero_padded_buffer = m_allocate_array(real32*, num_channels, "rename");
    
    for(u32 channel = 0; channel < num_channels; channel++)
    {
        IR_buffer[channel] = m_allocate_array(real32, ir_sample_count, "rename");
        windowed_zero_padded_buffer[channel] = m_allocate_array(real32, ir_sample_count * 4, "rename");
        memset(windowed_zero_padded_buffer[channel], 0, ir_sample_count * 4 * sizeof(real32));
    }
    
    return {
        .ir_sample_count = ir_sample_count,
        .ir_num_channels = num_channels,
        .ipp_context = ipp_initialize(allocator),
        .IR_buffer = IR_buffer,
        .windowed_zero_padded_buffer = windowed_zero_padded_buffer,
        .fft_out = m_allocate_array(Vec2, ir_sample_count * 4, "rename"),
        .magnitudes  = m_allocate_array(real32, ir_sample_count * 4, "rename")
    };
}

void fft_perform_and_get_magnitude(Analysis *analysis)
{
    
    for(u32 channel = 0; channel < analysis->ir_num_channels; channel++)
    {
        windowing_hamming(analysis->IR_buffer[channel], analysis->windowed_zero_padded_buffer[channel], IR_BUFFER_LENGTH);
    }
    
    fft_forward(analysis->windowed_zero_padded_buffer[0], 
                analysis->fft_out, 
                analysis->ir_sample_count * 4, 
                &analysis->ipp_context);
    
    for(i32 i = 0; i < IR_BUFFER_LENGTH * 4; i++)
        analysis->magnitudes[i] = sqrt(analysis->fft_out[i].a * analysis->fft_out[i].a + 
                                       analysis->fft_out[i].b * analysis->fft_out[i].b);
}
#define ipp_ensure(status) ipp_ensure_impl(status, __FILE__, __LINE__); 

void windowing_hamming(real32 *in_buffer, real32 *out_buffer, i32 sample_count)
{
    ipp_ensure(ippsWinHamming_32f(in_buffer, out_buffer, sample_count));
}

void fft_forward(real32 *in, Vec2 *out, i32 input_sample_count, Ipp_Context *ipp_ctx)
{
    ensure((input_sample_count & (input_sample_count - 1)) == 0);
    real32 r_s = log((real32)input_sample_count);
    real32 l = log(2.0);
    i32 order = (i32)(log((real32)input_sample_count)/log(2.0));
    
    Ipp_Order_Context *ctx = &ipp_ctx->order_to_ctx[order];
    ipp_ensure(ippsFFTFwd_RToCCS_32f(in, ctx->temp_perm_buffer, (IppsFFTSpec_R_32f*)ctx->spec, ipp_ctx->work_buffer));
    
    //NOTE il faut que Ipp32fc et Vec2 aient le même layout
    ipp_ensure(ippsConjCcs_32fc(ctx->temp_perm_buffer, (Ipp32fc*)out, input_sample_count));
}

Ipp_Order_Context ipp_create_spec_for_order(Arena *allocator, i32 fft_order)
{
    i32 buffer_size = 2 << fft_order;
    
    i32 spec_size;
    i32 spec_buffer_size;
    i32 work_buffer_size;
    ipp_ensure(ippsFFTGetSize_R_32f(fft_order, IPP_FFT_DIV_BY_SQRTN, ippAlgHintFast ,&spec_size, &spec_buffer_size, &work_buffer_size));
    
    u8 *spec_holder = (u8*) arena_allocate(allocator, spec_size);
    u8 *spec_initialization_buffer = (u8*) arena_allocate(allocator, spec_buffer_size);
    
    IppsFFTSpec_R_32f *spec; 
    ipp_ensure(ippsFFTInit_R_32f(&spec, fft_order, IPP_FFT_DIV_BY_SQRTN, ippAlgHintFast, spec_holder, spec_initialization_buffer));
    
    allocator->current = (char*)spec_initialization_buffer;
    
    return Ipp_Order_Context{
        .fft_order = fft_order,
        .buffer_size = buffer_size,
        .spec_holder = spec_holder,
        .spec = spec, 
        .temp_perm_buffer = (real32*) arena_allocate(allocator, sizeof(real32) * (buffer_size + 2))
    };
}

Ipp_Context ipp_initialize(Arena *allocator)
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
    ipp_ensure(ippsFFTGetSize_R_32f(MAX_FFT_ORDER, IPP_FFT_DIV_BY_SQRTN, ippAlgHintFast ,&spec_size, &spec_buffer_size, &work_buffer_size));
    
    u8 *work_buffer = (u8*) arena_allocate(allocator, work_buffer_size);
    
    
    Ipp_Order_Context *order_to_ctx = (Ipp_Order_Context*) arena_allocate(allocator, sizeof(Ipp_Order_Context) *  (MAX_FFT_ORDER + 1));
    for(i32 i = 0; i < MAX_FFT_ORDER + 1; i++)
    {
        order_to_ctx[i] = ipp_create_spec_for_order(allocator, i);
    }
    
    return Ipp_Context{
        .highest_order = MAX_FFT_ORDER,
        .work_buffer = work_buffer,
        .order_to_ctx = order_to_ctx
    };
}