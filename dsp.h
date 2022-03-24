/* date = February 23rd 2022 2:17 pm */

#ifndef FFT_H
#define FFT_H


IPP_FFT_Context ipp_initialize(Arena *allocator);
Analysis analysis_initialize(u32 ir_sample_count, u32 num_channels, Arena *allocator, IPP_FFT_Context *ipp_context);

void fft_perform_and_get_magnitude(Analysis *analysis);
void windowing_hamming(real32 *in_buffer, real32 *out_buffer, i32 sample_count);
void fft_forward(real32 *in, real32 *out_real, real32 *out_im, i32 input_sample_count, IPP_FFT_Context *ipp_context);
void fft_reverse(real32 *in_real, real32 *in_im, real32 *out, i32 input_sample_count, IPP_FFT_Context *ipp_ctx);

void phasor_32_array(real32 *out, real32 freq, i32 sample_count, real32 *phase_in_out);
void sin_32_array(real32 *out, real32 freq, i32 sample_count, real32 *phase_in_out);
void cos_32_array(real32 *out, real32 freq, i32 sample_count, real32 *phase_in_out);
void tan_32_array(real32 *out, real32 freq, i32 sample_count, real32 *phase_in_out);
void cos_32_array(real32 *out, real32 freq, i32 sample_count, real32 *phase_in_out);


void sin_32_array(real32 *out, real32 freq, i32 sample_count, real32 *phase_in_out);
void cos_32_array(real32 *out, real32 freq, i32 sample_count, real32 *phase_in_out);
void tan_32_array(real32 *out, real32 freq, i32 sample_count, real32 *phase_in_out);
void cos_32_array(real32 *out, real32 freq, i32 sample_count, real32 *phase_in_out);
void gain_32_array(real32 *in, real32 *out, real32 gain, i32 sample_count);
void sqrt_32_array(real32 *in, real32 *out, i32 sample_count);
void abs_32_array(real32 *in, real32 *out, i32 sample_count);
void to_db_32_array(real32 *in, real32 *out, i32 sample_count);
void from_db_32_array(real32 *in, real32 *out, i32 sample_count);
void pythagore_array(real32 *in_x, real32 *in_y, real32* out, i32 sample_count);
void log2_32_array(Vec2 *in, real32* out, i32 sample_count);
void log10_32_array(Vec2 *in, real32* out, i32 sample_count);

#endif //FFT_H
