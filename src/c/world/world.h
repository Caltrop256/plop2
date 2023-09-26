#ifndef WORLD_H
#define WORLD_H

#include "../main.h"
#include "../elements/elements.h"

#define GETCHUNKLINKEDLISTIND(CHUNKPTR) (((CHUNKPTR->y % 2 != 0) << 1) + (CHUNKPTR->x % 2 != 0))

Chunk *allocChunk(i64 x, i64 y, _Bool isActive, AllocationPriority allocationPriority);
void deallocChunk(Chunk *chunk);

Chunk *getChunk(i64 x, i64 y);
void updateChunks(u32 threadId, u32 nThreads, u8 i);
void promoteChunk(Chunk *chunk);
void demoteChunk(Chunk *chunk);
void postUpdateChunkCleanup(void);

#define TEMPERATURE_EPSILON (1.0f)

#endif
