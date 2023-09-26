#ifndef MAIN_H
#define MAIN_H

#define DEBUG 1

#include <stdint.h>
typedef uint8_t     u8;
typedef int8_t      i8;
typedef uint16_t    u16;
typedef int16_t     i16;
typedef uint32_t    u32;
typedef int32_t     i32;
typedef uint64_t    u64;
typedef int64_t     i64;
typedef float       f32;
typedef double      f64;

#define OFFSETOF(TYPE, MEMBER) ((u32)&((TYPE *)0)->MEMBER)

#define export __attribute__((visibility("default")))
#define import(mod, name, signature) __attribute__((import_module(#mod), import_name(#name))) signature

import(js, log, void printf());
import(js, throw, void panicf());
import(js, notify_main_thread, void notify_main_thread(void));

#define MAX(x, y) ((x) >= (y) ? (x) : (y))
#define MIN(x, y) ((x) <= (y) ? (x) : (y))
#define CLAMP(val, min, max) MIN(max, MAX(min, val))
#define SIGN(x) ((x) > 0 ? (1) : (x) < 0 ? (-1) : (0))
#define ABS(x) ((x) < 0 ? (-(x)) : x)
#define FLOOR(x) __builtin_floor(x)
#define CEIL(x) __builtin_ceil(x)

#define APPROACH(source, target, slope) ((source) = ((source) - (target)) * (slope) + (target))
#define APPROACHIFLESS(source, target, slope) if(source < target) ((source) = ((source) - (target)) * (slope) + (target))
#define APPROACHIFMORE(source, target, slope) if(source > target) ((source) = ((source) - (target)) * (slope) + (target))

#define INRANGE(min, max, n) ((min) <= (n) && (n) < (max))

#if DEBUG
#define ASSERT(expr, message) do{ if(!(expr)) panicf(message); } while(0)
#else
#define ASSERT(expr, message)
#endif
#define STATIC_ASSERT(expr, message) _Static_assert(expr, message)

#define USE_GPU 1

#ifndef NULL
#define NULL ((void*)0)
#endif

inline f32 fabs(f32 f) {
    u32 i = ((*(u32*)&f) & 0x7fffffff);
    return (*(f32*)&i);
}

struct GlueInformation {
    u32 gpuRenderMode;
    u32 elementLookupStructSize;
    u32 numberOfUniqueElementTypes;
};

#endif
