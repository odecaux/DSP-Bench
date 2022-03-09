/* date = February 25th 2022 10:17 am */

#ifndef WIN32_PLATFORM_H
#define WIN32_PLATFORM_H


LRESULT CALLBACK WindowProc(HWND window, UINT message, WPARAM w_param, LPARAM l_param)
{
    LRESULT result = 0;
    
    
    if (message == WM_CREATE)
    {
        LPCREATESTRUCT pcs = (LPCREATESTRUCT)l_param;
        Vec2 *window_dim = (Vec2*)pcs->lpCreateParams;
        SetWindowLongPtr(window, GWLP_USERDATA, (LONG_PTR)(window_dim));
        
        result = 1;
    }
    else {
        
        Vec2 *window_dim = (Vec2*)(GetWindowLongPtrW(window, GWLP_USERDATA));
        
        if(message == WM_SIZE)
        {
            UINT width = LOWORD(l_param);
            UINT height = HIWORD(l_param);
            *window_dim = { (real32)width, (real32)height };
            return 0;
        }
        
        else if(message == WM_SIZING)
        {
            RECT rect = *(RECT*)(l_param);
            real32 new_width = (real32)(rect.right - rect.left);
            real32 new_height = (real32)(rect.bottom - rect.top);
            *window_dim = { new_width, new_height};
            return TRUE;
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
    
    return { window };
}

void win32_message_dispatch(Window_Context *window_ctx, IO *frame_io, bool *done)
{
    MSG message;
    HWND window = window_ctx->window;
    
    frame_io->delete_pressed = false;
    frame_io->left_ctrl_pressed = false;
    
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
                switch(message.wParam)
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
            }break;
            
            case WM_KEYUP:{
                switch(message.wParam)
                {
                    case VK_CONTROL :
                    {
                        frame_io->left_ctrl_down = false;
                    }break;
                }
            }break;
        }
        
        TranslateMessage(&message);
        DispatchMessage(&message);
    }
}

#endif //WIN32_PLATFORM_H
