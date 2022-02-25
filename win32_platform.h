/* date = February 25th 2022 10:17 am */

#ifndef WIN32_PLATFORM_H
#define WIN32_PLATFORM_H


typedef struct Window_Context{
    HWND window;
} Window_Context;


Vec2 win32_get_mouse_position(Window_Context *window_ctx)
{
    POINT mouse_position;
    GetCursorPos(&mouse_position);
    ScreenToClient(window_ctx->window, &mouse_position);
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



LRESULT CALLBACK WindowProc(HWND window, UINT message, WPARAM w_param, LPARAM l_param)
{
    LRESULT result = 0;
    
    
    if (message == WM_CREATE)
    {
        LPCREATESTRUCT pcs = (LPCREATESTRUCT)l_param;
        Vec2 *window_dim = (Vec2*)pcs->lpCreateParams;
        
        SetWindowLongPtr(
                         window,
                         GWLP_USERDATA,
                         (LONG_PTR)(window_dim)
                         );
        
        result = 1;
    }
    else {
        
        Vec2 *window_dim = (Vec2*)(GetWindowLongPtrW(window, GWLP_USERDATA));
        
        if(message == WM_SIZE)
        {
            UINT width = LOWORD(l_param);
            UINT height = HIWORD(l_param);
            *window_dim = { (real32)width, (real32)height };
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

Window_Context win32_init_window(Vec2* window_dim)
{
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
    
    
    HWND window = CreateWindow("Main Class", "Test", 
                               WS_OVERLAPPEDWINDOW,
                               CW_USEDEFAULT, 
                               CW_USEDEFAULT, 
                               (u32)window_dim->x, (u32)window_dim->y + 30,
                               0,0,
                               instance, 
                               window_dim);
    
    ShowWindow(window, 1);
    UpdateWindow(window);
    
    return { window };
}

void win32_message_dispatch(Window_Context *window_ctx, IO *frame_io, bool *done)
{
    MSG message;
    HWND window = window_ctx->window;
    
    frame_io->delete_pressed = false;
    
    while(PeekMessage(&message,0,0,0, PM_REMOVE))
    {
        switch (message.message)
        {
            
            case WM_QUIT : 
            {
                *done = true;
            }break;
            
            case WM_LBUTTONDOWN :
            {
                frame_io->mouse_down = true;
                SetCapture(window); 
            }break;
            
            case WM_LBUTTONUP :
            {
                frame_io->mouse_down = false;
                if(!frame_io->right_mouse_down)
                    ReleaseCapture(); 
            }break;
            
            
            case WM_RBUTTONDOWN :
            {
                frame_io->right_mouse_down = true;
                SetCapture(window); 
            }break;
            
            case WM_RBUTTONUP :
            {
                frame_io->right_mouse_down = false;
                if(!frame_io->mouse_down)
                    ReleaseCapture();
            }break;
            
            case WM_KEYDOWN:{
                if (message.wParam == VK_BACK || message.wParam == VK_DELETE)
                {
                    frame_io->delete_pressed = true;
                }
            }break;
        }
        
        TranslateMessage(&message);
        DispatchMessage(&message);
    }
}

#endif //WIN32_PLATFORM_H
