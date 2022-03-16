/* date = November 29th 2021 10:08 am */

#ifndef COMILER_H
#define COMILER_H

#ifdef DEBUG 
extern "C" __declspec(dllexport)
#endif
void* create_clang_context();

#ifdef DEBUG 
extern "C" __declspec(dllexport)
#endif
Plugin try_compile(const char* filename, void* clang_ctx_ptr, Plugin_Allocator *allocator);

#ifdef DEBUG
extern "C" __declspec(dllexport) 
#endif
void release_jit(Plugin *plugin);


#ifdef DEBUG 
extern "C" __declspec(dllexport)
#endif
void release_clang_ctx(void* clang_ctx_void);


#endif //COMILER_H
