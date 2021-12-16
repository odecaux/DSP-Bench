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

#define Kilobytes(count) (u64)(count*(u64)1024)
#define Megabytes(count) (u64)(count*Kilobytes(1024))
#define Gigabytes(count) (u64)(count*Megabytes(1024))

#define COMMIT_BLOCK_SIZE (Kilobytes(4))

#define internal static

#define octave_max(x, y) (((x) > (y)) ? (x) : (y))
#define octave_min(x, y) (((x) < (y)) ? (x) : (y))

typedef struct{
    char* str;
    u64 size;
} String;

typedef struct {
    real32 x;
    real32 y;
} Vec2;

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

#endif //BASE_H
