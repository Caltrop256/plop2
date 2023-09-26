#include "../main.h"
#include "world.h"
#include "../walloc.h"

export World world = {
    .chunkThreadingList = {NULL, NULL, NULL, NULL},
    .map = NULL,
    .fluid = {
        .dt = 0.0005f,
        .diff = 0.1f,
        .visc = 0.001f
    }
};

struct XYPair {
    i64 x; i64 y;
};

export Chunk *getChunk(i64 x, i64 y) {
    uintptr_t res = 0;
    struct XYPair pair = {x, y};
    hashmap_get(world.map, &pair, sizeof(struct XYPair), &res);
    return (Chunk *)res;
}

u8 magic(u32 n) {
    static const u32 decode[32] = { 0, 1, 2, 26, 23, 3, 15, 27, 24, 21, 19, 4, 12, 16, 28, 6, 31, 25, 22, 14, 20, 18, 11, 5, 30, 13, 17, 10, 29, 9, 8, 7 };
    n = 0x05f66a47 * (n & (-n));
    return decode[n >> 27];
}

export Chunk *allocChunk(i64 x, i64 y, _Bool isActive, AllocationPriority allocationPriority) {
    Chunk *chunk = NULL;
    for(u8 targetedPriority = PRIORITY_DEALLOCATION_RECOMMENDED; targetedPriority < allocationPriority; ++targetedPriority) {
        for(u32 i = 0; i < 32; ++i) {
            if(world.freeChunkList[targetedPriority][i]) {
                u32 blockInd = magic(world.freeChunkList[targetedPriority][i]);
                if(targetedPriority > PRIORITY_DEALLOCATION_RECOMMENDED) {
                    deallocChunk(&world.chunks[(i << 5) + blockInd]);
                }
                world.freeChunkList[targetedPriority][i] &= ~(1 << blockInd);
                world.freeChunkList[allocationPriority][i] |= (1 << blockInd);
                chunk = &world.chunks[(i << 5) + blockInd];
                break;
            }
        }
    }
    ASSERT(chunk != NULL, "chunk allocator failed!");

    chunk->x = x;
    chunk->y = y;

    chunk->elementsInChunk = 0;
    chunk->active = isActive;
    chunk->activeNeighbors = 0;

    chunk->dirtyRect.maxY = 0;
    chunk->dirtyRect.minY = CHUNKSIZE - 1;
    chunk->dirtyRect.maxX = 0;
    chunk->dirtyRect.minX = CHUNKSIZE - 1;

    chunk->keepAliveTimer = 1;

    chunk->temperatureActive = 0;

    for(u32 y = 0; y < CHUNKSIZE; ++y) {
        for(u32 x = 0; x < CHUNKSIZE; ++x) {
            chunk->cells[y][x] = (Cell){
                .el = {.type = 0},
                .temperature = {AMBIENT_TEMPERATURE, AMBIENT_TEMPERATURE}
            };
        }
    }

    hashmap_set(world.map, &chunk->x, sizeof(struct XYPair), (uintptr_t)chunk);

    u8 chunkllInd = GETCHUNKLINKEDLISTIND(chunk);
    chunk->prev = NULL;
    chunk->next = world.chunkThreadingList[chunkllInd];
    if(chunk->next) chunk->next->prev = chunk;
    world.chunkThreadingList[chunkllInd] = chunk;
    world.chunksInList[chunkllInd] += 1;
    
    for(i8 y = -1; y <= 1; ++y) {
        for(i8 x = -1; x <= 1; ++x) {
            if(x == 0 && y == 0) {
                chunk->neighbors[y + 1][x + 1] = chunk;
            } else {
                Chunk *neighbor = getChunk(chunk->x + x, chunk->y + y);

                if(neighbor && neighbor->active) chunk->activeNeighbors += 1;

                if(isActive) {
                    if(neighbor) neighbor->activeNeighbors += 1;
                    else neighbor = allocChunk(chunk->x + x, chunk->y + y, 0, PRIORITY_LOW);
                }

                chunk->neighbors[y + 1][x + 1] = neighbor;

                if(neighbor != NULL) {
                    neighbor->neighbors[-y + 1][-x + 1] = chunk;
                }
            }
        }
    }

    return chunk;
}

void deallocChunk(Chunk *chunk) {
    for(i8 y = -1; y <= 1; ++y) {
        for(i8 x = -1; x <= 1; ++x) {
            Chunk *neighbor = chunk->neighbors[y + 1][x + 1];
            if(neighbor != NULL) neighbor->neighbors[-y + 1][-x + 1] = NULL;
        }
    }

    hashmap_remove(world.map, &chunk->x, sizeof(struct XYPair));

    u8 chunkllInd = GETCHUNKLINKEDLISTIND(chunk);
    if(chunk->prev == NULL) world.chunkThreadingList[chunkllInd] = chunk->next;
    else chunk->prev->next = chunk->next;
    if(chunk->next != NULL) chunk->next->prev = chunk->prev;
    world.chunksInList[chunkllInd] -= 1;


    u32 arrayInd = (u32)((u8 *)chunk - (u8 *)&world.chunks[0]) / sizeof(Chunk);
    u32 listInd = arrayInd >> 5;
    u32 blockInd = arrayInd & 31;

    world.freeChunkList[PRIORITY_DEALLOCATION_RECOMMENDED][listInd] |= (1 << blockInd);
    world.freeChunkList[chunk->priority][listInd] &= ~(1 << blockInd);
} 
