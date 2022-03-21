/* date = February 23rd 2022 11:14 am */

#ifndef APP_H
#define APP_H

IO io_initial_state();

IO io_state_advance(const IO old_io);

void compute_IR(Plugin& handle, 
                real32** IR_buffer, 
                u32 IR_length, 
                Audio_Parameters& audio_parameters,
                Plugin_Parameter_Value* current_parameters_values,
                Arena *scratch_allocator,
                Initializer *initializer);

void integrate_fft(real32* magnitude_buffer, u32 sample_count, real32* pixel_buffer, u32 pixel_count);

#endif //APP_H
