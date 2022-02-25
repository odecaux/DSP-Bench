/* date = February 23rd 2022 2:17 pm */

#ifndef FFT_H
#define FFT_H

typedef struct {
    i32 fft_order;
    i32 buffer_size;
    u8* spec_holder;
    IppsFFTSpec_R_32f *spec;
    u8 *work_buffer;
    real32 *temp_perm_buffer; //TODO rename, c'est pas clair si on connait pas IPP
} Ipp_Order_Context;
//TODO rename, c'est nul comme nom ORDER, en 
typedef struct {
    i32 highest_order;
    Ipp_Order_Context *order_to_ctx;
} Ipp_Context;

Ipp_Context ipp_initialize();

void generate_tone(real32 freq, real32 *buffer, u32 sample_count);

void fft_forward(real32 *in, Vec2 *out, i32 input_sample_count, Ipp_Context *ipp_ctx);

#endif //FFT_H
