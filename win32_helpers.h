/* date = February 25th 2022 11:12 am */

#ifndef WIN32_HELPERS_H
#define WIN32_HELPERS_H

#define compare_exchange_8(address, new_value, old_value) (InterlockedCompareExchange((CHAR volatile *) address, new_value, old_value) == old_value)

#define compare_exchange_32(address, new_value, old_value) (InterlockedCompareExchange((LONG volatile *) address, new_value, old_value) == old_value)
#define compare_exchange_64(address, new_value, old_value) (InterlockedCompareExchange64((LONG64 volatile *) address, new_value, old_value) == old_value)
#define compare_exchange_ptr(address, new_value, old_value) (InterlockedCompareExchangePointer((PVOID  volatile *) address, new_value, old_value) == old_value)


#define exchange_8(address, new_value) InterlockedExchange((CHAR volatile *) address, new_value)

#define exchange_32(address, new_value) InterlockedExchange((LONG volatile *) address, new_value)

#define exchange_64(address, new_value) InterlockedExchange64((LONG64 volatile *) address, new_value)

#define exchange_ptr(address, new_value) InterlockedExchangePointer((PVOID  volatile *) address, new_value)


function i64 file_get_size(const char* filename)
{
    HANDLE handle = CreateFileA(filename,
                                GENERIC_READ,
                                0,
                                0,
                                OPEN_EXISTING,
                                FILE_ATTRIBUTE_NORMAL ,
                                0);
    
    if(handle == INVALID_HANDLE_VALUE)
    {
        //printf("error opening font file\n");
        return -1;
    }
    
    LARGE_INTEGER file_size_quad;
    
    //TODO check result
    BOOL result = GetFileSizeEx(handle, &file_size_quad);
    u64 file_size = file_size_quad.QuadPart;
    
    CloseHandle(handle);
    
    octave_assert(file_size < 0x7FFFFFFFFFFFFFFF);
    
    return (i64)file_size;
}

//TODO cleanup
function bool load_file_to_memory(const char* filename, u8* out_buffer)
{
    HANDLE handle = CreateFileA(filename,
                                GENERIC_READ,
                                0,
                                0,
                                OPEN_EXISTING,
                                FILE_ATTRIBUTE_NORMAL ,
                                0);
    
    if(handle == INVALID_HANDLE_VALUE)
    {
        //printf("error opening font file\n");
        return false;
    }
    
    LARGE_INTEGER file_size_quad;
    //TODO check result
    BOOL result = GetFileSizeEx(handle, &file_size_quad);
    u64 file_size = file_size_quad.QuadPart; octave_assert(file_size <= 0xFFFFFFFF);
    
    auto success = ReadFile(handle, out_buffer, (u32)file_size, 0, 0);
    if(success == FALSE)
    {
        return false;
    }
    
    CloseHandle(handle);
    return true;
}

function bool win32_open_file(char *out_buffer, u32 max_buffer_size, char *filter)
{
    octave_assert(max_buffer_size > 0);
    out_buffer[0] = '\0';
    OPENFILENAME open_file_ctx = {
        .lStructSize = sizeof(open_file_ctx),
        .hwndOwner = NULL ,
        .lpstrFilter = filter,
        .nFilterIndex = 0,
        .lpstrFile = out_buffer,
        .nMaxFile = max_buffer_size,
        .lpstrFileTitle = NULL,
        .nMaxFileTitle = 0,
        .lpstrInitialDir = NULL,
        .Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST,
    };
	if(GetOpenFileName(&open_file_ctx) == TRUE)
        return true;
    else 
        return false;
}

typedef struct Window_Context{
    HWND window;
} Window_Context;

typedef struct {
    bool worked;
    char* filename;
    HANDLE handle;
    u64 last_write_time;
} File_Change_Listener ;

function File_Change_Listener win32_init_file_change_listener(char *filename) 
{
    HANDLE handle = CreateFileA(filename,
                                FILE_READ_ATTRIBUTES,
                                FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                                nullptr,
                                OPEN_EXISTING,
                                FILE_ATTRIBUTE_NORMAL,
                                nullptr);
    
    if(handle == INVALID_HANDLE_VALUE)
    {
        return {false};
    }
    FILETIME win32_write_time;
    BOOL result = GetFileTime(handle,
                              nullptr,
                              nullptr,
                              &win32_write_time);
    ULARGE_INTEGER temp_large_int { .u = { .LowPart = win32_write_time.dwLowDateTime, .HighPart = win32_write_time.dwHighDateTime}} ;
    
    u64 write_time = temp_large_int.QuadPart;
    
    return File_Change_Listener{
        .worked = true,
        .filename = filename,
        .handle = handle,
        .last_write_time = write_time,
    };
}

function bool win32_query_file_change(File_Change_Listener *listener)
{
    if(!listener->worked)
        return false;
    FILETIME win32_write_time;
    BOOL result = GetFileTime(listener->handle,
                              nullptr,
                              nullptr,
                              &win32_write_time);
    if(result == 0)
        return false;
    
    ULARGE_INTEGER temp_large_int { .u = { .LowPart = win32_write_time.dwLowDateTime, .HighPart = win32_write_time.dwHighDateTime}} ;
    u64 write_time = temp_large_int.QuadPart;
    if(listener->last_write_time < write_time){
        listener->last_write_time = write_time;
        return true;
    }
    else{
        return false;
    }
}

function Vec2 win32_get_mouse_position(Window_Context *window_ctx)
{
    POINT mouse_position;
    GetCursorPos(&mouse_position);
    ScreenToClient(window_ctx->window, &mouse_position);
    return Vec2{(real32)mouse_position.x, (real32)mouse_position.y};
}


function void win32_error_window(const String title, const String text)
{
    MessageBox(0, text.str, title.str, MB_OK);
}


function i64 win32_get_time()
{
    LARGE_INTEGER time; QueryPerformanceCounter(&time);
    return (i64) time.QuadPart;
}


function void win32_get_elapsed_ms_since(i64 last_time, i64 *current_time, real32 *delta_time)
{
    LARGE_INTEGER counter_frequency; QueryPerformanceFrequency(&counter_frequency);
    LARGE_INTEGER frame_end; QueryPerformanceCounter(&frame_end);
    i64 elapsed_tick  = frame_end.QuadPart - last_time;
    i64 elapsed_milli = ((1000000*elapsed_tick) / counter_frequency.QuadPart) / 1000;
    if(delta_time) *delta_time = (real32)elapsed_milli;
    if(current_time) *current_time = frame_end.QuadPart;
}


function void win32_print_elapsed(i64 last_time, const char* text)
{
    real32 elapsed_time;
    win32_get_elapsed_ms_since(last_time, nullptr, &elapsed_time);
    //printf("%s : %f\n", text, elapsed_time);
}

#endif //WIN32_HELPERS_H
