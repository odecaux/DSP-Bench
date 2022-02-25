/* date = November 29th 2021 4:49 pm */

#ifndef BASE_H
#define BASE_H

#include "assert.h"

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

typedef char i8;
typedef short i16;
typedef int i32;
typedef long long i64;

typedef float real32;
typedef double real64;

#define ArrayCount(a) (sizeof(a) / sizeof((a)[0]))
#define OffsetOf(type, member)  ((size_t)&(((type*)0)->member))

#define Kilobytes(count) (u64)(count*(u64)1024)
#define Megabytes(count) (u64)(count*Kilobytes(1024))
#define Gigabytes(count) (u64)(count*Megabytes(1024))

#define COMMIT_BLOCK_SIZE (Kilobytes(4))

#define M_2I (3.1415926535897932384626433 * 2.0)

#define internal static

#define octave_max(x, y) (((x) > (y)) ? (x) : (y))
#define octave_min(x, y) (((x) < (y)) ? (x) : (y))
#define octave_abs(x) ((x > 0) ? (x) : -(x))

typedef struct{
    union { 
        char* str;
        char* data;
    };
    u64 size;
} String;


#define StringLit(s) String((char*)(s), ArrayCount(s) - 1)

typedef struct {
    union{
        struct {
            real32 x;
            real32 y;
        };
        struct {
            real32 a;
            real32 b;
        };
    };
} Vec2;

internal inline real32 vec2_length(Vec2 a, Vec2 b)
{
    return sqrt((a.x - b.x)*(a.x - b.x) +  (a.y - b.y) * (a.y - b.y));
}

internal inline Vec2 vec2_normalize(Vec2 a, Vec2 b)
{
    real32 length = vec2_length(a, b);
    
    if(length == 0.0f) 
        return Vec2 {0.0f, 0.0f};
    else 
        return Vec2 {(a.x - b.x) / length, (a.y - b.y) / length};
}

internal inline Vec2 vec2_mult_scalar(real32 lambda, Vec2 a)
{
    return Vec2{ a.x * lambda, a.y * lambda};
}

internal inline Vec2 vec2_add(Vec2 a, Vec2 b)
{
    return Vec2{a.x + b.x, a.y + b.y};
}


internal inline Vec2 vec2_minus(Vec2 a, Vec2 b)
{
    return Vec2{a.x - b.x, a.y - b.y};
}

typedef struct {
    Vec2 origin;
    Vec2 dim;
} Rect;

internal bool rect_contains(Rect bounds, Vec2 position)
{
    return position.x > bounds.origin.x &&
        position.x < (bounds.origin.x + bounds.dim.x) &&
        position.y > bounds.origin.y &&
    (position.y < bounds.origin.y + bounds.dim.y);
}

internal Rect rect_remove_padding(Rect rect, real32 padding_x, real32 padding_y)
{
    return Rect{
        {
            rect.origin.x + padding_x,
            rect.origin.y + padding_y
        },
        {
            rect.dim.x - padding_x * 2,
            rect.dim.y - padding_y * 2
        }
    };
}

internal bool float_cmp(real32 a, real32 b, real32 epsilon)
{
    return octave_abs(a - b) < epsilon;
}

#endif //BASE_H
