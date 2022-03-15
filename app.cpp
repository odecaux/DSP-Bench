#include "hardcoded_values.h"


#include "stdlib.h"
#include "assert.h"
#include "stdio.h"
#include "math.h"
#include "string.h"

#include "windows.h"


#include "memory.h"
#include "base.h"
#include "win32_helpers.h"
#include "structs.h"
#include "plugin.h"
#include "draw.h"
#include "app.h"


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
        .mouse_pos_prev, */
        .mouse_delta = {0.0f, 0.0f},
        
        .mouse_double_click_time = 175.0f,
        .mouse_down_time = -1.0f,
        .right_mouse_down_time = -1.0f,
        .mouse_clicked_time = 0, // TODO(octave): on est sur ?
        
        .left_ctrl_pressed = false,
        .left_ctrl_down = false,
    };
}

IO io_state_advance(IO io)
{
    io.frame_count++;
    io.time += io.delta_time;
    
    if(io.mouse_position.x < 0.0f && io.mouse_position.y < 0.0f)
        io.mouse_position = Vec2(-99999.0f, -99999.0f);
    
    if((io.mouse_position.x < 0.0f && io.mouse_position.y < 0.0f) ||
       (io.mouse_pos_prev.x < 0.0f && io.mouse_pos_prev.y < 0.0f))
        io.mouse_delta = Vec2{0.0f, 0.0f};
    else{
        io.mouse_delta.x = io.mouse_position.x - io.mouse_pos_prev.x;
        io.mouse_delta.y = io.mouse_position.y - io.mouse_pos_prev.y;
        
    }
    io.mouse_pos_prev = io.mouse_position;
    
    io.mouse_released = !io.mouse_down && io.mouse_down_time >= 0.0f;
    
    
    if(io.mouse_down)
    {
        if(io.mouse_down_time < 0.0f)
        {
            io.mouse_down_time = 0.1f;
            io.mouse_clicked = true;
            
        }
        else
        {
            io.mouse_down_time += 0.2f;// io.delta_time;
            io.mouse_clicked = false;
        }
    }
    else
    {
        io.mouse_down_time = -1.0f;
        io.mouse_clicked = false;
    }
    
    
    io.mouse_double_clicked = false;
    if(io.mouse_clicked)
    {
        if(io.time - io.mouse_clicked_time < io.mouse_double_click_time)
        {
            io.mouse_double_clicked = true;
            io.mouse_clicked_time = -1000000.0f;
        }
        else
        {
            io.mouse_clicked_time = io.time;
        }
    }
    
    
    if(io.right_mouse_down)
    {
        if(io.right_mouse_down_time < 0.0f)
        {
            io.right_mouse_down_time = 0.1f;
            io.right_mouse_clicked = true;
        }
        else
        {
            io.right_mouse_down_time += io.delta_time;
            io.right_mouse_clicked = false;
        }
    }
    else
    {
        io.right_mouse_down_time = -1.0f;
        io.right_mouse_clicked = false;
    }
    
    return io;
}

void compute_IR(Plugin& handle, 
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
    
    char* IR_parameters_holder = (char*) m_allocate(handle.descriptor.parameters_struct.size, "ir parameter blob");
    char* IR_state_holder = (char*) m_allocate(handle.descriptor.state_struct.size, "ir state blob");
    
    plugin_set_parameter_holder_from_values(&handle.descriptor, current_parameters_values, IR_parameters_holder);
    
    handle.initialize_state_f(IR_parameters_holder, 
                              IR_state_holder, 
                              audio_parameters.num_channels, 
                              audio_parameters.sample_rate, 
                              nullptr);
    
    handle.audio_callback_f(IR_parameters_holder, 
                            IR_state_holder, 
                            IR_buffer, 
                            audio_parameters.num_channels, 
                            IR_length, 
                            audio_parameters.sample_rate);
    
    m_free(IR_parameters_holder, "ir parameter blob");
    m_free(IR_state_holder, "ir state blob");
}
