#include "../main.h"
#include "world.h"

void promoteChunk(Chunk *chunk) {
    chunk->active = 1;
    for(i8 y = -1; y <= 1; ++y) {
        for(i8 x = -1; x <= 1; ++x) {
            if(x == 0 && y == 0) continue;
            Chunk *neighbor = chunk->neighbors[y + 1][x + 1];
            if(neighbor) neighbor->activeNeighbors += 1;
            else neighbor = allocChunk(chunk->x + x, chunk->y + y, 0, PRIORITY_LOW);

            chunk->neighbors[y + 1][x + 1] = neighbor;
        }
    }
}

void demoteChunk(Chunk *chunk) {
    chunk->active = 0;
    for(i8 y = -1; y <= 1; ++y) {
        for(i8 x = -1; x <= 1; ++x) {
            if(x == 0 && y == 0) continue;
            Chunk *neighbor = chunk->neighbors[y + 1][x + 1];
            if(neighbor) neighbor->activeNeighbors -= 1;
        }
    }
}

void updateChunkPriority(Chunk *chunk) {
    
}

void postUpdateChunkCleanup(void) {
    for(u8 i = 0; i < 4; ++i) {
        Chunk *chunk = world.chunkThreadingList[i];
        while(chunk != NULL) {
            Chunk *next = chunk->next;

            if(chunk->awesomePuwaDebugReturns == 0) chunk->keepAliveTimer = 1;
            chunk->awesomePuwaDebugReturns = 0;

            if(chunk->active) {
                if(chunk->updateEventsInLastTick == 0) {
                    demoteChunk(chunk);
                }
            } else {
                if(chunk->updateEventsInLastTick > 0) {
                    promoteChunk(chunk);
                } else if(chunk->activeNeighbors == 0 && !chunk->temperatureActive && !chunk->visible) {
                    deallocChunk(chunk);
                }
            }

            chunk->updateEventsInLastTick = 0;

            chunk = next;
        }
    }
}
