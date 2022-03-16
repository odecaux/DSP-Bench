#include "math.h"
#include "windows.h"
#include "stdio.h"

#include "base.h"
#include "structs.h"
#include "memory.h"
#include "plugin.h"

#include "hardcoded_values.h"
#include "draw.h"


struct UI_Context{
    IO io;
    UI_State *state;
    Graphics_Context *g;
};

//~ Widgets

real32 simple_slider(real32 normalized_value, i32 id, 
                     Rect bounds, UI_Context ui)
{
    
    draw_rectangle(bounds, 1.0f, Color_Front, ui.g);
    draw_slider(bounds, normalized_value, ui.g);
    
    if(ui.io.mouse_clicked && rect_contains(bounds, ui.io.mouse_position))
    {
        octave_assert(ui.state->selected_parameter_id == -1);
        ui.state->selected_parameter_id = id;
    }
    
    bool dragging = ui.io.mouse_down && (ui.io.mouse_delta.x != 0.0f
                                         || ui.io.mouse_delta.y != 0.0f);
    
    if(ui.state->selected_parameter_id == id && ui.io.mouse_clicked)
    {
        real32 mouse_x = ui.io.mouse_position.x;
        real32 normalized_mouse_value = (mouse_x - bounds.origin.x - (SLIDER_WIDTH / 2)) / (bounds.dim.x - SLIDER_WIDTH);
        
        return octave_clamp(normalized_mouse_value, 0.0f, 1.0f);
    }
    else if(ui.state->selected_parameter_id == id && dragging)
    {
        
        real32 mouse_delta_x = (ui.io.left_ctrl_down) ? ui.io.mouse_delta.x / 4 : ui.io.mouse_delta.x;
        real32 normalized_delta = (mouse_delta_x) / (bounds.dim.x - SLIDER_WIDTH);
        return octave_clamp(normalized_delta + normalized_value, 0.0f, 1.0f);
    }
    else
    {
        return normalized_value;
    }
}

real32 slider(real32 normalized_value, 
              i32 id, 
              String title, 
              String current_value_label, 
              String min_label, 
              String max_label, 
              Rect bounds, 
              bool use_relative,
              UI_Context ui)
{
    Rect title_bounds = rect_take_top(bounds, FIELD_TITLE_HEIGHT);
    Rect current_value_bounds = rect_shrinked(rect_drop_top(bounds, FIELD_TITLE_HEIGHT), 2.5f, 2.5f);
    Rect slider_and_minmax_bounds = rect_move_by(current_value_bounds, {0.0f, FIELD_TITLE_HEIGHT});
    
    draw_text(title, title_bounds, Color_Front, ui.g);
    
    Rect slider_bounds = rect_shrinked(slider_and_minmax_bounds, MIN_MAX_LABEL_WIDTH, 0.0f);
    Rect min_label_bounds = rect_take_left(slider_and_minmax_bounds, MIN_MAX_LABEL_WIDTH);
    Rect max_label_bounds = rect_take_right(slider_and_minmax_bounds, MIN_MAX_LABEL_WIDTH);
    
    draw_rectangle(slider_bounds, 1.0f, Color_Front, ui.g);
    draw_rectangle(min_label_bounds, 1.0f, Color_Front, ui.g);
    draw_rectangle(max_label_bounds, 1.0f, Color_Front, ui.g);
    
    draw_text(min_label, min_label_bounds, Color_Front, ui.g);
    draw_text(max_label, max_label_bounds, Color_Front, ui.g);
    draw_text(current_value_label, current_value_bounds, Color_Front, ui.g);
    
    draw_slider(slider_bounds, normalized_value, ui.g);
    
    if(ui.io.mouse_clicked && rect_contains(slider_bounds, ui.io.mouse_position))
    {
        octave_assert(ui.state->selected_parameter_id == -1);
        ui.state->selected_parameter_id = id;
    }
    
    bool dragging = ui.io.mouse_down && (ui.io.mouse_delta.x != 0.0f
                                         || ui.io.mouse_delta.y != 0.0f);
    
    real32 mouse_x = ui.io.mouse_position.x;
    real32 normalized_mouse_x = (mouse_x - slider_bounds.origin.x - (SLIDER_WIDTH / 2)) / (slider_bounds.dim.x - SLIDER_WIDTH);
    
    real32 mouse_delta_x = (ui.io.left_ctrl_down) ? ui.io.mouse_delta.x / 4 : ui.io.mouse_delta.x;
    real32 normalized_delta = (mouse_delta_x) / (slider_bounds.dim.x - SLIDER_WIDTH);
    
    if(ui.state->selected_parameter_id == id && ui.io.mouse_clicked)
    {
        
        return octave_clamp(normalized_mouse_x, 0.0f, 1.0f);
    }
    else if(ui.state->selected_parameter_id == id && dragging)
    {
        if(use_relative)
        {
            return octave_clamp(normalized_delta + normalized_value, 0.0f, 1.0f);
        }
        else 
        {
            
            return octave_clamp(normalized_mouse_x, 0.0f, 1.0f);
        }
    }
    else
    {
        return normalized_value;
    }
}



bool button(Rect bounds, 
            String text, 
            u32 id, 
            UI_Context ui)
{
    Rect outline_bounds = rect_shrinked(bounds, 2.0f, 2.0f);
    
    bool hovered = rect_contains(bounds,ui.io.mouse_position); 
    bool clicked = false;
    if(hovered && ui.io.mouse_clicked)
    {
        clicked = true;
        ui.state->selected_parameter_id = id;
    }
    bool down = ui.state->selected_parameter_id == id;
    
    Rect text_bounds = rect_shrinked(bounds, 10.0f, 10.0f);
    if(down)
    {
        fill_rectangle(bounds, Color_Front, ui.g); 
        draw_rectangle(outline_bounds, 3.0f, 0xff000000, ui.g);
        draw_text(text, text_bounds, 0xff000000, ui.g);
    }
    else if(hovered)
    {
        fill_rectangle(bounds, 0xff000000, ui.g); 
        draw_rectangle(outline_bounds, 3.0f, Color_Front, ui.g);
        draw_text(text, text_bounds, Color_Front, ui.g);
    }
    else
    {
        fill_rectangle(bounds, 0xff000000, ui.g); 
        draw_text(text, text_bounds, Color_Front, ui.g);
    }
    
    draw_rectangle(bounds, 2.0f, Color_Front, ui.g);
    return clicked;
}


bool toggle(Rect bounds, 
            String text, 
            u32 id, 
            UI_Context ui,
            bool *v)
{
    Rect outline_bounds = rect_shrinked(bounds, 2.0f, 2.0f);
    
    bool hovered = rect_contains(bounds,ui.io.mouse_position); 
    bool clicked = false;
    if(hovered && ui.io.mouse_clicked)
    {
        clicked = true;
        octave_assert(ui.state->selected_parameter_id == -1);
        ui.state->selected_parameter_id = id;
    }
    
    bool down = ui.state->selected_parameter_id == id;
    
    Rect text_bounds = rect_shrinked(bounds, 10.0f, 10.0f);
    
    if(clicked)
        *v = !(*v);
    
    if(*v)
    {
        fill_rectangle(bounds, Color_Front, ui.g); 
        draw_rectangle(outline_bounds, 3.0f, 0xff000000, ui.g);
        draw_text(text, text_bounds, 0xff000000, ui.g);
    }
    else if(hovered)
    {
        fill_rectangle(bounds, 0xff000000, ui.g); 
        draw_rectangle(outline_bounds, 3.0f, Color_Front, ui.g);
        draw_text(text, text_bounds, Color_Front, ui.g);
    }
    else
    {
        fill_rectangle(bounds, 0xff000000, ui.g); 
        draw_text(text, text_bounds, Color_Front, ui.g);
    }
    
    draw_rectangle(bounds, 2.0f, Color_Front, ui.g);
    
    return clicked;
}

void parameter_slider(u32 parameter_idx, Plugin_Descriptor_Parameter *parameter_descriptor, Plugin_Parameter_Value *current_parameter_value, Rect parameter_bounds, UI_Context ui, bool *parameters_were_tweaked)
{
    switch(parameter_descriptor->type)
    {
        case Int :
        {
            int current_value = current_parameter_value->int_value; 
            char current_value_text[256];
            int text_size = sprintf(current_value_text, "%d", current_value);
            octave_assert(text_size >= 0);
            String current_value_label = {.str = current_value_text, .size = (u64)text_size};
            
            real32 current_normalized_value = normalize_parameter_int_value(parameter_descriptor->int_param, current_value);
            
            
            i32 min_value = parameter_descriptor->int_param.min;
            char min_value_text[256];
            text_size = sprintf(min_value_text, "%d", min_value);
            octave_assert(text_size >= 0);
            String min_label = {.str = min_value_text, .size = (u64)text_size};
            
            i32 max_value = parameter_descriptor->int_param.max;
            char max_value_text[256];
            text_size = sprintf(max_value_text, "%d", max_value);
            octave_assert(text_size >= 0);
            String max_label = {.str = max_value_text, .size = (u64)text_size};
            
            real32 new_normalized_value =
                slider(current_normalized_value, parameter_idx, 
                       parameter_descriptor->name, current_value_label, min_label, max_label, 
                       parameter_bounds, 
                       true,
                       ui);
            
            if(new_normalized_value != current_normalized_value )
            {
                *parameters_were_tweaked = true;
                auto new_int_value = denormalize_int_value(parameter_descriptor->int_param, new_normalized_value);
                current_parameter_value->int_value = new_int_value;
            }
            
        }break;
        case Float : 
        {
            real32 current_value = current_parameter_value->float_value; 
            char current_value_text[256];
            int text_size = sprintf(current_value_text, "%.3f", current_value);
            octave_assert(text_size >= 0);
            String current_value_label = {.str = current_value_text, .size = (u64)text_size};
            
            real32 current_normalized_value = normalize_parameter_float_value(parameter_descriptor->float_param, current_value);
            
            real32 min_value = parameter_descriptor->float_param.min;
            char min_value_text[256];
            text_size = sprintf(min_value_text, "%.3f", min_value);
            octave_assert(text_size >= 0);
            String min_label = {.str = min_value_text, .size = (u64)text_size};
            
            real32 max_value = parameter_descriptor->float_param.max;
            char max_value_text[256];
            text_size = sprintf(max_value_text, "%.3f", max_value);
            octave_assert(text_size >= 0);
            String max_label = {.str = max_value_text, .size = (u64)text_size};
            
            real32 new_normalized_value =
                slider(current_normalized_value, parameter_idx, 
                       parameter_descriptor->name, current_value_label, min_label, max_label, 
                       parameter_bounds, 
                       true,
                       ui);
            
            if(new_normalized_value != current_normalized_value)
            {
                *parameters_were_tweaked = true;
                auto new_float_value = denormalize_float_value(parameter_descriptor->float_param, new_normalized_value);
                current_parameter_value->float_value = new_float_value;
            }
            
        }break;
        case Enum : 
        {
            i32 index = current_parameter_value->enum_value;
            real32 current_normalized_value = normalize_parameter_enum_index(parameter_descriptor->enum_param, index);
            Parameter_Enum_Entry value = parameter_descriptor->enum_param.entries[index];
            
            real32 new_normalized_value =
                slider(current_normalized_value, parameter_idx, 
                       parameter_descriptor->name, value.name, {}, {}, 
                       parameter_bounds, false,
                       ui);
            
            if(new_normalized_value != current_normalized_value)
            {
                *parameters_were_tweaked = true;
                auto new_index = denormalize_enum_index(parameter_descriptor->enum_param, new_normalized_value);
                auto new_value = enum_index_to_value(parameter_descriptor->enum_param, new_index);
                current_parameter_value->enum_value = new_value;
            }
            
        }break;
    }
}


//~ Panels

void header(Asset_File_State plugin_state, 
            String plugin_name,
            Rect header_bounds, 
            UI_Context ui,
            bool *load_plugin_was_clicked,
            bool *mute_plugin_was_clicked)
{
    switch(plugin_state)
    {
        case Asset_File_State_IN_USE:
        case Asset_File_State_HOT_RELOAD_CHECK_FILE_FOR_UPDATE :
        case Asset_File_State_HOT_RELOAD_STAGE_BACKGROUND_LOADING :
        case Asset_File_State_HOT_RELOAD_BACKGROUND_LOADING :
        case Asset_File_State_HOT_RELOAD_STAGE_VALIDATION :
        case Asset_File_State_HOT_RELOAD_VALIDATING :
        case Asset_File_State_HOT_RELOAD_STAGE_SWAP :
        case Asset_File_State_HOT_RELOAD_SWAPPING :
        case Asset_File_State_HOT_RELOAD_STAGE_DISPOSE :
        case Asset_File_State_HOT_RELOAD_DISPOSING :
        {
            Rect title_bounds = rect_drop_right(header_bounds,  TITLE_HEIGHT);
            
            if(plugin_state == Asset_File_State_IN_USE)
            {
                Rect load_plugin_button_bounds = rect_take_left(title_bounds, TITLE_HEIGHT * 3);
                title_bounds = rect_drop_left(title_bounds, TITLE_HEIGHT * 3);
                if(button(load_plugin_button_bounds, StringLit("Load Plugin"), 1024, ui))
                {
                    *load_plugin_was_clicked = true;
                }
            }
            
            draw_rectangle(title_bounds, 1.0f, Color_Front, ui.g);
            
            
            draw_text(plugin_name, title_bounds, Color_Front, ui.g);
            Rect plugin_play_stop_bounds = rect_take_right(header_bounds, TITLE_HEIGHT);
            
            
            if(button(plugin_play_stop_bounds, StringLit("Plugin"), 258, ui))
            {
                *mute_plugin_was_clicked = true;
            }
            
        } break;
        case Asset_File_State_STAGE_BACKGROUND_LOADING :
        case Asset_File_State_BACKGROUND_LOADING :
        case Asset_File_State_STAGE_VALIDATION :
        case Asset_File_State_VALIDATING :
        case Asset_File_State_STAGE_USAGE :
        {
            draw_rectangle(header_bounds, 1.0f, Color_Front, ui.g);
            draw_text(StringLit("Loading"), header_bounds, Color_Front, ui.g);
        }break;
        case Asset_File_State_STAGE_UNLOADING :
        case Asset_File_State_OK_TO_UNLOAD :
        case Asset_File_State_UNLOADING :
        {
            draw_rectangle(header_bounds, 1.0f, Color_Front, ui.g);
            draw_text(StringLit("Unloading"), header_bounds, Color_Front, ui.g);
        }break;
        case Asset_File_State_NONE :
        {
            Rect load_plugin_button_bounds = rect_take_left(header_bounds, TITLE_HEIGHT * 3);
            header_bounds = rect_drop_left(header_bounds, TITLE_HEIGHT * 3);
            if(button(load_plugin_button_bounds, StringLit("Load Plugin"), 1024, ui))
            {
                *load_plugin_was_clicked = true;
            }
            draw_rectangle(header_bounds, 1.0f, Color_Front, ui.g);
            draw_text(StringLit("No plugin file"), header_bounds, Color_Front, ui.g);
        }break;
        
        case Asset_File_State_FAILED :
        {
            
            Rect load_plugin_button_bounds = rect_take_left(header_bounds, TITLE_HEIGHT * 3);
            header_bounds = rect_drop_left(header_bounds, TITLE_HEIGHT * 3);
            
            if(button(load_plugin_button_bounds, StringLit("Load Plugin"), 1024, ui))
            {
                *load_plugin_was_clicked = true;
            }
            
            draw_rectangle(header_bounds, 1.0f, Color_Front, ui.g);
            draw_text(StringLit("Compilation Error"), header_bounds, Color_Front, ui.g);
        }break;
    }
    
}

void draw_compiler_log(Compiler_Gui_Log *error_log, 
                       Rect bounds, 
                       UI_Context ui);

void plugin_parameter_panel(Plugin_Descriptor *descriptor, 
                            Plugin_Parameter_Value* current_parameter_values,
                            Rect bounds, 
                            UI_Context ui,
                            bool *parameters_were_tweaked);

void draw_visualization_panel(Rect bounds, UI_Context ui);

void main_panel_pluin_and_viz(Asset_File_State plugin_state, 
                              Plugin_Descriptor *descriptor, 
                              Plugin_Parameter_Value* current_parameter_values,
                              Compiler_Gui_Log *error_log,
                              Rect main_panel_bounds, 
                              UI_Context ui,
                              bool *parameters_were_tweaked)
{
    
    switch(plugin_state)
    {
        case Asset_File_State_IN_USE:
        case Asset_File_State_HOT_RELOAD_CHECK_FILE_FOR_UPDATE :
        case Asset_File_State_HOT_RELOAD_STAGE_BACKGROUND_LOADING :
        case Asset_File_State_HOT_RELOAD_BACKGROUND_LOADING :
        case Asset_File_State_HOT_RELOAD_STAGE_VALIDATION :
        case Asset_File_State_HOT_RELOAD_VALIDATING :
        case Asset_File_State_HOT_RELOAD_STAGE_SWAP :
        case Asset_File_State_HOT_RELOAD_SWAPPING :
        case Asset_File_State_HOT_RELOAD_STAGE_DISPOSE :
        case Asset_File_State_HOT_RELOAD_DISPOSING :
        {
            Rect tab_switcher_bounds = rect_shrinked(rect_take_top(main_panel_bounds, TITLE_HEIGHT), 5.0f, 5.0f); 
            
            main_panel_bounds = rect_drop_top(main_panel_bounds, TITLE_HEIGHT); 
            
            tab_switcher_bounds = rect_take_left(tab_switcher_bounds, 100.0f);
            if(button(tab_switcher_bounds, StringLit("Log"), 444, ui))
            {
                ui.state->show_error_log = !ui.state->show_error_log;
            }
            
            if(!ui.state->show_error_log)
            {
                Rect left_panel_bounds = rect_shrinked(rect_take_left(main_panel_bounds, PARAMETER_PANEL_WIDTH), 5.0f, 5.0f);
                Rect right_panel_bounds = rect_shrinked(rect_drop_left(main_panel_bounds, PARAMETER_PANEL_WIDTH), 5.0f, 5.0f);
                
                plugin_parameter_panel(descriptor, current_parameter_values, left_panel_bounds, ui, parameters_were_tweaked);
                
                draw_visualization_panel(right_panel_bounds, ui);
            }
            else
            {
                
                rect_shrink(&main_panel_bounds, 5.0f, 5.0f);
                draw_compiler_log(error_log, main_panel_bounds, ui);
            }
        } break;
        
        case Asset_File_State_FAILED :
        {
            draw_compiler_log(error_log, main_panel_bounds, ui);
            
        }break;
    }
    
}

void plugin_parameter_panel(Plugin_Descriptor *descriptor, 
                            Plugin_Parameter_Value* current_parameter_values,
                            Rect bounds, 
                            UI_Context ui,
                            bool *parameters_were_tweaked)
{
    draw_rectangle(bounds, 1.0f, Color_Front, ui.g);
    
    Rect parameter_bounds = bounds;
    parameter_bounds.dim.y = FIELD_TOTAL_HEIGHT;
    
    for(u32 parameter_idx = 0; parameter_idx < descriptor->num_parameters && descriptor->error.flag == Compiler_Success; parameter_idx++)
    {
        auto *current_parameter_value = &current_parameter_values[parameter_idx];
        auto *parameter_descriptor = &descriptor->parameters[parameter_idx];
        
        parameter_slider(parameter_idx, parameter_descriptor, current_parameter_value, parameter_bounds, ui, parameters_were_tweaked);
        
        parameter_bounds.origin.y += FIELD_TOTAL_HEIGHT + FIELD_MARGIN * 2;
    }
}

void draw_visualization_panel(Rect bounds, UI_Context ui) 
{
    draw_rectangle(bounds, 1.0f, Color_Front, ui.g);
    
    
    Rect ir_panel_bounds;
    Rect fft_panel_bounds;
    rect_split_vert_middle(bounds, &ir_panel_bounds, &fft_panel_bounds);
    
    //~IR
    
    draw_rectangle(ir_panel_bounds, 1.0f, Color_Front, ui.g);
    Rect ir_title_bounds = rect_take_top(ir_panel_bounds, 50.0f);
    draw_text(StringLit("Impulse Response"), ir_title_bounds, Color_Front, ui.g); 
    
    Rect ir_graph_bounds = rect_drop_top(ir_panel_bounds, 50.0f);
    Rect zoom_slider_bounds = rect_take_bottom(ir_graph_bounds, 30.0f);
    ir_graph_bounds = rect_drop_bottom(ir_graph_bounds, 30.0f);
    
    ui.g->ir.zoom_state = simple_slider(ui.g->ir.zoom_state, 600, zoom_slider_bounds, ui);
    
    draw_rectangle(ir_graph_bounds, 1.0f, Color_Front, ui.g);
    ui.g->ir.bounds = ir_graph_bounds;
    
    
    //~fft
    draw_rectangle(fft_panel_bounds, 1.0f, Color_Front, ui.g);
    Rect fft_title_bounds = rect_take_top(fft_panel_bounds, 50.0f);
    Rect fft_graph_bounds = rect_drop_top(fft_panel_bounds, 50.0f);
    
    draw_text(StringLit("Frequency Response"), fft_title_bounds, Color_Front, ui.g); 
    draw_rectangle(fft_graph_bounds, 1.0f, Color_Front, ui.g);
    ui.g->fft.bounds = fft_graph_bounds;
    
}

void draw_compiler_log(Compiler_Gui_Log *error_log, 
                       Rect bounds, 
                       UI_Context ui)
{
    
    draw_rectangle(bounds, 1.0f, Color_Front, ui.g);
    
    Rect message_bounds = rect_take_top(bounds, TITLE_HEIGHT);
    
    for(i32 i = 0; i < error_log->message_count; i++)
    {
        draw_text(error_log->messages[i], message_bounds, Color_Front, ui.g);
        message_bounds = rect_move_by(message_bounds, {0.0f, TITLE_HEIGHT});
    }
}

void audio_file_footer(Asset_File_State audio_file_state, 
                       Rect footer_bounds, 
                       UI_Context ui,
                       bool *clicked_on_load,
                       bool *clicked_on_play,
                       bool *clicked_on_loop)
{
    switch(audio_file_state)
    {
        case Asset_File_State_IN_USE :
        {
            Rect play_loop_bounds = rect_take_right(footer_bounds, TITLE_HEIGHT * 2);
            footer_bounds = rect_drop_right(footer_bounds, TITLE_HEIGHT * 2);
            
            draw_rectangle(play_loop_bounds, 1.0f, Color_Front, ui.g);
            
            Rect play_stop_bounds = rect_take_left(play_loop_bounds, TITLE_HEIGHT);
            Rect loop_bounds = rect_move_by(play_stop_bounds, {TITLE_HEIGHT, 0.0f});
            
            if(button(play_stop_bounds, StringLit("Play/Stop"), 256, ui))
                *clicked_on_play = true;
            
            if(button(loop_bounds, StringLit("Loop"), 257, ui))
                *clicked_on_loop = true;
            
            
            draw_text(StringLit("todo : draw filename, waveform idk"), footer_bounds, Color_Front, ui.g);
            
        }break;
        case Asset_File_State_STAGE_BACKGROUND_LOADING :
        case Asset_File_State_BACKGROUND_LOADING :
        case Asset_File_State_STAGE_VALIDATION :
        case Asset_File_State_VALIDATING :
        case Asset_File_State_STAGE_USAGE :
        {
            draw_text(StringLit("Loading"), footer_bounds, Color_Front, ui.g);
        }break;
        case Asset_File_State_STAGE_UNLOADING :
        case Asset_File_State_OK_TO_UNLOAD :
        case Asset_File_State_UNLOADING :
        {
            draw_text(StringLit("Unloading"), footer_bounds, Color_Front, ui.g);
            
        }break;
        case Asset_File_State_NONE :
        {
            draw_text(StringLit("No audio file"), footer_bounds, Color_Front, ui.g);
        }break;
        case Asset_File_State_FAILED :
        {
            draw_text(StringLit("Bad audio file"), footer_bounds, Color_Front, ui.g);
        }break;
    }
    Rect load_button_bounds = rect_take_left(footer_bounds, TITLE_HEIGHT * 3);
    footer_bounds = rect_drop_left(footer_bounds, TITLE_HEIGHT * 3);
    
    if(button(load_button_bounds, StringLit("Load"), 1024, ui))
        *clicked_on_load = true;
    
    draw_rectangle(footer_bounds, 1.0f, Color_Front, ui.g);
}




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
           bool *load_wav_was_clicked,
           bool *load_plugin_was_clicked)
{
    
    UI_Context ui{ frame_io, &ui_state, graphics_ctx };
    
    if(frame_io.mouse_released)
    {
        ui_state.previous_selected_parameter_id = ui_state.selected_parameter_id;
        ui_state.selected_parameter_id = -1;
    }
    
    Rect window_bounds = { Vec2{0.0f, 0.0f}, graphics_ctx->window_dim };
    Rect header_bounds = rect_shrinked(rect_take_top(window_bounds, TITLE_HEIGHT + 10.0f), 5.0f, 10.0f);
    
    window_bounds = rect_drop_top(window_bounds, TITLE_HEIGHT); 
    Rect main_panel_bounds = rect_drop_bottom(window_bounds, TITLE_HEIGHT);
    
    Rect footer_bounds = rect_shrinked(rect_take_bottom(window_bounds, TITLE_HEIGHT + 10.0f), 5.0f, 10.0f);
    draw_rectangle(footer_bounds, 1.0f, Color_Front, ui.g);
    
    MemoryBarrier();
    auto plugin_state = *audio_ctx->m->plugin_state; 
    auto audio_file_state = *audio_ctx->audio_file_state;
    MemoryBarrier();
    
    {
        bool local_load_plugin_was_clicked = false;
        bool mute_plugin_was_clicked = false;
        
        header(plugin_state, descriptor.name, header_bounds, ui, &local_load_plugin_was_clicked, &mute_plugin_was_clicked);
        
        if(mute_plugin_was_clicked)
            audio_ctx->plugin_play = audio_ctx->plugin_play == 0 ? 1 : 0;
        
        if(local_load_plugin_was_clicked)
            *load_plugin_was_clicked = true;
    }
    
    
    main_panel_pluin_and_viz(plugin_state, &descriptor, current_parameter_values, error_log,
                             main_panel_bounds, ui, parameters_were_tweaked);
    
    
    //~footer
    {
        bool clicked_on_load = false;
        bool clicked_on_play = false;
        bool clicked_on_loop = false;
        
        audio_file_footer(audio_file_state,
                          footer_bounds,
                          ui,
                          &clicked_on_load,
                          &clicked_on_play,
                          &clicked_on_loop);
        
        if(clicked_on_load)
        {
            *load_wav_was_clicked = true;
        }
        if(clicked_on_play)
        {
            if(audio_ctx->audio_file_play){
                audio_ctx->audio_file_play = 0;
                MemoryBarrier();
                audio_ctx->audio_file->read_cursor = 0;
            }
            else{
                audio_ctx->audio_file_play = 1;
                MemoryBarrier();
            }
        }
        if(clicked_on_loop)
        {
            audio_ctx->audio_file_loop = audio_ctx->audio_file_loop == 0 ? 1 : 0;
            octave_assert(audio_file_state == Asset_File_State_IN_USE);
        }
    }
}
