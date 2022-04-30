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

#define M_PI (3.1415926535897932384626433)
#define M_2PI (3.1415926535897932384626433 * 2.0)

#define internal static
#define function static

#define octave_max(x, y) (((x) > (y)) ? (x) : (y))
#define octave_min(x, y) (((x) < (y)) ? (x) : (y))
#define octave_clamp(x, low, high) (((x) > (low)) ? (((x) < (high)) ? (x) : (high)) : (low))
#define octave_abs(x) ((x > 0) ? (x) : -(x))
#define octave_lerp(val, in_min, in_max, out_min, out_max) (out_min) + ((out_max) - (out_min)) * ((val) - (in_min)) / ((in_max)- (in_min))

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define AT __FILE__ ":" TOSTRING(__LINE__)


#ifdef DEBUG
#define ensure(expr) assert(expr)
#endif
#ifdef RELEASE
#define ensure(expr) if(!(expr)) { printf(AT " : " #expr); exit(-1); } 
#endif

#ifdef ATOMIC_FORCE_SLEEP
#define ATOMIC_HARNESS() { Sleep(rand() * 60 / (RAND_MAX + 1)); }
#else
#define ATOMIC_HARNESS()
#endif

typedef struct{
    union { 
        char* str;
        char* data;
    };
    union {
        u64 size;
        u64 length;
        u64 len;
    };
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
    return (real32)sqrt((a.x - b.x)*(a.x - b.x) +  (a.y - b.y) * (a.y - b.y));
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

internal inline Vec2 vec2_lerp(Vec2 a, Vec2 b, real32 x)
{
    return {
        octave_lerp(x, 0.0f, 1.0f, a.x, b.x),
        octave_lerp(x, 0.0f, 1.0f, a.y, b.y)
    };
}

typedef struct {
    union{
        struct {
            Vec2 origin;
            Vec2 dim;
        };
        struct {
            real32 x;
            real32 y;
            real32 w;
            real32 h;
        };
    };
} Rect;

internal bool rect_equal(Rect a, Rect b)
{
    return 
        a.x == b.x &&
        a.y == b.y &&
        a.w == b.w &&
        a.h == b.h;
}

internal bool rect_contains(Rect bounds, Vec2 position)
{
    return position.x > bounds.x &&
        position.x < (bounds.x + bounds.w) &&
        position.y > bounds.y &&
    (position.y < bounds.y + bounds.h);
}

internal Rect rect_move_by(Rect bounds, Vec2 delta)
{
    bounds.x += delta.x;
    bounds.y += delta.y;
    return bounds;
}

internal Rect rect_drop_bottom(Rect rect, real32 margin_bottom)
{
    rect.h -= margin_bottom;
    return rect;
}

internal Rect rect_take_bottom(Rect rect, real32 margin_bottom)
{
    rect.y += rect.h - margin_bottom;
    rect.h = margin_bottom;
    return rect;
}

internal Rect rect_drop_top(Rect rect, real32 margin_top)
{
    rect.h -= margin_top;
    rect.y += margin_top;
    return rect;
}

internal Rect rect_take_top(Rect rect, real32 margin_top)
{
    rect.h = margin_top;
    return rect;
}


internal Rect rect_drop_left(Rect rect, real32 margin_left)
{
    rect.w -= margin_left;
    rect.x += margin_left;
    return rect;
}

internal Rect rect_take_left(Rect rect, real32 margin_left)
{
    rect.w = margin_left;
    return rect;
}

internal Rect rect_take_right(Rect rect, real32 margin_right)
{
    rect.x += rect.w - margin_right;
    rect.w = margin_right;
    return rect;
}

internal Rect rect_drop_right(Rect rect, real32 margin_right)
{
    rect.w -= margin_right;
    return rect;
}

internal void rect_split_from_left(Rect in, real32 margin_left, Rect *out_left, Rect *out_right)
{
    *out_left = rect_take_left(in, margin_left);
    *out_right = rect_drop_left(in, margin_left);
}

internal void rect_split_vert_middle(Rect in, Rect *out_top, Rect *out_bottom)
{
    in.h /= 2.0f;
    *out_top = in;
    in.y += in.h;
    *out_bottom = in;
}

internal Rect rect_shrinked(Rect rect, real32 padding_x, real32 padding_y)
{
    
    return {
        .origin = {
            rect.x + padding_x,
            rect.y + padding_y
        },
        .dim = {
            octave_max(rect.w - padding_x * 2, 0.0f),
            octave_max(rect.h - padding_y * 2, 0.0f)
        }
    };
}

internal void rect_shrink(Rect *rect, real32 padding_x, real32 padding_y)
{
    *rect = rect_shrinked(*rect, padding_x, padding_y);
}

internal bool float_cmp(real32 a, real32 b, real32 epsilon)
{
    return octave_abs(a - b) < epsilon;
}


typedef union Vec4u8 Color;

union Vec4u8 {
    struct {
        u8 a;
        u8 r;
        u8 g;
        u8 b;
    };
    u32 argb;
};

internal Color color_lighten_by(Color col, u8 val)
{
    
    col.r = (u8) octave_min((i32) col.r + (i32) val, 255);
    col.g = (u8) octave_min((i32) col.g + (i32) val, 255);
    col.b = (u8) octave_min((i32) col.b + (i32) val, 255);
    return col;
}


internal Color color_darken_by(Color col, u8 val)
{
    col.r = (u8) octave_max((i32) col.r - (i32) val, 0);
    col.g = (u8) octave_max((i32) col.g - (i32) val, 0);
    col.b = (u8) octave_max((i32) col.b - (i32) val, 0);
    return col;
}


#endif //BASE_H
