#include "main.h"
#include "elements/elements.h"
#include "world/world.h"

export void paint(i64 chunkX, i64 chunkY, i16 cellX, i16 cellY, u8 brushSize, ElementType type) {
    ASSERT(brushSize < CHUNKSIZE, "Brush size larger than chunk!");

    Chunk *centralChunk = getChunk(chunkX, chunkY);
    if(!centralChunk) {
        if(type == TYPE_EMPTY) return;
        centralChunk = allocChunk(chunkX, chunkY, 1, PRIORITY_VERY_HIGH);
    } else if(!centralChunk->active) promoteChunk(centralChunk);
    CellView target;

    for(i16 y = -brushSize; y <= brushSize; ++y) {
        for(i16 x = -brushSize; x <= brushSize; ++x) {
            if(y * y + x * x >= brushSize * brushSize) continue;

            getCellView(&target, centralChunk, cellX + x, cellY + y);
            ASSERT(!target.invalid, "Got invalid cell view while painting!");
            if(type == TYPE_EMPTY) {
                if(target.cell->el.type != TYPE_EMPTY) freeElement(&target);
            } else if(target.cell->el.type == TYPE_EMPTY) {
                spawnElementOnCellView(&target, type);
            }
        }
    }
}
