/* date = February 23rd 2022 3:13 pm */

#ifndef MEMORY_H
#define MEMORY_H

//
//typedef struct {
//struct Memory_Log_Node* next;
//
//void* address;
//char* filename;
//i32 line;
//i32 tag_int;
//char* tag_string;
//} Memory_Log_Node;
//
//typedef struct{
//Memory_Log_Node *head;
//Memory_Log_Node *tail;
//Memory_Log_Node *free_list;
//} Memory_Log;
//


struct Arena{
    char *base;
    char *current; 
    u64 capacity;
}; 


#define __FILENAME__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)

//#define LOG_ALLOCATIONS 1

#ifdef LOG_ALLOCATIONS

static void* instrumented_malloc(size_t size, const char* file, int line)
{
    printf("malloc : %llu bytes at %s:%d\n", size, file, line);
    return malloc(size);
}


static void* instrumented_array_malloc(size_t count, size_t size, const char* type, const char* file, int line)
{
    printf("malloc : %llu * %s at %s:%d\n", count, type, file, line);
    return malloc(count * size);
}

static void instrumented_free(void* address, const char* file, int line)
{
    printf("free : %s:%d\n", file, line);
    free(address);
}


#define m_allocate(size, tag) instrumented_malloc((size), __FILENAME__, __LINE__)
#define m_allocate_array(type, count, tag) ((type) *)instrumented_array_malloc(count, sizeof(type), #type,__FILENAME__, __LINE__)
#define m_free(address, tag) instrumented_free(address, __FILENAME__, __LINE__)

#else 

#define m_allocate(size, tag) malloc((size))
#define m_allocate_array(type, count, tag) (type *)malloc((count) * sizeof(type))
#define m_free(address, tag) free((address))

#endif

#define m_safe_free(pointer) if((pointer)) { m_free((pointer)); (pointer) = 0; }

#define word_size (sizeof(void*))
#define align(n) ((n + word_size - 1) & ~(word_size - 1))

internal Arena allocator_init(u64 size)
{
    Arena allocator = {
        .base = (char*)m_allocate(size, "plugin : allocator"),
        .current = allocator.base,
        .capacity = size,
    };
    return allocator;
}


internal void *arena_allocate(Arena *allocator, u64 size)
{
    char *begin = allocator->current;
    u64 aligned = align(size);
    
    if(begin + aligned <= allocator->base + allocator->capacity)
    {
        allocator->current += aligned;
        
        return begin;
    }
    else{
        printf("%f\n", float(allocator->current - allocator->base) / float(allocator->capacity));
        ensure(false);
        return nullptr;
    } 
}

#endif //MEMORY_H
