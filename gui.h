/* date = March 10th 2022 1:03 pm */

#ifndef GUI_H
#define GUI_H


#ifdef DEBUG 
extern "C" __declspec(dllexport)
#endif

void frame(Plugin_Descriptor& descriptor, 
           Graphics_Context *graphics_ctx, 
           UI_State& ui_state, 
           IO frame_io, 
           Plugin_Parameter_Value* current_parameter_values,
           Audio_Thread_Context *audio_ctx,
           Compiler_Gui_Log *error_log,
           bool *parameters_were_tweaked,
           bool *load_was_clicked,
           bool *load_plugin_was_clicked);


#endif //GUI_H
