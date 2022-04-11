/* date = February 25th 2022 10:17 am */

#ifndef WIN32_PLATFORM_H
#define WIN32_PLATFORM_H

LRESULT CALLBACK WindowProc(HWND window, UINT message, WPARAM w_param, LPARAM l_param);

void win32_init_window(Window_Backend_Context *window, Vec2* window_dim, IO *frame_io)
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
    
    window->frame_io = frame_io;
    window->dim = window_dim;
    window->window = CreateWindow("Main Class", "DSP Bench", 
                                  WS_OVERLAPPEDWINDOW,
                                  CW_USEDEFAULT, 
                                  CW_USEDEFAULT, 
                                  (u32)window_dim->x, (u32)window_dim->y + 30,
                                  0,0,
                                  instance, 
                                  window);
}


void win32_message_dispatch(Window_Backend_Context *window_ctx, bool *done)
{
    MSG message;
    IO *frame_io = window_ctx->frame_io;
    
    frame_io->delete_pressed = false;
    frame_io->left_ctrl_pressed = false;
    frame_io->mousewheel_delta = 0.0f;
    
    while(PeekMessage(&message,0,0,0, PM_REMOVE))
    {
        switch (message.message)
        {
            
            case WM_QUIT : 
            {
                *done = true;
            }break;
            
        }
        
        TranslateMessage(&message);
        DispatchMessage(&message);
    }
}

LRESULT CALLBACK WindowProc(HWND window_handle, UINT message, WPARAM w_param, LPARAM l_param)
{
    if (message == WM_CREATE)
    {
        LPCREATESTRUCT pcs = (LPCREATESTRUCT)l_param;
        Window_Backend_Context *window_ctx = (Window_Backend_Context*)pcs->lpCreateParams;
        SetWindowLongPtr(window_handle, GWLP_USERDATA, (LONG_PTR)(window_ctx));
        return 1;
    }
    
    Window_Backend_Context *window_ctx = (Window_Backend_Context*)(GetWindowLongPtrW(window_handle, GWLP_USERDATA));
    
    if(!window_ctx)
    {
        return CallWindowProc(DefWindowProc, window_handle, message, w_param, l_param);
    } 
    
    ensure(window_handle == window_ctx->window);
    IO *frame_io = window_ctx->frame_io;
    
    switch(message)
    {
        case WM_SIZE : 
        {
            UINT width = LOWORD(l_param);
            UINT height = HIWORD(l_param);
            *window_ctx->dim = { (real32)width, (real32)height };
            return 0;
        } break;
        
        case WM_SIZING : 
        {
            RECT rect = *(RECT*)(l_param);
            real32 new_width = (real32)(rect.right - rect.left);
            real32 new_height = (real32)(rect.bottom - rect.top);
            *window_ctx->dim = { new_width, new_height};
            return TRUE;
        } break;
        
        case WM_DESTROY : 
        case WM_QUIT : 
        {
            PostQuitMessage(0);
            return 0;
        } break;
        
        
        case WM_LBUTTONDOWN :
        {
            frame_io->mouse_down = true;
            SetCapture(window_handle); 
            return 0;
        }break;
        
        case WM_LBUTTONUP :
        {
            frame_io->mouse_down = false;
            if(!frame_io->right_mouse_down)
                ReleaseCapture();
            return 0;
        }break;
        
        case WM_RBUTTONDOWN :
        {
            frame_io->right_mouse_down = true;
            SetCapture(window_handle); 
            return 0;
        }break;
        
        case WM_RBUTTONUP :
        {
            frame_io->right_mouse_down = false;
            if(!frame_io->mouse_down)
                ReleaseCapture();
            return 0;
        }break;
        
        case WM_MOUSEWHEEL:
        {
            i16 amount = GET_WHEEL_DELTA_WPARAM(w_param);
            frame_io->mousewheel_delta = (real32) amount / WHEEL_DELTA;
            return 0;
        }break;
        
        case WM_MOUSEHWHEEL:
        {
            i16 amount = GET_WHEEL_DELTA_WPARAM(w_param);
            frame_io->mousewheel_h_delta = (real32) amount / WHEEL_DELTA;
            return 0;
        }break;
        
        //keys 
        
        case WM_KEYDOWN:
        {
            switch(w_param)
            {
                case VK_DELETE :
                case VK_BACK :
                {
                    frame_io->delete_pressed = true;
                }break;
                
                case VK_CONTROL :
                {
                    frame_io->left_ctrl_pressed = true;
                    frame_io->left_ctrl_down = true;
                }break;
            }
            return 0;
        }break;
        
        case WM_KEYUP:
        {
            switch(w_param)
            {
                case VK_CONTROL :
                {
                    frame_io->left_ctrl_down = false;
                }break;
            }
            return 0;
        }break;
        
        default : 
        {
            return CallWindowProc(DefWindowProc, window_handle, message, w_param, l_param); 
        } break;
    }
}

#endif //WIN32_PLATFORM_H
