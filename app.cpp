#include "hardcoded_values.h"

#include "stdlib.h"
#include "assert.h"
#include "stdio.h"
#include "math.h"
#include "string.h"

#include "windows.h"

#include "memory.h"

#include "base.h"
#include "descriptor.h"
#include "structs.h"
#include "opengl.h"
#include "os_helpers.h"
#include "font.h"
#include "draw.h"
#include "app.h"

#include <ippdefs.h>

#include "fft.h"
//#define log printf
#define log noop_log

int noop_log(const char *__restrict __format, ...){
    return 1;
}

Vec2 win32_get_mouse_position(HWND window)
{
    POINT mouse_position;
    GetCursorPos(&mouse_position);
    ScreenToClient(window, &mouse_position);
    return Vec2{(real32)mouse_position.x, (real32)mouse_position.y};
}


void win32_error_window(const String title, const String text)
{
    MessageBox(0, text.str, title.str, MB_OK);
}


i64 win32_init_timer()
{
    LARGE_INTEGER time;
    QueryPerformanceCounter(&time);
    return (i64) time.QuadPart;
}

i64 win32_pace_60_fps(i64 last_time, i64 *current_time, real32 *delta_time) // TODO(octave): refactor variable names
{
    
    //manual framerate management
    //unnecessary because we use OpenGL's vsync thing : wglSwapIntervalEXT(1);
#if 0
    LARGE_INTEGER render_end;
    QueryPerformanceCounter(&render_end);
    u64 render_elapsed  = render_end.QuadPart - last_time;
    auto ms_render = ((1000000.0f*render_elapsed) / counter_frequency.QuadPart) / 1000.0f;
    int ceiled = ceil(ms_render);
    
    if(16-ceiled  > 0)
    {
        Sleep(16 - ceiled);
        printf("%d\n", ceiled);
    }
    
#endif
    
    LARGE_INTEGER counter_frequency;
    QueryPerformanceFrequency(&counter_frequency);
    
    LARGE_INTEGER frame_end;
    QueryPerformanceCounter(&frame_end);
    auto frame_elapsed  = frame_end.QuadPart - last_time;
    auto ms_frame = ((1000000.0f*frame_elapsed) / counter_frequency.QuadPart) / 1000.0f;
    *delta_time = ms_frame;
    return frame_end.QuadPart;
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

void render_IR(real32** IR_buffer, u32 channel_count, u32 IR_length, real32* min_pixel_buffer, real32* max_pixel_buffer, u32 pixel_count)
{
    memset(min_pixel_buffer, 0, pixel_count * sizeof(real32));
    memset(max_pixel_buffer, 0, pixel_count * sizeof(real32));
    
    for(u32 channel = 0; channel < channel_count; channel++)
    {
        for(u32 sample = 0; sample < IR_length; sample++)
        {
            u32 pixel_idx = sample * pixel_count / IR_length;
            real32 value = IR_buffer[channel][sample];
            
            if(value > max_pixel_buffer[pixel_idx])
                max_pixel_buffer[pixel_idx] = value;
            
            if(value < min_pixel_buffer[pixel_idx])
                min_pixel_buffer[pixel_idx] = value;
        }
    }
}


//TODO stereo 
void render_fft(real32* magnitude_buffer, u32 fft_length, real32* pixel_buffer, u32 pixel_count)
{
    memset(pixel_buffer, 0, pixel_count * sizeof(real32));
    
    for(u32 sample = 0; sample < fft_length; sample++)
    {
        u32 pixel_idx = sample * pixel_count / fft_length;
        real32 value = magnitude_buffer[sample];
        if(value > pixel_buffer[pixel_idx])
            pixel_buffer[pixel_idx] = value;
    }
}



//TODO bad API, pour l'instant ça passe pcq la dimension est constante
void draw_IR(Rect bounds,
             /*real32* IR_min_buffer,
             real32* IR_max_buffer,
             u32 IR_pixel_count,*/
             GraphicsContext *graphics_ctx)
{
    bounds.dim.x -= 2.0f;
    bounds.dim.y -= 2.0f;
    Vec2 top_left = bounds.origin;
    Vec2 top_right = Vec2{ bounds.origin.x + bounds.dim.x, bounds.origin.y };
    Vec2 bottom_right = Vec2{ bounds.origin.x + bounds.dim.x, bounds.origin.y + bounds.dim.y};
    Vec2 bottom_left = Vec2{ bounds.origin.x, bounds.origin.y + bounds.dim.y};
    
    Vec2 uv_top_left = Vec2{ 0.0f, 1.0f};
    Vec2 uv_top_right = Vec2{ 1.0f, 1.0f};
    Vec2 uv_bottom_left = Vec2{ 0.0f, 0.0f};
    Vec2 uv_bottom_right = Vec2{ 1.0f, 0.0f};
    
    graphics_ctx->ir_vertices[0].pos = bottom_left;
    graphics_ctx->ir_vertices[0].quad_pos = uv_bottom_left;
    
    graphics_ctx->ir_vertices[1].pos = top_right;
    graphics_ctx->ir_vertices[1].quad_pos = uv_top_right;
    
    graphics_ctx->ir_vertices[2].pos = top_left;
    graphics_ctx->ir_vertices[2].quad_pos = uv_top_left;
    
    graphics_ctx->ir_vertices[3].pos = bottom_left;
    graphics_ctx->ir_vertices[3].quad_pos = uv_bottom_left;
    
    graphics_ctx->ir_vertices[4].pos = bottom_right;
    graphics_ctx->ir_vertices[4].quad_pos = uv_bottom_right;
    
    graphics_ctx->ir_vertices[5].pos = top_right;
    graphics_ctx->ir_vertices[5].quad_pos = uv_top_right;
}

//TODO bad API, pour l'instant ça passe pcq la dimension est constante
void draw_fft(Rect bounds,
              GraphicsContext *graphics_ctx)
{
    bounds.dim.x -= 2.0f;
    bounds.dim.y -= 2.0f;
    Vec2 top_left = bounds.origin;
    Vec2 top_right = Vec2{ bounds.origin.x + bounds.dim.x, bounds.origin.y };
    Vec2 bottom_right = Vec2{ bounds.origin.x + bounds.dim.x, bounds.origin.y + bounds.dim.y};
    Vec2 bottom_left = Vec2{ bounds.origin.x, bounds.origin.y + bounds.dim.y};
    
    Vec2 uv_top_left = Vec2{ 0.0f, 1.0f};
    Vec2 uv_top_right = Vec2{ 1.0f, 1.0f};
    Vec2 uv_bottom_left = Vec2{ 0.0f, 0.0f};
    Vec2 uv_bottom_right = Vec2{ 1.0f, 0.0f};
    
    graphics_ctx->fft_vertices[0].pos = bottom_left;
    graphics_ctx->fft_vertices[0].quad_pos = uv_bottom_left;
    
    graphics_ctx->fft_vertices[1].pos = top_right;
    graphics_ctx->fft_vertices[1].quad_pos = uv_top_right;
    
    graphics_ctx->fft_vertices[2].pos = top_left;
    graphics_ctx->fft_vertices[2].quad_pos = uv_top_left;
    
    graphics_ctx->fft_vertices[3].pos = bottom_left;
    graphics_ctx->fft_vertices[3].quad_pos = uv_bottom_left;
    
    graphics_ctx->fft_vertices[4].pos = bottom_right;
    graphics_ctx->fft_vertices[4].quad_pos = uv_bottom_right;
    
    graphics_ctx->fft_vertices[5].pos = top_right;
    graphics_ctx->fft_vertices[5].quad_pos = uv_top_right;
}


void frame(Plugin_Descriptor& descriptor, 
           GraphicsContext *graphics_ctx, 
           UI_State& ui_state, 
           IO frame_io, 
           /*
           real32* IR_min_buffer,
           real32* IR_max_buffer,
           u32 IR_pixel_count,
           */
           Plugin_Parameter_Value* current_parameter_values,
           bool& parameters_were_tweaked)
{
    
    if(frame_io.mouse_released)
    {
        ui_state.selected_parameter_idx = -1;
    }
    
    Rect title_bounds = { 
        Vec2{0.0f, 0.0f}, 
        Vec2{TOTAL_WIDTH, TITLE_HEIGHT}
    };
    draw_rectangle(title_bounds, Color_Front, graphics_ctx);
    draw_text(StringLit("gain.cpp"), title_bounds, Color_Front, graphics_ctx);
    
    Vec2 position = {0.0f, TITLE_HEIGHT};
    
    
    for(u32 parameter_idx = 0; parameter_idx < descriptor.num_parameters; parameter_idx++)
    {
        position.y += FIELD_MARGIN * 2;
        
        auto& current_parameter_value = current_parameter_values[parameter_idx];
        auto& parameter_descriptor = descriptor.parameters[parameter_idx];
        
        Rect field_title_bounds = {
            position, {TOTAL_WIDTH, FIELD_TITLE_HEIGHT}
        };
        
        draw_text(parameter_descriptor.name, field_title_bounds, Color_Front, graphics_ctx);
        draw_rectangle(field_title_bounds, Color_Front, graphics_ctx);
        position.y += FIELD_TITLE_HEIGHT + FIELD_MARGIN;
        
        Rect field_bounds = {
            position,
            {TOTAL_WIDTH, FIELD_HEIGHT}
        };
        
        Rect slider_bounds = rect_remove_padding(field_bounds, MIN_MAX_LABEL_WIDTH, 0.0f);
        
        Rect min_label_bounds = {
            {0.0f, position.y},
            {MIN_MAX_LABEL_WIDTH, FIELD_HEIGHT}
        };
        
        
        Rect max_label_bounds = {
            {0.0f + TOTAL_WIDTH - MIN_MAX_LABEL_WIDTH, position.y},
            {MIN_MAX_LABEL_WIDTH, FIELD_HEIGHT}
        };
        
        draw_rectangle(min_label_bounds, Color_Front, graphics_ctx);
        
        draw_rectangle(slider_bounds, Color_Front, graphics_ctx);
        draw_rectangle(max_label_bounds, Color_Front, graphics_ctx);
        
        real32 new_normalized_value;
        
        if(frame_io.mouse_clicked && rect_contains(slider_bounds, frame_io.mouse_position))
        {
            ui_state.selected_parameter_idx = parameter_idx;
        }
        
        bool should_update_this_parameter = false;
        
        if(ui_state.selected_parameter_idx == parameter_idx 
           && frame_io.mouse_down 
           && (frame_io.mouse_delta.x != 0.0f
               || frame_io.mouse_delta.y != 0.0f))
        {
            real32 mouse_x = frame_io.mouse_position.x;
            real32 normalized_mouse_value = (mouse_x - slider_bounds.origin.x) / slider_bounds.dim.x;
            if(normalized_mouse_value < 0.0f)
                normalized_mouse_value = 0.0f;
            else if(normalized_mouse_value > 1.0f)
                normalized_mouse_value = 1.0f;
            
            new_normalized_value = normalized_mouse_value;
            parameters_were_tweaked = true;
            should_update_this_parameter = true;
        }
        
        
        
        switch(parameter_descriptor.type)
        {
            case Int :
            {
                int current_value = current_parameter_value.int_value; 
                //int value = int_parameter_extract_value(parameter, plugin_state_holder);
                real32 current_normalized_value = normalize_parameter_int_value(parameter_descriptor.int_param, current_value);
                draw_slider(slider_bounds, current_normalized_value, graphics_ctx);
                
                if(should_update_this_parameter)
                {
                    auto new_int_value = denormalize_int_value(parameter_descriptor.int_param, new_normalized_value);
                    current_parameter_value.int_value = new_int_value;
                }
            }break;
            case Float : 
            {
                float current_value = current_parameter_value.float_value;
                real32 current_normalized_value = normalize_parameter_float_value(parameter_descriptor.float_param, current_value);
                draw_slider(slider_bounds, current_normalized_value, graphics_ctx);
                
                if(should_update_this_parameter)
                {
                    auto new_float_value = denormalize_float_value(parameter_descriptor.float_param, new_normalized_value);
                    current_parameter_value.float_value = new_float_value;
                }
            }break;
            case Enum : 
            {
                i32 index = current_parameter_value.enum_value;
                real32 current_normalized_value = normalize_parameter_enum_index(parameter_descriptor.enum_param, index);
                Parameter_Enum_Entry value = parameter_descriptor.enum_param.entries[index];
                draw_slider(slider_bounds, current_normalized_value, graphics_ctx);
                
                draw_text(value.name, slider_bounds, Color_Front, graphics_ctx);
                
                if(should_update_this_parameter)
                {
                    auto new_index = denormalize_enum_index(parameter_descriptor.enum_param, new_normalized_value);
                    auto new_value = enum_index_to_value(parameter_descriptor.enum_param, new_index);
                    current_parameter_value.enum_value = new_value;
                }
            }break;
        }
        
        position.y += FIELD_HEIGHT;
    }
    
    
    //TODO hardcoded
    
    //~ 
    //IR
    {
        Rect IR_title_bounds = {
            {TOTAL_WIDTH + 10.0f, 5.0f},
            {200.0f, 10.0f}
        };
        
        
        draw_text(StringLit("Impulse Response"), IR_title_bounds, Color_Front, graphics_ctx); 
        
        Rect IR_outer_bounds{
            {TOTAL_WIDTH, 15.0f},
            {500.0f, 150.0f}
        };
        auto IR_inner_bounds = rect_remove_padding(IR_outer_bounds, 10.0f, 10.0f);
        draw_rectangle(IR_inner_bounds, Color_Front, graphics_ctx);
        draw_IR(IR_inner_bounds /*, IR_min_buffer, IR_max_buffer, IR_pixel_count*/ , graphics_ctx);
        
        
    }
    
    
    //~ 
    
    {
        
        Rect IR_title_bounds = {
            {TOTAL_WIDTH + 10.0f, 160.0f},
            {200.0f, 10.0f}
        };
        
        
        draw_text(StringLit("Frequency Response"), IR_title_bounds, Color_Front, graphics_ctx); 
        
        Rect IR_outer_bounds{
            {TOTAL_WIDTH, 170.0f},
            {500.0f, 150.0f}
        };
        auto IR_inner_bounds = rect_remove_padding(IR_outer_bounds, 10.0f, 10.0f);
        draw_rectangle(IR_inner_bounds, Color_Front, graphics_ctx);
        draw_fft(IR_inner_bounds, graphics_ctx);
        
    }
}


LRESULT CALLBACK WindowProc(HWND window, UINT message, WPARAM w_param, LPARAM l_param)
{
    LRESULT result = 0;
    
    
    if (message == WM_CREATE)
    {
        LPCREATESTRUCT pcs = (LPCREATESTRUCT)l_param;
        GraphicsContext* graphics_ctx = (GraphicsContext*)pcs->lpCreateParams;
        
        SetWindowLongPtr(
                         window,
                         GWLP_USERDATA,
                         (LONG_PTR)(graphics_ctx)
                         );
        
        result = 1;
    }
    else {
        
        GraphicsContext *graphics_ctx = (GraphicsContext*)(GetWindowLongPtrW(window, GWLP_USERDATA));
        
        if(message == WM_SIZE)
        {
            UINT width = LOWORD(l_param);
            UINT height = HIWORD(l_param);
            graphics_ctx->window_dim = { (real32)width, (real32)height};
        }
        else if(message == WM_DESTROY || message == WM_DESTROY || message == WM_QUIT)
        {
            printf("quit\n");
            PostQuitMessage(0);
            return 0;
        }
        else 
        {
            result = DefWindowProc(window, message, w_param, l_param); 
        }
    }
    return result;
}




void initialize_gui(Plugin_Handle& handle,
                    Audio_Parameters& audio_parameters,
                    Plugin_Parameter_Value *current_value,
                    Plugin_Parameters_Ring_Buffer* ring)
{
    //~ Win32 initialization
    HINSTANCE instance = GetModuleHandle(0);
    
    CoInitializeEx(NULL, COINIT_MULTITHREADED); 
    
    WNDCLASSEX main_class{
        .cbSize        = sizeof main_class,
        .style         = CS_HREDRAW | CS_VREDRAW | CS_OWNDC,
        .lpfnWndProc   = WindowProc,
        .hInstance     = instance,
        .hCursor       = LoadCursor(NULL, IDC_ARROW),
        .lpszClassName = "Main Class"
    };
    RegisterClassEx(&main_class);
    
    
    GraphicsContext graphics_ctx = {
        .font = nullptr,
        //TODO hardcoded
        .window_dim = { 600.0f + TOTAL_WIDTH, 400.0f},
        .draw_vertices = m_allocate_array(Vertex, ATLAS_MAX_VERTEX_COUNT),
        .draw_vertices_count = 0,
        .draw_indices = m_allocate_array(u32, ATLAS_MAX_VERTEX_COUNT), 
        .draw_indices_count = 0
    };
    
    HWND window = CreateWindow("Main Class", "Test", 
                               WS_OVERLAPPEDWINDOW,
                               CW_USEDEFAULT, 
                               CW_USEDEFAULT, 
                               (u32)graphics_ctx.window_dim.x, (u32)graphics_ctx.window_dim.y+ 30,
                               0,0,
                               instance, 
                               &graphics_ctx);
    
    
    //TODO c'est un peu nul comme interface en vrai
    Font jetbrains_mono = load_fonts(DEFAULT_FONT_FILENAME);
    
    OpenGL_Context opengl_ctx = opengl_initialize(window, &jetbrains_mono);
    graphics_ctx.font = &jetbrains_mono;
    
    
    //~ IR initialization
    Plugin_Descriptor& descriptor = handle.descriptor;
    
    real32** IR_buffer = m_allocate_array(real32*, audio_parameters.num_channels);
    for(u32 channel = 0; channel < audio_parameters.num_channels; channel++)
    {
        IR_buffer[channel] = m_allocate_array(real32, IR_BUFFER_LENGTH);
    }
    
    compute_IR(handle, IR_buffer, IR_BUFFER_LENGTH, audio_parameters, current_value);
    
    
    Ipp_Context ipp_ctx = ipp_initialize();
    
    Vec2* fft_out = m_allocate_array(Vec2, IR_BUFFER_LENGTH);
    forward_fft(IR_buffer[0], fft_out, IR_BUFFER_LENGTH, &ipp_ctx);
    
    real32 *magnitudes  = m_allocate_array(real32, IR_BUFFER_LENGTH);
    for(i32 i = 0; i < IR_BUFFER_LENGTH; i++)
        magnitudes[i] = sqrt(fft_out[i].x * fft_out[i].x + fft_out[i].y * fft_out[i].y);
    
    
    graphics_ctx.IR_max_buffer = m_allocate_array(real32, IR_PIXEL_LENGTH);
    graphics_ctx.IR_min_buffer = m_allocate_array(real32, IR_PIXEL_LENGTH);
    graphics_ctx.IR_pixel_count = IR_PIXEL_LENGTH;
    
    graphics_ctx.fft_buffer = m_allocate_array(real32, IR_PIXEL_LENGTH);
    graphics_ctx.fft_pixel_count = IR_PIXEL_LENGTH;
    
    render_fft(magnitudes, IR_BUFFER_LENGTH / 2, graphics_ctx.fft_buffer, IR_PIXEL_LENGTH);
    
    render_IR(IR_buffer, audio_parameters.num_channels, IR_BUFFER_LENGTH, graphics_ctx.IR_min_buffer, graphics_ctx.IR_max_buffer, IR_PIXEL_LENGTH);
    
    //~ 
    ShowWindow(window, 1);
    UpdateWindow(window);
    
    
    IO frame_io
    {
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
    
    UI_State ui_state = {-1};
    
    BOOL done = FALSE;
    MSG message;
    
    i64 last_time = win32_init_timer();
    
    //~ Main Loop
    while(done == FALSE)
    {
        
        frame_io.delete_pressed = false;
        
        while(PeekMessage(&message,0,0,0, PM_REMOVE))
        {
            switch (message.message)
            {
                
                case WM_QUIT : 
                {
                    done = true;
                }break;
                
                case WM_LBUTTONDOWN :
                {
                    frame_io.mouse_down = true;
                    SetCapture(window); 
                }break;
                
                case WM_LBUTTONUP :
                {
                    frame_io.mouse_down = false;
                    if(!frame_io.right_mouse_down)
                        ReleaseCapture(); 
                }break;
                
                
                case WM_RBUTTONDOWN :
                {
                    frame_io.right_mouse_down = true;
                    SetCapture(window); 
                }break;
                
                case WM_RBUTTONUP :
                {
                    frame_io.right_mouse_down = false;
                    if(!frame_io.mouse_down)
                        ReleaseCapture();
                }break;
                
                case WM_KEYDOWN:{
                    if (message.wParam == VK_BACK || message.wParam == VK_DELETE)
                    {
                        frame_io.delete_pressed = true;
                    }
                }break;
            }
            
            TranslateMessage(&message);
            DispatchMessage(&message);
        }
        
        // TODO(octave): hack ?
        if(done)
            break;
        
        //~
        //frame
        
        graphics_ctx.draw_vertices_count = 0;
        graphics_ctx.draw_indices_count = 0;
        
        frame_io = io_state_advance(frame_io);
        frame_io.mouse_position = win32_get_mouse_position(window);
        
        bool parameters_were_tweaked = false;
        
        frame(descriptor, &graphics_ctx, ui_state, frame_io, 
              /*IR_min_buffer, IR_max_buffer, IR_PIXEL_LENGTH,*/ 
              current_value, parameters_were_tweaked);
        
        
        if(parameters_were_tweaked)
        {
            plugin_parameters_buffer_push(*ring, current_value);
            compute_IR(handle, IR_buffer, IR_BUFFER_LENGTH, audio_parameters, current_value);
            
            Vec2* fft_out = m_allocate_array(Vec2, IR_BUFFER_LENGTH);
            forward_fft(IR_buffer[0], fft_out, IR_BUFFER_LENGTH, &ipp_ctx);
            
            real32 *magnitudes  = m_allocate_array(real32, IR_BUFFER_LENGTH);
            for(i32 i = 0; i < IR_BUFFER_LENGTH; i++)
                magnitudes[i] = sqrt(fft_out[i].x * fft_out[i].x + fft_out[i].y * fft_out[i].y);
            
            render_IR(IR_buffer, audio_parameters.num_channels, IR_BUFFER_LENGTH, graphics_ctx.IR_min_buffer, graphics_ctx.IR_max_buffer, IR_PIXEL_LENGTH);
            render_fft(magnitudes, IR_BUFFER_LENGTH / 2, graphics_ctx.fft_buffer, IR_PIXEL_LENGTH);
            
        }
        
        opengl_render_ui(&opengl_ctx, &graphics_ctx);
        
        i64 current_time;
        win32_pace_60_fps(last_time, &current_time, &frame_io.delta_time);
        last_time = current_time;
    }
    
    opengl_uninitialize(&opengl_ctx);
}
