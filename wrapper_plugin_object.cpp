
#define CUSTOM_COMPILER_ERROR_FLAG(flag)
#define CUSTOM_RUNTIME_ERROR_FLAG(flag) flag,
enum Runtime_Error{
#include "errors.inc"
};
#undef CUSTOM_COMPILER_ERROR_FLAG
#undef CUSTOM_RUNTIME_ERROR_FLAG

extern "C" {
    
    typedef unsigned __int64 size_t;
    typedef __int64 ptrdiff_t;
    typedef __int64 intptr_t;
    
    //TODO align
    struct SETJMP_FLOAT128 
    {
        unsigned __int64 Part[2];
    } __attribute__((aligned (16))) ;
    
    typedef SETJMP_FLOAT128 _JBTYPE;
    
    typedef struct _JUMP_BUFFER
    {
        unsigned __int64 Frame;
        unsigned __int64 Rbx;
        unsigned __int64 Rsp;
        unsigned __int64 Rbp;
        unsigned __int64 Rsi;
        unsigned __int64 Rdi;
        unsigned __int64 R12;
        unsigned __int64 R13;
        unsigned __int64 R14;
        unsigned __int64 R15;
        unsigned __int64 Rip;
        unsigned long MxCsr;
        unsigned short FpCsr;
        unsigned short Spare;
        
        SETJMP_FLOAT128 Xmm6;
        SETJMP_FLOAT128 Xmm7;
        SETJMP_FLOAT128 Xmm8;
        SETJMP_FLOAT128 Xmm9;
        SETJMP_FLOAT128 Xmm10;
        SETJMP_FLOAT128 Xmm11;
        SETJMP_FLOAT128 Xmm12;
        SETJMP_FLOAT128 Xmm13;
        SETJMP_FLOAT128 Xmm14;
        SETJMP_FLOAT128 Xmm15;
    } _JUMP_BUFFER;
    
    typedef _JBTYPE jmp_buf[16];
    int __cdecl _setjmp(jmp_buf _Buf);
    
    __attribute__((noreturn)) void longjmp(jmp_buf _Buf, int _Value);
    
    //~
    
    static jmp_buf buf;
    static Runtime_Error error;
    
    void initialize_state_type_wrapper(void* parameters_ptr, void* out_initial_state_ptr, unsigned int num_channels, float sample_rate, void *allocator);
    
    float *plugin_allocate_buffer_app(int num_sample, void* initializer);
    __cdecl float *allocate_buffer(int num_samples, void* initializer)
    {
        float *buffer = plugin_allocate_buffer_app(num_samples, initializer);
        if(buffer == 0)
        {
            error = Runtime_Low_Memory;
            longjmp(buf, 1);
        }
        return buffer;
    }
    
    float **plugin_allocate_buffers_app(int num_samples, int num_channels, void* initializer);
    __cdecl float **allocate_buffers(int num_samples, int num_channels, void* initializer)
    {
        float **buffers = plugin_allocate_buffers_app(num_samples, num_channels, initializer);
        if(buffers == 0)
        {
            error = Runtime_Low_Memory;
            longjmp(buf, 1);
        }
        return buffers; 
    }
    
    void *plugin_allocate_bytes_app(int num_sample, void* initializer);
    __cdecl void *allocate_bytes(int num_bytes, void* initializer)
    {
        void *bytes = plugin_allocate_bytes_app(num_bytes, initializer);
        if(bytes == 0)
        {
            error = Runtime_Low_Memory;
            longjmp(buf, 1);
        }
        return bytes;
    }
    
    
    __cdecl int initialize_state_error_wrapper(void* parameters_ptr, void* out_initial_state_ptr, unsigned int num_channels, float sample_rate, void *allocator)
    {
        error = Runtime_No_Error;
        if(_setjmp(buf) == 0)
        {
            initialize_state_type_wrapper(parameters_ptr, out_initial_state_ptr, num_channels,
                                          sample_rate, allocator);
            return Runtime_No_Error;
        }
        else 
        {
            return Runtime_Low_Memory;
        }
    }
}