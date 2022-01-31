//#include "stdlib.h"
#include "assert.h"
#include "stdio.h"
#include "math.h"

#include "windows.h"
#include <GL/GL.h>

#define STB_TRUETYPE_IMPLEMENTATION 
#include "stb_truetype.h"

#include "base.h"
#include "structs.h"



//~ OpenGL defines
#define GL_VERTEX_SHADER                  0x8B31
#define GL_FRAGMENT_SHADER                0x8B30
#define GL_ARRAY_BUFFER                   0x8892
#define GL_STATIC_DRAW                    0x88E4


typedef GLuint(*glCreateProgram_t) (void);
static glCreateProgram_t glCreateProgram = nullptr;

typedef GLuint(*glCreateShader_t)(GLenum);
static glCreateShader_t glCreateShader = nullptr;

typedef void (*glDeleteProgram_t) (GLuint program);
static glDeleteProgram_t  glDeleteProgram = nullptr;

typedef void (*glDeleteShader_t) (GLuint shader);
static glDeleteShader_t  glDeleteShader = nullptr;

typedef void (*glCompileShader_t) (GLuint shader);
static glCompileShader_t  glCompileShader = nullptr;

typedef void (*glLinkProgram_t) (GLuint program);
static glLinkProgram_t  glLinkProgram = nullptr;

typedef void (*glGenBuffers_t) (GLsizei n, GLuint *buffers);
static glGenBuffers_t  glGenBuffers = nullptr;

typedef void (*glBindBuffer_t) (GLenum target, GLuint buffer);
static glBindBuffer_t  glBindBuffer = nullptr;

typedef void (*glShaderSource_t)(GLuint shader, GLsizei count, const char *const* string, const GLint *length);
static glShaderSource_t glShaderSource = nullptr;

typedef void (*glBufferData_t) (GLenum target, i64 size, const void *data, GLenum usage);
static glBufferData_t  glBufferData = nullptr;

typedef void(*glAttachShader_t) (GLuint program, GLuint shader);
static glAttachShader_t glAttachShader = nullptr;

typedef void (*glGenVertexArrays_t) (GLsizei n, GLuint *arrays);
static glGenVertexArrays_t glGenVertexArrays = nullptr;

typedef void (*glBindVertexArray_t) (GLuint array);
static glBindVertexArray_t glBindVertexArray = nullptr;

typedef void (*glUseProgram_t) (GLuint program);
static glUseProgram_t glUseProgram = nullptr;

typedef void (*glVertexAttribPointer_t) (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer);
static glVertexAttribPointer_t glVertexAttribPointer = nullptr;

typedef void (*glEnableVertexAttribArray_t) (GLuint index);
static glEnableVertexAttribArray_t glEnableVertexAttribArray = nullptr;

#define LOAD_GL(function_name) function_name = (function_name##_t) wglGetProcAddress(#function_name); if(!function_name) { printf("failed to load" #function_name "\n"); return false; }

bool load_opengl_functions()
{
    LOAD_GL(glEnableVertexAttribArray);
    LOAD_GL(glCreateProgram);
    LOAD_GL(glCreateShader);
    LOAD_GL(glDeleteProgram);
    LOAD_GL(glDeleteShader);
    LOAD_GL(glCompileShader);
    LOAD_GL(glLinkProgram);
    LOAD_GL(glGenBuffers);
    LOAD_GL(glBindBuffer);
    LOAD_GL(glShaderSource);
    LOAD_GL(glBufferData);
    LOAD_GL(glAttachShader);
    LOAD_GL(glGenVertexArrays);
    LOAD_GL(glBindVertexArray);
    LOAD_GL(glUseProgram);
    LOAD_GL(glVertexAttribPointer);
    return true;
}


//#define log printf
#define log noop_log

int noop_log(const char *__restrict __format, ...){
    return 1;
}

typedef struct
{
    u64 frame_count;
    
    real32 delta_time;
    real32 time;
    
    bool mouse_down;
    bool mouse_clicked;
    bool mouse_released;
    
    bool right_mouse_down;
    bool mouse_double_clicked;
    bool right_mouse_clicked;
    
    bool delete_pressed;
    
    Vec2 mouse_position;
    Vec2 mouse_pos_prev;
    Vec2 mouse_delta;
    
    real32 mouse_double_click_time;
    real32 mouse_down_time;
    real32 right_mouse_down_time;
    real32 mouse_clicked_time;
} IO;



enum Color
{
    Color_Back,
    Color_Front
};


typedef struct{
    i64 selected_parameter_idx;
} UI_State;


typedef struct {
    Vec2 *vertex_buffer;
    u64 used_triangles;
} GraphicsContext;

void push_triangle(Vec2 a, Vec2 b, Vec2 c, GraphicsContext *ctx)
{
    /*ctx->vertex_buffer[ctx->used_triangles * 3] = a;
        ctx->vertex_buffer[ctx->used_triangles * 3 + 1] = b;
        ctx->vertex_buffer[ctx->used_triangles * 3 + 2] = c;
        */
    
    ctx->vertex_buffer[ctx->used_triangles * 3] = {0.0f, 0.0f};
    ctx->vertex_buffer[ctx->used_triangles * 3 + 1] = {0.0f, 0.5f};
    ctx->vertex_buffer[ctx->used_triangles * 3 + 2] = {0.5f, 0.5f};
    
    ctx->used_triangles++;
    
}

static real32 total_width = 200.0f;
static real32 title_height = 40.0f;
static real32 field_height = 30.0f;
static real32 field_title_height = 15.0f;
//static real32 outer_margin = 10.0f;
static real32 field_margin = 5.0f;
static real32 cursor_size = 10.0f;
static real32 min_max_label_width = 40.0f;

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

i64 win32_pace_60_fps(i64 last_time, LARGE_INTEGER counter_frequency, real32* delta_time) // TODO(octave): refactor variable names
{
    LARGE_INTEGER render_end;
    QueryPerformanceCounter(&render_end);
    u64 render_elapsed  = render_end.QuadPart - last_time;
    auto ms_render = ((1000000.0f*render_elapsed) / counter_frequency.QuadPart) / 1000.0f;
    int ceiled = ceil(ms_render);
    
    if(50-ceiled  > 0)
    {
        Sleep(50 - ceiled);
    }
    
    LARGE_INTEGER frame_end;
    QueryPerformanceCounter(&frame_end);
    auto frame_elapsed  = frame_end.QuadPart - last_time;
    auto ms_frame = ((1000000.0f*frame_elapsed) / counter_frequency.QuadPart) / 1000.0f;
    *delta_time = ms_frame;
    ceiled = floor(ms_frame);
    return frame_end.QuadPart;
}



void win32_draw_rectangle(Rect bounds, Color color, GraphicsContext& graphics_ctx)
{
    /* auto d2d_bounds  = to_d2d_rect(bounds);
    auto brush = win32_color_to_brush(color, graphics_ctx);
    graphics_ctx.render_target->DrawRectangle(&d2d_bounds, brush);

*/
}
void win32_fill_rectangle(Rect bounds, Color color, GraphicsContext& graphics_ctx)
{
    //auto brush = win32_color_to_brush(color, graphics_ctx);
    //graphics_ctx.render_target->FillRectangle(&d2d_bounds, brush);
    auto top_left = bounds.origin;
    auto top_right = Vec2{ bounds.origin.x + bounds.dim.x, bounds.origin.y };
    auto bottom_right = Vec2{ bounds.origin.x + bounds.dim.x, bounds.origin.y + bounds.dim.y};
    auto bottom_left = Vec2{ bounds.origin.x, bounds.origin.y + bounds.dim.y};
    
    push_triangle(top_left, top_right, bottom_left, &graphics_ctx);
    push_triangle(bottom_left, top_right, bottom_right, &graphics_ctx);
}

void win32_draw_text(const String& text, Rect bounds, Color color, GraphicsContext& graphics_ctx)
{
    /*auto brush = win32_color_to_brush(color, graphics_ctx);
    // TODO(octave): verifier que y a pas de bug ici
    WCHAR dest[512] = {0};
    MultiByteToWideChar(CP_UTF8, 0,text.str, (i32)text.size,dest,511);
    graphics_ctx.render_target->DrawText(dest, text.size,  graphics_ctx.text_format,to_d2d_rect(bounds), brush); 
    */
}

void win32_draw_line(Vec2 start, Vec2 end, Color color, real32 width, GraphicsContext& graphics_ctx)
{
    /*auto brush = win32_color_to_brush(color, graphics_ctx);
    graphics_ctx.render_target->DrawLine({start.x, start.y}, {end.x, end.y}, brush, width);
    */
}

void draw_slider(Rect slider_bounds, 
                 real32 normalized_value, 
                 GraphicsContext& graphics_ctx)
{
    real32 slider_x = slider_bounds.origin.x + normalized_value * slider_bounds.dim.x; 
    Rect slider_rect = {
        Vec2{slider_x, slider_bounds.origin.y},
        Vec2{5.0f, slider_bounds.dim.y}
    };
    win32_fill_rectangle(slider_rect, Color_Front, graphics_ctx);
    
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

// TODO(octave): rename, what does this function actually do ? it's not really buffers
void render_IR(real32** IR_buffer, u32 channel_count, u32 IR_length, real32* min_buffer, real32* max_buffer, u32 pixel_count)
{
    memset(min_buffer, 0, pixel_count * sizeof(real32));
    memset(max_buffer, 0, pixel_count * sizeof(real32));
    
    for(u32 channel = 0; channel < channel_count; channel++)
    {
        for(u32 sample = 0; sample < IR_length; sample++)
        {
            u32 pixel_idx = sample * pixel_count / IR_length;
            real32 value = IR_buffer[channel][sample];
            if(value > max_buffer[pixel_idx])
                max_buffer[pixel_idx] = value;
            if(value < min_buffer[pixel_idx])
                min_buffer[pixel_idx] = value;
        }
    }
}

void win32_draw_IR(Rect bounds,
                   real32* IR_min_buffer,
                   real32* IR_max_buffer,
                   u32 IR_pixel_count,
                   GraphicsContext& graphics_ctx)
{
    real32 middle_y = bounds.origin.y  + bounds.dim.y / 2.0f;
    real32 half_h = bounds.dim.y / 2.0f;
    
    for(u32 i = 0; i < IR_pixel_count; i++)
    {
        Vec2 start = {bounds.origin.x + i, middle_y};
        Vec2 end_max = {bounds.origin.x + i, middle_y - IR_max_buffer[i] * half_h};
        Vec2 end_min = {bounds.origin.x + i, middle_y - IR_min_buffer[i] * half_h};
        win32_draw_line(start, end_max, Color_Front, 1.0f, graphics_ctx);
        win32_draw_line(start, end_min, Color_Front, 1.0f, graphics_ctx);
    }
}

void compute_IR(Plugin_Handle& handle, 
                real32** IR_buffer, 
                u32 IR_length, 
                Audio_Parameters& audio_parameters,
                Plugin_Parameter_Value* current_parameters_values)
{
    for(u32 channel = 0; channel < audio_parameters.num_channels; channel++){
        IR_buffer[channel][0] = 1.0f;
        for(u32 sample = 1; sample < IR_length; sample++)
        {
            IR_buffer[channel][sample] = 0.0f;
        }
    }
    
    char* IR_parameters_holder = (char*) malloc(handle.descriptor.parameters_struct.size);
    char* IR_state_holder = (char*) malloc(handle.descriptor.state_struct.size);
    
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
    
    delete IR_parameters_holder;
    delete IR_state_holder;
}

void frame(Plugin_Descriptor& descriptor, 
           GraphicsContext& graphics_ctx, 
           UI_State& ui_state, 
           IO frame_io, 
           real32* IR_min_buffer,
           real32* IR_max_buffer,
           u32 IR_pixel_count,
           Plugin_Parameter_Value* current_parameter_values,
           bool& parameters_were_tweaked)
{
    
    if(frame_io.mouse_released)
    {
        ui_state.selected_parameter_idx = -1;
    }
    
    Rect title_bounds = { 
        Vec2{0.0f, 0.0f}, 
        Vec2{total_width, title_height}
    };
    win32_draw_rectangle(title_bounds, Color_Front, graphics_ctx);
    win32_draw_text(StringLit("gain.cpp"), title_bounds, Color_Front, graphics_ctx);
    
    Vec2 position = {0.0f, title_height};
    
    
    for(u32 parameter_idx = 0; parameter_idx < descriptor.num_parameters; parameter_idx++)
    {
        position.y += field_margin * 2;
        
        auto& current_parameter_value = current_parameter_values[parameter_idx];
        auto& parameter_descriptor = descriptor.parameters[parameter_idx];
        
        Rect field_title_bounds = {
            position, {total_width, field_title_height}
        };
        
        win32_draw_text(parameter_descriptor.name, field_title_bounds, Color_Front, graphics_ctx);
        //win32_draw_rectangle(field_title_bounds, Color_Front, graphics_ctx);
        position.y += field_title_height + field_margin;
        
        Rect field_bounds = {
            position,
            {total_width, field_height}
        };
        
        Rect slider_bounds = rect_remove_padding(field_bounds, min_max_label_width, 0.0f);
        
        Rect min_label_bounds = {
            {0.0f, position.y},
            {min_max_label_width, field_height}
        };
        
        
        Rect max_label_bounds = {
            {0.0f + total_width - min_max_label_width, position.y},
            {min_max_label_width, field_height}
        };
        
        win32_draw_rectangle(min_label_bounds, Color_Front, graphics_ctx);
        
        win32_draw_rectangle(slider_bounds, Color_Front, graphics_ctx);
        win32_draw_rectangle(max_label_bounds, Color_Front, graphics_ctx);
        
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
                
                win32_draw_text(value.name, slider_bounds, Color_Front, graphics_ctx);
                
                if(should_update_this_parameter)
                {
                    auto new_index = denormalize_enum_index(parameter_descriptor.enum_param, new_normalized_value);
                    auto new_value = enum_index_to_value(parameter_descriptor.enum_param, new_index);
                    current_parameter_value.enum_value = new_value;
                }
            }break;
        }
        
        position.y += field_height;
    }
    
    
    
    //~ 
    //IR
    {
        Rect IR_title_bounds = {
            {total_width + 10.0f, 5.0f},
            {200.0f, 10.0f}
        };
        
        
        win32_draw_text(StringLit("Impulse Response"), IR_title_bounds, Color_Front, graphics_ctx); 
        
        Rect IR_outer_bounds{
            {total_width, 10.0f},
            {500.0f, 150.0f}
        };
        auto IR_inner_bounds = rect_remove_padding(IR_outer_bounds, 10.0f, 10.0f);
        win32_draw_rectangle(IR_inner_bounds, Color_Front, graphics_ctx);
        win32_draw_IR(IR_inner_bounds, IR_min_buffer, IR_max_buffer, IR_pixel_count, graphics_ctx);
        
        
    }
    
    
    //~ 
    
    {
        
        Rect IR_title_bounds = {
            {total_width + 10.0f, 160.0f},
            {200.0f, 10.0f}
        };
        
        
        win32_draw_text(StringLit("Frequency Response"), IR_title_bounds, Color_Front, graphics_ctx); 
        
        Rect IR_outer_bounds{
            {total_width, 170.0f},
            {500.0f, 150.0f}
        };
        auto IR_inner_bounds = rect_remove_padding(IR_outer_bounds, 10.0f, 10.0f);
        win32_draw_rectangle(IR_inner_bounds, Color_Front, graphics_ctx);
        
        
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
        
        GraphicsContext *graphics_ctx = (GraphicsContext*)(GetWindowLongPtrW(
                                                                             window,
                                                                             GWLP_USERDATA
                                                                             ));
        
        if(message == WM_SIZE)
        {
            /*
            if(graphics_ctx->render_target != nullptr)
            {
                UINT width = LOWORD(l_param);
                UINT height = HIWORD(l_param);
                graphics_ctx->render_target->Resize(D2D1::SizeU(width, height));
            }
            */
        }
        else if (message == WM_SIZE)
        {
            printf("debug\n");
        }
        else if(message == WM_DESTROY || message == WM_DESTROY || message == WM_QUIT)
        {
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

internal void load_fonts(const char* font_filename)
{
    
    HANDLE handle = CreateFileA(
                                font_filename,
                                GENERIC_READ,
                                0,
                                0,
                                OPEN_EXISTING,
                                FILE_ATTRIBUTE_NORMAL ,
                                0);
    
    
    if(handle == INVALID_HANDLE_VALUE)
    {
        printf("error opening font\n");
        exit(1);
    }
    
    LARGE_INTEGER file_size_quad;
    BOOL result = GetFileSizeEx(handle, &file_size_quad);
    u64 file_size = file_size_quad.QuadPart;
    
    char* font_file_buffer = (char*) malloc(file_size);
    
    auto success = ReadFile(handle, font_file_buffer, file_size, 0, 0);
    if(success == FALSE)
    {
        printf("error reading font file\n");
        exit(1);
    }
    
    
    CloseHandle(handle);
    free(font_file_buffer);
    
}



extern "C" __declspec(dllexport)  void initialize_gui(Plugin_Handle& handle,
                                                      Audio_Parameters& audio_parameters,
                                                      Plugin_Parameter_Value *current_value,
                                                      Plugin_Parameters_Ring_Buffer* ring)
{
    
    //~ IR initialization
    Plugin_Descriptor& descriptor = handle.descriptor;
    
    const u32 IR_length = 44100;
    real32** IR_buffer = (real32**)malloc(sizeof(real32*) * audio_parameters.num_channels);
    for(u32 channel = 0; channel < audio_parameters.num_channels; channel++)
    {
        IR_buffer[channel] = (real32*)malloc(sizeof(real32) * IR_length);
    }
    
    compute_IR(handle, IR_buffer, IR_length, audio_parameters, current_value);
    
    real32* IR_max_buffer = (real32*)malloc(sizeof(real32) * 480);
    real32* IR_min_buffer = (real32*)malloc(sizeof(real32) * 480);
    
    render_IR(IR_buffer, audio_parameters.num_channels, 44100, IR_min_buffer, IR_max_buffer, 480);
    
    //~ Win32 initialization
    HINSTANCE instance = GetModuleHandle(0);
    
    CoInitializeEx(NULL, COINIT_MULTITHREADED); 
    
    
    LARGE_INTEGER counter_frequency;
    QueryPerformanceFrequency(&counter_frequency);
    
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
        (Vec2*) malloc(sizeof(Vec2) * 1024),
        0
    };
    
    
    real32 window_width = 600.0f + total_width;
    real32 window_height = 400.0f;
    HWND window = CreateWindow("Main Class", "Test", 
                               WS_OVERLAPPEDWINDOW,
                               CW_USEDEFAULT, 
                               CW_USEDEFAULT, 
                               (u32)window_width, (u32)window_height + 30,
                               0,0,
                               instance, 
                               &graphics_ctx);
    
    
    //~ Init OpenGL
    
    
    u32 pixel_buffer_size = window_height * window_width * sizeof(u8) * 4;
    u8* pixel_buffer = (u8*) malloc(pixel_buffer_size);
    memset(pixel_buffer, 1, pixel_buffer_size);
    
    
    HDC window_dc = GetDC(window);
    
    
    PIXELFORMATDESCRIPTOR pixel_format =
    {
        sizeof(PIXELFORMATDESCRIPTOR),
        1,
        PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
        PFD_TYPE_RGBA,        
        32,                   
        0, 0, 0, 0, 0, 0,
        0,
        0,
        0,
        0, 0, 0, 0,
        24,                   
        8,                   
        0,
        PFD_MAIN_PLANE,
        0,
        0, 0, 0
    };
    
    int pixel_format_idx = ChoosePixelFormat(window_dc, &pixel_format);
    if(pixel_format_idx == 0)
    {
        printf("couldn't find a valid pixel format\n");
        exit(1);
    }
    
    SetPixelFormat(window_dc, pixel_format_idx, &pixel_format);
    
    HGLRC opengl_rc = wglCreateContext(window_dc);
    if(wglMakeCurrent(window_dc, opengl_rc) == FALSE)
    {
        printf("couldn't open opengl context\n");
        exit(1);
    }
    
    
    if(!load_opengl_functions())
    {
        printf("failed to load opengl functions\n");
        exit(1);
    }
    
    
    const char *vertexShaderSource = 
        "#version 330 core\n"
        "layout (location = 0) in vec2 aPos;\n"
        "void main()\n"
        "{\n"
        "   gl_Position = vec4(aPos.x, aPos.y, 1.0, 1.0);\n"
        "}\0";
    
    unsigned int vertexShader;
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    
    const char *fragmentShaderSource =  
        "#version 330 core\n"
        "out vec4 FragColor;\n"
        "void main()\n"
        "{\n"
        "    FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
        "} \n";
    
    
    unsigned int fragmentShader;
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    
    unsigned int shaderProgram;
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    
    
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    
    
    
    float vertices[] = {
        0.0f, 1.0f,
        0.0f, 0.0f,
        1.0f, 1.0f
    };
    
    unsigned int VBO;
    unsigned int VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    
    glBindVertexArray(VAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    
    ReleaseDC(window, window_dc);
    
    ShowWindow(window, 1);
    UpdateWindow(window);
    
    
    //~ IO initialization
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
    
    UI_State ui_state = { -1 };
    
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
        
        graphics_ctx.used_triangles = 0;
        
        frame_io = io_state_advance(frame_io);
        frame_io.mouse_position = win32_get_mouse_position(window);
        
        bool parameters_were_tweaked = false;
        
        frame(descriptor, graphics_ctx, ui_state, frame_io, IR_min_buffer, IR_max_buffer, 480, current_value, parameters_were_tweaked);
        
        if(parameters_were_tweaked)
        {
            plugin_parameters_buffer_push(*ring, current_value);
            compute_IR(handle, IR_buffer, IR_length, audio_parameters, current_value);
            render_IR(IR_buffer, audio_parameters.num_channels, 44100, IR_min_buffer, IR_max_buffer, 480);
        }
        
        glViewport(0,0, window_width, window_height);
        
        
        
        unsigned int texture = 0;
        static bool init = false;
        if(!init)
        {
            glGenTextures(1, &texture);
            init = true;
        }
        
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D,
                     0,
                     GL_RGBA4,
                     window_width,
                     window_height,
                     0,
                     GL_RGBA,
                     GL_UNSIGNED_BYTE,
                     pixel_buffer
                     );
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);   
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        
        glEnable(GL_TEXTURE_2D);
        
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        
        glMatrixMode(GL_PROJECTION);
        float projection_matrix[] = 
        {
            1.0f / window_width, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f / window_height, 0.0f, 0.0f ,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f,
        };
        glLoadMatrixf(projection_matrix);
        
        //glLoadIdentity();
        
        glUseProgram(shaderProgram);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBindVertexArray(VAO);
        
        glBufferData(GL_ARRAY_BUFFER, sizeof(Vec2) * 1024, graphics_ctx.vertex_buffer, GL_STATIC_DRAW);
        
        glDrawArrays(GL_TRIANGLES, 0, 3);
        
        SwapBuffers(window_dc);
        
        last_time = win32_pace_60_fps(last_time, counter_frequency, &frame_io.delta_time);
        
    }
    
    wglMakeCurrent (NULL, NULL) ; 
    wglDeleteContext (opengl_rc);
    
}
