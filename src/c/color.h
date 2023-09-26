#ifndef COLOR_H
#define COLOR_H

#include "./main.h"

#define COLOR(n) ((RGBA8888)(u32)((0xff << 24) | ((n & 0xff) << 16) | (n & 0xff00) | ((n & 0xff0000) >> 16)))
typedef union RGBA8888 {
    struct {
        u8 r;
        u8 g;
        u8 b;
        u8 a;
    };
    u32 value;
} RGBA8888;

typedef union RGB332 {
    struct {
        u8 b: 2;
        u8 g: 3;
        u8 r: 3;
    };
    u8 value;
} RGB332;

#endif
