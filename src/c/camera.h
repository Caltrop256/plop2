#ifndef CAMERA_H
#define CAMERA_H

#include "main.h"

typedef struct Camera {
    i64 chunkX;
    i64 chunkY;

    f64 cellX;
    f64 cellY;

    u32 scale;
    u32 tileWidth;
    u32 tileHeight;
    u32 screenWidth;
    u32 screenHeight;

    Chunk **visibleChunks;
    u32 chunksVisibleX;
    u32 chunksVisibleY;
} Camera;

extern Camera camera;

void draw(void);

#endif
