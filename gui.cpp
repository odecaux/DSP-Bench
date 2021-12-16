#include "stdlib.h"
#include "assert.h"
#include "stdio.h"
#include "math.h"

#include "initguid.h"
#include "windows.h"
#include "windowsx.h"
#include "d2d1.h"
#define  DIRECTINPUT_VERSION 0x0800
#include "Dinput.h"
#include "Dwrite.h"


#include "base.h"
#include "structs.h"


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
    ID2D1Factory* factory;
    ID2D1HwndRenderTarget* render_target;
    IDWriteFactory* write_factory;
    IDWriteTextFormat* text_format;
    ID2D1SolidColorBrush* black_brush;
    ID2D1SolidColorBrush* white_brush;
} GraphicsContext;

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


D2D1_POINT_2F to_d2d_point(Vec2 point)
{
    return {point.x, point.y};
}
D2D1_RECT_F to_d2d_rect(Rect rect)
{
    return D2D1::RectF(rect.origin.x, rect.origin.y, rect.origin.x + rect.dim.x, rect.origin.y + rect.dim.y);
}

ID2D1SolidColorBrush* win32_color_to_brush(Color color, GraphicsContext& graphics_ctx)
{
    ID2D1SolidColorBrush* brush;
    switch(color)
    {
        case Color_Front:
        {
            brush = graphics_ctx.white_brush;
        }break;
        case Color_Back:
        {
            brush = graphics_ctx.black_brush;
        }break;
    }
    return brush;
}

void win32_draw_rectangle(Rect bounds, Color color, GraphicsContext& graphics_ctx)
{
    auto d2d_bounds  = to_d2d_rect(bounds);
    auto brush = win32_color_to_brush(color, graphics_ctx);
    graphics_ctx.render_target->DrawRectangle(&d2d_bounds, brush);
}
void win32_fill_rectangle(Rect bounds, Color color, GraphicsContext& graphics_ctx)
{
    auto d2d_bounds  = to_d2d_rect(bounds);
    auto brush = win32_color_to_brush(color, graphics_ctx);
    graphics_ctx.render_target->FillRectangle(&d2d_bounds, brush);
}

void win32_draw_text(const String& text, Rect bounds, Color color, GraphicsContext& graphics_ctx)
{
    auto brush = win32_color_to_brush(color, graphics_ctx);
    // TODO(octave): verifier que y a pas de bug ici
    WCHAR dest[512] = {0};
    MultiByteToWideChar(CP_UTF8, 0,text.str, (i32)text.size,dest,511);
    graphics_ctx.render_target->DrawText(dest, text.size,  graphics_ctx.text_format,to_d2d_rect(bounds), brush); 
    
}

void win32_draw_line(Vec2 start, Vec2 end, Color color, real32 width, GraphicsContext& graphics_ctx)
{
    auto brush = win32_color_to_brush(color, graphics_ctx);
    graphics_ctx.render_target->DrawLine({start.x, start.y}, {end.x, end.y}, brush, width);
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


void frame(Plugin_Descriptor& descriptor, 
           GraphicsContext& graphics_ctx, 
           UI_State& ui_state, 
           IO frame_io, 
           Plugin_Parameters_Ring_Buffer* ring,
           Plugin_Parameter_Value* current_parameter_values)
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
    win32_draw_text(descriptor.name, title_bounds, Color_Front, graphics_ctx);
    
    Vec2 position = {0.0f, title_height};
    
    bool should_update = false;
    
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
        
        bool should_update_parameter = false;
        real32 new_normalized_value;
        
        if(frame_io.mouse_clicked && rect_contains(slider_bounds, frame_io.mouse_position))
        {
            ui_state.selected_parameter_idx = parameter_idx;
        }
        
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
            should_update_parameter = true;
            should_update = true;
        }
        
        
        
        switch(parameter_descriptor.type)
        {
            case Int :
            {
                int current_value = current_parameter_value.int_value; 
                //int value = int_parameter_extract_value(parameter, plugin_state_holder);
                real32 current_normalized_value = normalize_parameter_int_value(parameter_descriptor.int_param, current_value);
                draw_slider(slider_bounds, current_normalized_value, graphics_ctx);
                
                if(should_update_parameter)
                {
                    auto new_int_value = denormalize_int_value(parameter_descriptor.int_param, new_normalized_value);
                    current_parameter_value.int_value = new_int_value;
                }
            }break;
            case Float : 
            {
                if(should_update_parameter)
                {
                    auto x = 1;
                }
                float current_value = current_parameter_value.float_value;
                real32 current_normalized_value = normalize_parameter_float_value(parameter_descriptor.float_param, current_value);
                draw_slider(slider_bounds, current_normalized_value, graphics_ctx);
                
                if(should_update_parameter)
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
                
                if(should_update_parameter)
                {
                    
                    
                    auto new_index = denormalize_enum_index(parameter_descriptor.enum_param, new_normalized_value);
                    auto new_value = enum_index_to_value(parameter_descriptor.enum_param, new_index);
                    current_parameter_value.enum_value = new_value;
                    
                    
                }
            }break;
        }
        
        position.y += field_height;
    }
    
    if(should_update)
    {
        plugin_parameters_buffer_push(*ring, current_parameter_values);
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
            if(graphics_ctx->render_target != nullptr)
            {
                UINT width = LOWORD(l_param);
                UINT height = HIWORD(l_param);
                graphics_ctx->render_target->Resize(D2D1::SizeU(width, height));
            }
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



extern "C" __declspec(dllexport)  void initialize_gui(Plugin_Descriptor descriptor, 
                                                      Plugin_Parameter_Value *current_value,
                                                      Plugin_Parameters_Ring_Buffer* ring)
{
    HINSTANCE instance = GetModuleHandle(0);
    
    CoInitializeEx(NULL, COINIT_MULTITHREADED); 
    
    //d2d loading
    GraphicsContext graphics_ctx = {};
    {
        auto success  = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED,
                                          __uuidof(graphics_ctx.factory),
                                          (void**)&graphics_ctx.factory);
        assert(graphics_ctx.factory != nullptr && SUCCEEDED(success));
    }
    
    
    LARGE_INTEGER counter_frequency;
    QueryPerformanceFrequency(&counter_frequency);
    
    
    WNDCLASSEX main_class{
        .cbSize        = sizeof main_class,
        .style         = CS_HREDRAW | CS_VREDRAW,
        .lpfnWndProc   = WindowProc,
        .hInstance     = instance,
        .hCursor       = LoadCursor(NULL, IDC_ARROW),
        .lpszClassName = "Main Class"
    };
    RegisterClassEx(&main_class);
    
    HWND window = CreateWindow("Main Class", "Test", 
                               WS_OVERLAPPEDWINDOW,
                               CW_USEDEFAULT, CW_USEDEFAULT, 
                               1000,600,
                               0,0,
                               instance, 
                               &graphics_ctx);
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
    
    UI_State ui_state = { -1 };
    
    BOOL done = FALSE;
    MSG message;
    
    i64 last_time = win32_init_timer();
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
        
        
        //~
        //acquire render resources
        if(graphics_ctx.render_target == nullptr)
        {
            log("recreate target\n");
            RECT rc;
            GetClientRect(window, &rc);
            const D2D1_SIZE_U size = D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top);
            auto success = graphics_ctx.factory->CreateHwndRenderTarget(D2D1::RenderTargetProperties(),
                                                                        D2D1::HwndRenderTargetProperties(window, size, D2D1_PRESENT_OPTIONS_IMMEDIATELY),
                                                                        &graphics_ctx.render_target);
            assert(SUCCEEDED(success));
            
            assert(graphics_ctx.render_target != nullptr);
        }
        if(graphics_ctx.write_factory == nullptr)
        {
            // Create a DirectWrite factory.
            auto hr = DWriteCreateFactory(
                                          DWRITE_FACTORY_TYPE_SHARED,
                                          __uuidof(graphics_ctx.write_factory),
                                          reinterpret_cast<IUnknown **>(&graphics_ctx.write_factory)
                                          );
            assert(SUCCEEDED(hr));
            assert(graphics_ctx.write_factory != nullptr);
        }
        if(graphics_ctx.text_format == nullptr)
        {
            
            static const WCHAR msc_fontName[] = L"Jetbrains Mono";
            static const FLOAT msc_fontSize = 14;
            // Create a DirectWrite text format object.
            auto hr = graphics_ctx.write_factory->CreateTextFormat(
                                                                   msc_fontName,
                                                                   NULL,
                                                                   DWRITE_FONT_WEIGHT_NORMAL,
                                                                   DWRITE_FONT_STYLE_NORMAL,
                                                                   DWRITE_FONT_STRETCH_NORMAL,
                                                                   msc_fontSize,
                                                                   L"", //locale
                                                                   &graphics_ctx.text_format
                                                                   );
            
            assert(SUCCEEDED(hr));
            assert(graphics_ctx.text_format!= nullptr);
            // Center the text horizontally and vertically.
            graphics_ctx.text_format->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
            graphics_ctx.text_format->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
            
        }
        
        if(graphics_ctx.white_brush == nullptr)
        {
            auto success = graphics_ctx.render_target->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White),&graphics_ctx.white_brush);
            assert(SUCCEEDED(success));
        }
        if(graphics_ctx.black_brush == nullptr)
        {
            auto success = graphics_ctx.render_target->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black),&graphics_ctx.black_brush);
            assert(SUCCEEDED(success));
        }
        /*
        real32 temp = fmod(frame_io.time * 0.8f, 1000.0f) / 500.0f;
        if(temp > 1.0f) temp = 2.0F - temp;
        
        graphics_ctx.white_brush->SetColor(D2D1::ColorF(0.54f, 0.91f, 1.0f, temp)); 
        */
        //~
        //frame
        
        frame_io = io_state_advance(frame_io);
        frame_io.mouse_position = win32_get_mouse_position(window);
        graphics_ctx.render_target->BeginDraw();
        graphics_ctx.render_target->SetTransform(D2D1::Matrix3x2F::Identity());
        graphics_ctx.render_target->Clear(D2D1::ColorF(D2D1::ColorF::Black));
        
        
        frame(descriptor, graphics_ctx, ui_state, frame_io, ring, current_value);
        
        ValidateRect(window, NULL);
        
        //~
        //release render resource if needed
        auto result = graphics_ctx.render_target->EndDraw();
        if(result == D2DERR_RECREATE_TARGET)
        {
            log("recreate graphics\n");
            graphics_ctx.render_target->Release();
            graphics_ctx.render_target = nullptr;
            
            graphics_ctx.write_factory->Release();
            graphics_ctx.write_factory = nullptr;
            
            graphics_ctx.text_format->Release();
            graphics_ctx.text_format = nullptr;
            
            graphics_ctx.white_brush->Release();
            graphics_ctx.white_brush = nullptr;
            
            graphics_ctx.black_brush->Release();
            graphics_ctx.black_brush = nullptr;
        }
        
        last_time = win32_pace_60_fps(last_time, counter_frequency, &frame_io.delta_time);
        
    }
    
}
