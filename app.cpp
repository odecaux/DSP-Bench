#include "hardcoded_values.h"

#include "stdlib.h"
#include "assert.h"
#include "stdio.h"
#include "math.h"
#include "string.h"

#include "windows.h"


#include "memory.h"
#include "base.h"
#include "structs.h"
#include "descriptor.h"
#include "draw.h"
#include "app.h"



//#define log printf
#define log noop_log

int noop_log(const char *__restrict __format, ...){
    return 1;
}

IO io_initial_state()
{
    return {
        .frame_count = 0,
        .delta_time = 0,
        .time = 0,
        
        .mouse_down = false,
        .mouse_clicked = false,
        .mouse_released = true,
        
        .right_mouse_down = false,
        .mouse_double_clicked = false,
        .delete_pressed = false,
        
        /*
        .mouse_position,
        .mouse_pos_prev,
        .mouse_delta,
        */ // TODO(octave): on peut les initialiser ici en vrai
        
        .mouse_double_click_time = 175.0f,
        .mouse_down_time = -1.0f,
        .right_mouse_down_time = -1.0f,
        .mouse_clicked_time = 0 // TODO(octave): on est sur ?
    };
}

IO io_state_advance(const IO old_io)
{
    IO new_io = old_io;
    
    new_io.frame_count++;
    new_io.time += new_io.delta_time;
    
    if(new_io.mouse_position.x < 0.0f && new_io.mouse_position.y < 0.0f)
        new_io.mouse_position = Vec2(-99999.0f, -99999.0f);
    
    //?
    if((new_io.mouse_position.x < 0.0f && new_io.mouse_position.y < 0.0f) ||
       (new_io.mouse_pos_prev.x < 0.0f && new_io.mouse_pos_prev.y < 0.0f))
        new_io.mouse_delta = Vec2{0.0f, 0.0f};
    else{
        new_io.mouse_delta.x = new_io.mouse_position.x - new_io.mouse_pos_prev.x;
        new_io.mouse_delta.y = new_io.mouse_position.y - new_io.mouse_pos_prev.y;
        
    }
    new_io.mouse_pos_prev = new_io.mouse_position;
    
    new_io.mouse_released = !new_io.mouse_down && new_io.mouse_down_time >= 0.0f;
    
    
    if(new_io.mouse_down)
    {
        if(new_io.mouse_down_time < 0.0f)
        {
            new_io.mouse_down_time = 0.1f;
            new_io.mouse_clicked = true;
            
        }
        else
        {
            new_io.mouse_down_time += 0.2f;// new_io.delta_time;
            new_io.mouse_clicked = false;
        }
    }
    else
    {
        new_io.mouse_down_time = -1.0f;
        new_io.mouse_clicked = false;
    }
    
    
    new_io.mouse_double_clicked = false;
    if(new_io.mouse_clicked)
    {
        if(new_io.time - new_io.mouse_clicked_time < new_io.mouse_double_click_time)
        {
            new_io.mouse_double_clicked = true;
            new_io.mouse_clicked_time = -1000000.0f;
        }
        else
        {
            new_io.mouse_clicked_time = new_io.time;
        }
    }
    
    
    if(new_io.right_mouse_down)
    {
        if(new_io.right_mouse_down_time < 0.0f)
        {
            new_io.right_mouse_down_time = 0.1f;
            new_io.right_mouse_clicked = true;
        }
        else
        {
            new_io.right_mouse_down_time += new_io.delta_time;
            new_io.right_mouse_clicked = false;
        }
    }
    else
    {
        new_io.right_mouse_down_time = -1.0f;
        new_io.right_mouse_clicked = false;
    }
    
    return new_io;
}

void compute_IR(Plugin_Handle& handle, 
                real32** IR_buffer, 
                u32 IR_length, 
                Audio_Parameters& audio_parameters,
                Plugin_Parameter_Value* current_parameters_values)
{
    for(u32 channel = 0; channel < audio_parameters.num_channels; channel++)
    {
        IR_buffer[channel][0] = 1.0f;
        for(u32 sample = 1; sample < IR_length; sample++)
        {
            IR_buffer[channel][sample] = 0.0f;
        }
    }
    
    char* IR_parameters_holder = (char*) m_allocate(handle.descriptor.parameters_struct.size);
    char* IR_state_holder = (char*) m_allocate(handle.descriptor.state_struct.size);
    
    update_parameters_holder(&handle.descriptor, current_parameters_values, IR_parameters_holder);
    
    handle.initialize_state_f(IR_parameters_holder, 
                              IR_state_holder, 
                              audio_parameters.num_channels, 
                              audio_parameters.sample_rate, 
                              &malloc_allocator);
    
    handle.audio_callback_f(IR_parameters_holder, 
                            IR_state_holder, 
                            IR_buffer, 
                            audio_parameters.num_channels, 
                            IR_length, 
                            audio_parameters.sample_rate);
    
    m_free(IR_parameters_holder);
    m_free(IR_state_holder);
}

real32 slider(real32 normalized_value, i32 id, 
              String title, String current_value_label, String min_label, String max_label, Rect bounds, IO io, 
              UI_State *ui_state, Graphics_Context *graphics_ctx)
{
    Rect title_bounds = rect_take_top(bounds, FIELD_TITLE_HEIGHT);
    Rect current_value_bounds = rect_remove_padding(rect_drop_top(bounds, FIELD_TITLE_HEIGHT), 2.5f, 2.5f);
    Rect slider_and_minmax_bounds = rect_move_by(current_value_bounds, {0.0f, FIELD_TITLE_HEIGHT});
    
    draw_text(title, title_bounds, Color_Front, &graphics_ctx->atlas);
    
    Rect slider_bounds = rect_remove_padding(slider_and_minmax_bounds, MIN_MAX_LABEL_WIDTH, 0.0f);
    Rect min_label_bounds = rect_take_left(slider_and_minmax_bounds, MIN_MAX_LABEL_WIDTH);
    Rect max_label_bounds = rect_take_right(slider_and_minmax_bounds, MIN_MAX_LABEL_WIDTH);
    
    draw_rectangle(slider_bounds, 1.0f, Color_Front, &graphics_ctx->atlas);
    draw_rectangle(min_label_bounds, 1.0f, Color_Front, &graphics_ctx->atlas);
    draw_rectangle(max_label_bounds, 1.0f, Color_Front, &graphics_ctx->atlas);
    
    draw_text(min_label, min_label_bounds, Color_Front, &graphics_ctx->atlas);
    draw_text(max_label, max_label_bounds, Color_Front, &graphics_ctx->atlas);
    draw_text(current_value_label, current_value_bounds, Color_Front, &graphics_ctx->atlas);
    
    draw_slider(slider_bounds, normalized_value, &graphics_ctx->atlas);
    
    if(io.mouse_clicked && rect_contains(slider_bounds, io.mouse_position))
    {
        assert(ui_state->selected_parameter_idx == -1);
        ui_state->selected_parameter_idx = id;
    }
    
    bool dragging = io.mouse_down && (io.mouse_delta.x != 0.0f
                                      || io.mouse_delta.y != 0.0f);
    
    if(ui_state->selected_parameter_idx == id 
       && (dragging || io.mouse_clicked))
    {
        real32 mouse_x = io.mouse_position.x;
        real32 normalized_mouse_value = (mouse_x - slider_bounds.origin.x - (SLIDER_WIDTH / 2)) / (slider_bounds.dim.x - SLIDER_WIDTH);
        
        return octave_clamp(normalized_mouse_value, 0.0f, 1.0f);
    }
    else
    {
        return normalized_value;
    }
}

//TODO stereo 
void integrate_fft(real32* magnitude_buffer, u32 sample_count, real32* pixel_buffer, u32 pixel_count)
{
    if(pixel_count == sample_count)
    {
        memcpy(pixel_buffer, magnitude_buffer, pixel_count * sizeof(real32));
    }
    else if(pixel_count < sample_count)
    {
        for(u32 sample_idx = 0; sample_idx < sample_count; sample_idx++)
        {
            
            //NOTE overflow error
            u32 pixel_idx = sample_idx * pixel_count / sample_count;
            real32 value = magnitude_buffer[sample_idx];
            if(value > pixel_buffer[pixel_idx])
                pixel_buffer[pixel_idx] = value;
        }
    }
    else {
        for(u32 pixel_idx = 0; pixel_idx < pixel_count; pixel_idx++)
        {
            //NOTE overflow error
            u32 sample_idx = pixel_idx * sample_count / pixel_count ;
            pixel_buffer[pixel_idx] = magnitude_buffer[sample_idx];
        }
    }
}


bool button(Rect bounds, 
            String text, 
            u32 id, 
            Graphics_Context *graphics_ctx, 
            UI_State *ui_state,
            IO *io)
{
    fill_rectangle(bounds, 0xffffffff, &graphics_ctx->atlas);
    bounds = rect_remove_padding(bounds, 3.0f, 3.0f);
    
    bool hovered = rect_contains(bounds,io->mouse_position); 
    bool clicked = false;
    if(hovered && io->mouse_clicked)
    {
        clicked = true;
        ui_state->selected_parameter_idx = id;
    }
    bool down = ui_state->selected_parameter_idx == id;
    
    if(down)
        draw_rectangle(bounds, 5.0f, 0xff007700, &graphics_ctx->atlas);
    else if(hovered)
        draw_rectangle(bounds, 5.0f, 0xff770000, &graphics_ctx->atlas);
    else
        draw_rectangle(bounds, 4.0f, 0xff000000, &graphics_ctx->atlas);
    
    bounds = rect_remove_padding(bounds, 5.0f, 5.0f);
    draw_text(text, bounds, 0xff000000, &graphics_ctx->atlas);
    
    return clicked;
}

void frame(Plugin_Descriptor& descriptor, 
           Graphics_Context *graphics_ctx, 
           UI_State& ui_state, 
           IO frame_io, 
           Plugin_Parameter_Value* current_parameter_values,
           Audio_Context *audio_ctx,
           bool& parameters_were_tweaked)
{
    if(frame_io.mouse_released)
    {
        ui_state.selected_parameter_idx = -1;
    }
    
    Rect window_bounds = { Vec2{0.0f, 0.0f}, graphics_ctx->window_dim };
    Rect header_bounds = rect_remove_padding(rect_take_top(window_bounds, TITLE_HEIGHT), 10.0f, 10.0f);
    window_bounds = rect_drop_top(window_bounds, TITLE_HEIGHT);
    
    
    
    Rect title_bounds = rect_drop_right(header_bounds,  TITLE_HEIGHT * 3);
    draw_rectangle(title_bounds, 1.0f, Color_Front, &graphics_ctx->atlas);
    draw_text(descriptor.name, title_bounds, Color_Front, &graphics_ctx->atlas);
    
    Rect audio_buttons_panel_bounds = rect_take_right(header_bounds, TITLE_HEIGHT * 3);
    draw_rectangle(audio_buttons_panel_bounds, 1.0f, Color_Front, &graphics_ctx->atlas);
    Rect play_stop_bounds = rect_take_left(audio_buttons_panel_bounds, TITLE_HEIGHT);
    Rect loop_bounds = rect_move_by(play_stop_bounds, {TITLE_HEIGHT, 0.0f});
    Rect plugin_play_stop_bounds = rect_move_by(loop_bounds, {TITLE_HEIGHT, 0.0f});
    
    //TODO synchronization
    if(button(play_stop_bounds, StringLit("Play/Stop"), 256, graphics_ctx, &ui_state, &frame_io))
    {
        if(audio_ctx->audio_file_play){
            audio_ctx->audio_file_play = 0;
            MemoryBarrier();
            audio_ctx->audio_file_read_cursor = 0;
        }
        else{
            audio_ctx->audio_file_play = 1;
            MemoryBarrier();
        }
    }
    if(button(loop_bounds, StringLit("Loop"), 257, graphics_ctx, &ui_state, &frame_io))
    {
        audio_ctx->audio_file_loop = audio_ctx->audio_file_loop == 0 ? 1 : 0;
    }
    
    if(button(plugin_play_stop_bounds, StringLit("Plugin"), 258, graphics_ctx, &ui_state, &frame_io))
    {
        audio_ctx->plugin_play = audio_ctx->plugin_play == 0 ? 1 : 0;
    }
    
    
    Rect left_panel_bounds = rect_remove_padding(rect_take_left(window_bounds, PARAMETER_PANEL_WIDTH), 10.0f, 10.0f);
    Rect right_panel_bounds = rect_remove_padding(rect_drop_left(window_bounds, PARAMETER_PANEL_WIDTH), 10.0f, 10.0f);
    
    draw_rectangle(left_panel_bounds, 1.0f, Color_Front, &graphics_ctx->atlas);
    
    Rect parameter_bounds = left_panel_bounds;
    parameter_bounds.dim.y = FIELD_TOTAL_HEIGHT;
    
    for(u32 parameter_idx = 0; parameter_idx < descriptor.num_parameters; parameter_idx++)
    {
        auto& current_parameter_value = current_parameter_values[parameter_idx];
        auto& parameter_descriptor = descriptor.parameters[parameter_idx];
        
        switch(parameter_descriptor.type)
        {
            case Int :
            {
                int current_value = current_parameter_value.int_value; 
                char current_value_text[256];
                int text_size = sprintf(current_value_text, "%d", current_value);
                assert(text_size >= 0);
                String current_value_label = {.str = current_value_text, .size = (u64)text_size};
                
                real32 current_normalized_value = normalize_parameter_int_value(parameter_descriptor.int_param, current_value);
                
                
                i32 min_value = parameter_descriptor.int_param.min;
                char min_value_text[256];
                text_size = sprintf(min_value_text, "%d", min_value);
                assert(text_size >= 0);
                String min_label = {.str = min_value_text, .size = (u64)text_size};
                
                i32 max_value = parameter_descriptor.int_param.max;
                char max_value_text[256];
                text_size = sprintf(max_value_text, "%d", max_value);
                assert(text_size >= 0);
                String max_label = {.str = max_value_text, .size = (u64)text_size};
                
                real32 new_normalized_value =
                    slider(current_normalized_value, parameter_idx, 
                           parameter_descriptor.name, current_value_label, min_label, max_label, 
                           parameter_bounds, frame_io, &ui_state, graphics_ctx);
                
                if(new_normalized_value != current_normalized_value )
                {
                    parameters_were_tweaked = true;
                    auto new_int_value = denormalize_int_value(parameter_descriptor.int_param, new_normalized_value);
                    current_parameter_value.int_value = new_int_value;
                }
                
            }break;
            case Float : 
            {
                real32 current_value = current_parameter_value.float_value; 
                char current_value_text[256];
                int text_size = sprintf(current_value_text, "%.3f", current_value);
                assert(text_size >= 0);
                String current_value_label = {.str = current_value_text, .size = (u64)text_size};
                
                real32 current_normalized_value = normalize_parameter_float_value(parameter_descriptor.float_param, current_value);
                
                real32 min_value = parameter_descriptor.float_param.min;
                char min_value_text[256];
                text_size = sprintf(min_value_text, "%.3f", min_value);
                assert(text_size >= 0);
                String min_label = {.str = min_value_text, .size = (u64)text_size};
                
                real32 max_value = parameter_descriptor.float_param.max;
                char max_value_text[256];
                text_size = sprintf(max_value_text, "%.3f", max_value);
                assert(text_size >= 0);
                String max_label = {.str = max_value_text, .size = (u64)text_size};
                
                real32 new_normalized_value =
                    slider(current_normalized_value, parameter_idx, 
                           parameter_descriptor.name, current_value_label, min_label, max_label, 
                           parameter_bounds, frame_io, &ui_state, graphics_ctx);
                
                if(new_normalized_value != current_normalized_value)
                {
                    parameters_were_tweaked = true;
                    auto new_float_value = denormalize_float_value(parameter_descriptor.float_param, new_normalized_value);
                    current_parameter_value.float_value = new_float_value;
                }
                
            }break;
            case Enum : 
            {
                i32 index = current_parameter_value.enum_value;
                real32 current_normalized_value = normalize_parameter_enum_index(parameter_descriptor.enum_param, index);
                Parameter_Enum_Entry value = parameter_descriptor.enum_param.entries[index];
                
                real32 new_normalized_value =
                    slider(current_normalized_value, parameter_idx, 
                           parameter_descriptor.name, value.name, {}, {}, 
                           parameter_bounds, frame_io, &ui_state, graphics_ctx);
                
                if(new_normalized_value != current_normalized_value)
                {
                    parameters_were_tweaked = true;
                    auto new_index = denormalize_enum_index(parameter_descriptor.enum_param, new_normalized_value);
                    auto new_value = enum_index_to_value(parameter_descriptor.enum_param, new_index);
                    current_parameter_value.enum_value = new_value;
                }
                
            }break;
        }
        
        parameter_bounds.origin.y += FIELD_TOTAL_HEIGHT + FIELD_MARGIN;
    }
    
    
    draw_rectangle(right_panel_bounds, 1.0f, Color_Front, &graphics_ctx->atlas);
    
    Rect ir_panel_bounds;
    Rect fft_panel_bounds;
    rect_split_vert_middle(right_panel_bounds, &ir_panel_bounds, &fft_panel_bounds);
    
    draw_rectangle(ir_panel_bounds, 1.0f, Color_Front, &graphics_ctx->atlas);
    Rect ir_title_bounds = rect_take_top(ir_panel_bounds, 50.0f);
    Rect ir_graph_bounds = rect_drop_top(ir_panel_bounds, 50.0f);
    
    
    draw_text(StringLit("Impulse Response"), ir_title_bounds, Color_Front, &graphics_ctx->atlas); 
    
    draw_rectangle(ir_graph_bounds, 1.0f, Color_Front, &graphics_ctx->atlas);
    //draw_IR(ir_graph_bounds /*, IR_min_buffer, IR_max_buffer, IR_pixel_count*/ , &graphics_ctx->ir);
    graphics_ctx->ir.bounds = ir_graph_bounds;
    
    
    draw_rectangle(fft_panel_bounds, 1.0f, Color_Front, &graphics_ctx->atlas);
    Rect fft_title_bounds = rect_take_top(fft_panel_bounds, 50.0f);
    Rect fft_graph_bounds = rect_drop_top(fft_panel_bounds, 50.0f);
    
    
    draw_text(StringLit("Frequency Response"), fft_title_bounds, Color_Front, &graphics_ctx->atlas); 
    draw_rectangle(fft_graph_bounds, 1.0f, Color_Front, &graphics_ctx->atlas);
    //draw_fft(fft_graph_bounds, &graphics_ctx->fft);
    graphics_ctx->fft.bounds = fft_graph_bounds;
}

