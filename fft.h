/* date = February 23rd 2022 2:17 pm */

#ifndef FFT_H
#define FFT_H


void windowing_hamming(real32 *in_buffer, real32 *out_buffer, i32 sample_count);

Ipp_Context ipp_initialize();

FFT fft_initialize(u32 ir_sample_count, u32 num_channels);

void generate_tone(real32 freq, real32 *buffer, u32 sample_count);

void fft_forward(real32 *in, Vec2 *out, i32 input_sample_count, Ipp_Context *ipp_ctx);

void fft_perform(FFT *fft_ctx);

#endif //FFT_H
