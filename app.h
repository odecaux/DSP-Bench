/* date = February 23rd 2022 11:14 am */

#ifndef APP_H
#define APP_H

IO io_initial_state();

IO io_state_advance(const IO old_io);

void frame(Plugin_Descriptor& descriptor, 
           Graphics_Context *graphics_ctx, 
           UI_State& ui_state, 
           IO frame_io, 
           //TODO frame_io should not be a reference,
           //that's a hack : open_file_prompt steals the wm_mouse_up message 
           Plugin_Parameter_Value* current_parameter_values,
           Audio_Context *audio_ctx,
           bool *parameters_were_tweaked,
           bool *load_was_clicked);

void compute_IR(Plugin_Handle& handle, 
                real32** IR_buffer, 
                u32 IR_length, 
                Audio_Parameters& audio_parameters,
                Plugin_Parameter_Value* current_parameters_values);

void integrate_fft(real32* magnitude_buffer, u32 sample_count, real32* pixel_buffer, u32 pixel_count);

#endif //APP_H
