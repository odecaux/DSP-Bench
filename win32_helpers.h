/* date = February 25th 2022 11:12 am */

#ifndef WIN32_HELPERS_H
#define WIN32_HELPERS_H

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
    
    assert(file_size < 0x7FFFFFFFFFFFFFFF);
    
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
    u64 file_size = file_size_quad.QuadPart; assert(file_size <= 0xFFFFFFFF);
    
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
    assert(max_buffer_size > 0);
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

#endif //WIN32_HELPERS_H
