/* date = November 29th 2021 10:08 am */

#ifndef COMILER_H
#define COMILER_H

#ifdef DEBUG 
extern "C" __declspec(dllexport)
#endif
void* create_llvm_context();

#ifdef DEBUG 
extern "C" __declspec(dllexport)
#endif
Plugin try_compile(const char* filename, void* llvm_ctx_ptr, Arena *allocator, void* previous_clang_ctx);

#ifdef DEBUG
extern "C" __declspec(dllexport) 
#endif
void release_jit(Plugin *plugin);


#ifdef DEBUG 
extern "C" __declspec(dllexport)
#endif
void release_llvm_ctx(void* llvm_ctx_void);


#endif //COMILER_H
