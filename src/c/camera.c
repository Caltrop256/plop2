#include "main.h"
#include "./elements/elements.h"
#include "camera.h"
#include "walloc.h"
#include "./world/world.h"

export Camera camera = {
    .chunkX = 0, .chunkY = 0,
    .cellX = 0.0f, .cellY = 0.0f,
    .scale = 1,

    .chunksVisibleX = 0,
    .chunksVisibleY = 0,

    .screenWidth = 0,
    .screenHeight = 0,

    .tileWidth = 0,
    .tileHeight = 0
};

export void moveCamera(f64 dx, f64 dy) {
    f64 subX = camera.cellX + dx;
    f64 subY = camera.cellY + dy;

    f64 floorSubX = FLOOR(subX);
    f64 floorSubY = FLOOR(subY);

    camera.cellX = subX - floorSubX;
    camera.cellY = subY - floorSubY;

    i64 oChunkX = camera.chunkX;
    i64 oChunkY = camera.chunkY;

    camera.chunkX += floorSubX;
    camera.chunkY += floorSubY;

    if(floorSubX || floorSubY) {
        // for(i64 chunkOffsetY = 0; chunkOffsetY <= camera.chunksVisibleY; ++chunkOffsetY) {
        //     for(i64 chunkOffsetX = 0; chunkOffsetX <= camera.chunksVisibleX; ++chunkOffsetX) {
        //         Chunk *chunk = getChunk(oChunkX + chunkOffsetX, oChunkY + chunkOffsetX);
        //         if(chunk) chunk->visible = 0;
        //     }
        // }

        // for(i64 chunkOffsetY = 0; chunkOffsetY <= camera.chunksVisibleY; ++chunkOffsetY) {
        //     for(i64 chunkOffsetX = 0; chunkOffsetX <= camera.chunksVisibleX; ++chunkOffsetX) {
        //         Chunk *chunk = getChunk(camera.chunkX + chunkOffsetX, camera.chunkY + chunkOffsetY);
        //         if(!chunk) allocChunk(camera.chunkX + chunkOffsetX, camera.chunkY + chunkOffsetY, 0, PRIORITY_HIGH);
        //         chunk->visible = 1;
        //     }
        // }
    }
}

#if USE_GPU
    export GPUElement *screen = NULL;
#else
    export RGBA8888 *screen = NULL;
#endif

export void setScreenSize(u32 width, u32 height, u32 scale) {
    if(screen != NULL) free(screen);
    screen = malloc(sizeof(RGBA8888) * width * height);
    ASSERT(screen != NULL, "Failed to allocate screen!");

    camera.screenWidth = width;
    camera.screenHeight = height;
    camera.scale = scale;
    camera.tileWidth = camera.screenWidth / scale;
    camera.tileHeight = camera.screenHeight / scale;

    camera.chunksVisibleX = CEIL((f32)camera.screenWidth / (CHUNKSIZE * camera.scale));
    camera.chunksVisibleY = CEIL((f32)camera.screenHeight / (CHUNKSIZE * camera.scale));
}

#if USE_GPU

void draw(void) {
    u32 i = camera.screenWidth * camera.screenHeight;
    while(i --> 0) screen[i] = (GPUElement){0};

    i32 camOffsetX = -camera.cellX * CHUNKSIZE * camera.scale;
    i32 camOffsetY = -camera.cellY * CHUNKSIZE * camera.scale;

    for(i64 chunkOffsetY = 0; chunkOffsetY <= camera.chunksVisibleY; ++chunkOffsetY) {
        i32 chunkY = chunkOffsetY * CHUNKSIZE * camera.scale + camOffsetY;

        for(i64 chunkOffsetX = 0; chunkOffsetX <= camera.chunksVisibleX; ++chunkOffsetX) {
            i32 chunkX = chunkOffsetX * CHUNKSIZE * camera.scale + camOffsetX;

            Chunk *chunk = getChunk(camera.chunkX + chunkOffsetX, camera.chunkY + chunkOffsetY);
            if(chunk == NULL) continue;

            ASSERT(chunk->awesomePuwaDebugReturns, "whoops!");

            for(u32 cellY = 0; cellY < CHUNKSIZE; ++cellY) {
                i32 py = chunkY + cellY * camera.scale;
                if(py < 0) continue;
                else if((u32)py >= camera.screenHeight) break;

                for(u32 cellX = 0; cellX < CHUNKSIZE; ++cellX) {
                    i32 px = chunkX + cellX * camera.scale;
                    if(px < 0) continue;
                    else if((u32)px >= camera.screenWidth) break;

                    Element *el = &chunk->cells[cellY][cellX].el;
                    f32 temperature = chunk->cells[cellY][cellX].temperature[world.tick];
                    screen[py * camera.screenWidth + px] = (GPUElement){
                        .type = el->type,
                        .textureX = el->textureX,
                        .textureY = el->textureY,
                        .color = CLAMP(temperature - AMBIENT_TEMPERATURE, 0, 255)
                    };
                }
            }
        }
    }
}

#else

void line(i32 x0, i32 y0, i32 x1, i32 y1, RGBA8888 clr) {
    int dx = ABS(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -ABS(y1 - y0), sy = y0 < y1 ? 1 : -1; 
    int err = dx + dy, e2;
    
    for(;;) {
        if(y0 < 0 || y0 >= camera.screenHeight || x0 < 0 || x0 >= camera.screenWidth) break;
        screen[y0 * camera.screenWidth + x0] = clr;
        if(x0 == x1 && y0 == y1) break;
        e2 = 2 * err;
        if(e2 >= dy) { err += dy; x0 += sx; }
        if(e2 <= dx) { err += dx; y0 += sy; }
    }
}

void draw(void) {
    u32 i = camera.screenWidth * camera.screenHeight;
    while(i --> 0) *(u32 *)(&screen[i]) = 0; 

    i32 camOffsetX = -camera.cellX * CHUNKSIZE * camera.scale;
    i32 camOffsetY = -camera.cellY * CHUNKSIZE * camera.scale;

    for(i64 chunkOffsetY = 0; chunkOffsetY <= camera.chunksVisibleY; ++chunkOffsetY) {
        i32 chunkY = chunkOffsetY * CHUNKSIZE * camera.scale + camOffsetY;

        for(i64 chunkOffsetX = 0; chunkOffsetX <= camera.chunksVisibleX; ++chunkOffsetX) {
            i32 chunkX = chunkOffsetX * CHUNKSIZE * camera.scale + camOffsetX;

            Chunk *chunk = getChunk(camera.chunkX + chunkOffsetX, camera.chunkY + chunkOffsetY);
            if(chunk == NULL) continue;

            ASSERT(chunk->awesomePuwaDebugReturns, "whoops!");

            for(u32 cellY = 0; cellY < CHUNKSIZE; ++cellY) {
                i32 py = chunkY + cellY * camera.scale;
                if(py < 0) continue;
                else if(py >= camera.screenHeight) break;

                for(u32 cellX = 0; cellX < CHUNKSIZE; ++cellX) {
                    i32 px = chunkX + cellX * camera.scale;
                    if(px < 0) continue;
                    else if(px >= camera.screenWidth) break;

                    if(cellX % 4 == 0 && cellY % 4 == 0) {
                        CellView cv = { .chunk = chunk, .x = cellX, .y = cellY };
                        u32 ind = getFluidInfo(&cv);
                        if(ind != -1) {
                            f32 vx = world.fluid.vx[ind] * camera.scale;
                            f32 vy = world.fluid.vy[ind] * camera.scale;

                            line(px, py, px + vx, py + vy, (RGBA8888){{128, 128, 128, 255}});
                        }
                    }

                    if(chunk->cells[cellY][cellX].el.type != TYPE_EMPTY) {
                        ElementType type = chunk->cells[cellY][cellX].el.type;
                        f32 temperature = chunk->cells[cellY][cellX].temperature[world.tick];
                        RGBA8888 clr = elementLookup[type].color;
                        clr.r = CLAMP(clr.r + temperature - AMBIENT_TEMPERATURE, 0, 255);

                        for(u32 cellPixelY = 0; cellPixelY < camera.scale; ++cellPixelY) {
                            i32 spy = py + cellPixelY;
                            if(spy < 0) continue;
                            else if(spy >= camera.screenHeight) break;
                            for(u32 cellPixelX = 0; cellPixelX < camera.scale; ++cellPixelX) {
                                i32 spx = px + cellPixelX;
                                if(spx < 0) continue;
                                else if(spx >= camera.screenWidth) break;

                                screen[spy * camera.screenWidth + spx] = clr;
                            }
                        }
                    }
                }
            }


            #if DEBUG
                for(i32 i = 0, size = CHUNKSIZE * camera.scale; i < size; ++i) {
                    RGBA8888 colors[4] = {
                        {.b = 0xff, .a = 0xff}, {.g = 0xff, .a = 0xff},
                        {.r = 0xff, .b = 0xff, .a = 0xff}, {.g = 0xff, .b = 0xff, .a = 0xff}
                    };
                    RGBA8888 clr  = colors[GETCHUNKLINKEDLISTIND(chunk)];

                    i32 x0 = chunkX + i;
                    i32 y0 = chunkY + i;
                    if(INRANGE(0, camera.screenWidth, x0) && INRANGE(0, camera.screenHeight, chunkY)) screen[chunkY * camera.screenWidth + x0] = clr;
                    if(INRANGE(0, camera.screenWidth, x0) && INRANGE(0, camera.screenHeight, chunkY + size)) screen[(chunkY + size) * camera.screenWidth + x0] = clr;
                    if(INRANGE(0, camera.screenHeight, y0) && INRANGE(0, camera.screenWidth, chunkX)) screen[y0 * camera.screenWidth + chunkX] = clr;
                    if(INRANGE(0, camera.screenHeight, y0) && INRANGE(0, camera.screenWidth, chunkX + size)) screen[y0 * camera.screenWidth + chunkX + size] = clr;
                }

                for(i32 y = chunk->dirtyRect.minY * camera.scale; y < (chunk->dirtyRect.maxY + 1) * camera.scale; ++y) {
                    RGBA8888 red = (RGBA8888){.r = 0xff, .a = 0xff};
                    i32 x0 = chunkX + chunk->dirtyRect.minX * camera.scale;
                    i32 x1 = chunkX + (chunk->dirtyRect.maxX + 1) * camera.scale;
                    i32 y0 = chunkY + y;
                    if(INRANGE(0, camera.screenWidth, x0) && INRANGE(0, camera.screenHeight, y0)) screen[y0 * camera.screenWidth + x0] = red;
                    if(INRANGE(0, camera.screenWidth, x1) && INRANGE(0, camera.screenHeight, y0)) screen[y0 * camera.screenWidth + x1] = red;
                }

                for(i32 x = chunk->dirtyRect.minX * camera.scale; x < (chunk->dirtyRect.maxX + 1) * camera.scale; ++x) {
                    RGBA8888 red = (RGBA8888){.r = 0xff, .a = 0xff};
                    i32 x0 = chunkX + x;
                    i32 y0 = chunkY + chunk->dirtyRect.minY * camera.scale;
                    i32 y1 = chunkY + (chunk->dirtyRect.maxY + 1) * camera.scale;
                    if(INRANGE(0, camera.screenWidth, x0) && INRANGE(0, camera.screenHeight, y0)) screen[y0 * camera.screenWidth + x0] = red;
                    if(INRANGE(0, camera.screenWidth, x0) && INRANGE(0, camera.screenHeight, y1)) screen[y1 * camera.screenWidth + x0] = red;
                }
            #endif
        }
    }
}

#endif

