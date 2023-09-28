#include "../main.h"
#include "../walloc.h"
#include "elements.h"
#include "../threads/threads.h"
#include "../camera.h"

export ElementInfo elementLookup[type_length];

inline u8 clamp_0_CHUNKSIZE(i16 n) {
	n &= -(n >= 0);
	return n < (CHUNKSIZE) ? n : CHUNKSIZE - 1;
}

#define PADDING 3

static void updateDirtyRectAtomic(DirtyRect *addr, DirtyRect value) {
    i32 original;
    i32 real = atomic_load_i32(addr);

    do {
        original = real;
        DirtyRect rect = (DirtyRect){.value = original};

        if(value.minX < rect.minX) rect.minX = value.minX;
        if(value.minY < rect.minY) rect.minY = value.minY;
        if(value.maxX > rect.maxX) rect.maxX = value.maxX;
        if(value.maxY > rect.maxY) rect.maxY = value.maxY;

        real = atomic_rmw_cmpxchg_i32(addr, original, rect.value);
    } while(original != real);
}

void updateChunkDirtyRect(CellView *cv) {
    Chunk *chunk = cv->chunk;
    atomic_rmw_add_i32(&chunk->updateEventsInLastTick, 1);

    i16 centralMinX = (i16)cv->x - PADDING;
    i16 centralMaxX = (i16)cv->x + PADDING;
    i16 centralMinY = (i16)cv->y - PADDING;
    i16 centralMaxY = (i16)cv->y + PADDING;

    DirtyRect clampedRect = {
        .minX = clamp_0_CHUNKSIZE(centralMinX),
        .maxX = clamp_0_CHUNKSIZE(centralMaxX),
        .minY = clamp_0_CHUNKSIZE(centralMinY),
        .maxY = clamp_0_CHUNKSIZE(centralMaxY)
    };

    updateDirtyRectAtomic(&chunk->dirtyRect, clampedRect);

    i16 diffMinX = centralMinX - clampedRect.minX;
    i16 diffMaxX = centralMaxX - clampedRect.maxX;
    i16 diffMinY = centralMinY - clampedRect.minY;
    i16 diffMaxY = centralMaxY - clampedRect.maxY;

    i8 dirX = 0;
    i8 dirY = 0;
    u8 cxMin;
    u8 cxMax;
    u8 cyMin;
    u8 cyMax;

    Chunk *targetChunk;

    if(diffMinX != 0) {
        dirX = -1;
        cxMax = CHUNKSIZE - 1;
        cxMin = (CHUNKSIZE - 1) + diffMinX;
    } else if(diffMaxX != 0) {
        dirX = 1;
        cxMin = 0;
        cxMax = diffMaxX;
    }

    if(diffMinY != 0) {
        dirY = -1;
        cyMax = CHUNKSIZE - 1;
        cyMin = (CHUNKSIZE - 1) + diffMinY;
    } else if(diffMaxY != 0) {
        dirY = 1;
        cyMin = 0;
        cyMax = diffMaxY;
    }

    if(dirX) {
        targetChunk = chunk->neighbors[1][dirX + 1];
        if(targetChunk) {
            DirtyRect rect = {
                .minX = cxMin,
                .maxX = cxMax,
                .minY = clampedRect.minY,
                .maxY = clampedRect.maxY
            };
            updateDirtyRectAtomic(&targetChunk->dirtyRect, rect);
        }
    }
    if(dirY) {
        targetChunk = chunk->neighbors[dirY + 1][1];
        if(targetChunk) {
            DirtyRect rect = {
                .minX = clampedRect.minX,
                .maxX = clampedRect.maxX,
                .minY = cyMin,
                .maxY = cyMax
            };
            updateDirtyRectAtomic(&targetChunk->dirtyRect, rect);
        }
    }
    if(dirY && dirX) {
        targetChunk = chunk->neighbors[dirY + 1][dirX + 1];
        if(targetChunk) {
            DirtyRect rect = {
                .minX = cxMin,
                .maxX = cxMax,
                .minY = cyMin,
                .maxY = cyMax
            };
            updateDirtyRectAtomic(&targetChunk->dirtyRect, rect);
        }
    }
}

export CellView *getCellView(CellView *cv, Chunk *chunk, i16 x, i16 y) {
    i16 dx = x >> CHUNKSHIFT;
    i16 dy = y >> CHUNKSHIFT;

    Chunk *targetChunk = chunk->neighbors[dy + 1][dx + 1];
    if(targetChunk == NULL) {
        cv->invalid = 1;
        return cv;
    } 

    x = x - (1 << CHUNKSHIFT) * dx;
    y = y - (1 << CHUNKSHIFT) * dy;

    cv->cell = &targetChunk->cells[y][x];
    cv->chunk = targetChunk;
    cv->x = x;
    cv->y = y;

    cv->keepAlive = 0;
    cv->invalid = 0;

    return cv;
}

#define DECLARE_INIT_FUNCTION(UPPER, LOWER) void init_ ## LOWER(Element *);
FOREACH_ELEMENTTYPE(DECLARE_INIT_FUNCTION)
#undef DECLARE_INIT_FUNCTION

export void spawnElementOnCellView(CellView *cv, ElementType type) {
    ASSERT(cv->cell->el.type == TYPE_EMPTY, "Tried spawning element on occupied cell!");
    if(cv->invalid) return;
    //ASSERT(!cv->invalid, "Tried spawning element in the void!");

    cv->cell->el = (Element){
        .tick = !world.tick,
        .type = type,
        .textureX = cv->x & 7,
        .textureY = cv->y & 7,
        .subPixelX = 4,
        .subPixelY = 4
    };

    switch(type) {
        default : break;
        #define DEFINE_INIT_CASE(UPPER, LOWER) case TYPE_ ## UPPER : init_ ## LOWER(&cv->cell->el); break;
        FOREACH_ELEMENTTYPE(DEFINE_INIT_CASE)
        #undef DEFINE_INIT_CASE
    }

    cv->chunk->elementsInChunk += 1;

    updateChunkDirtyRect(cv);
}

InteractionResult freeElement(CellView *cv) {
    ASSERT(cv->cell->el.type != TYPE_EMPTY, "Tried freeing empty cell!");
    ASSERT(!cv->invalid, "Tried to free void element!");

    cv->cell->el = (Element){0};
    cv->chunk->elementsInChunk -= 1;
    updateChunkDirtyRect(cv);
    return RESULT_DELETED;
}

InteractionResult replaceElement(CellView *cv, ElementType type) {
    ASSERT(cv->cell->el.type != TYPE_EMPTY, "Tried replacing empty cell!");
    ASSERT(!cv->invalid, "Tried to replace void element!");

    cv->cell->el = (Element){
        .tick = !world.tick,
        .type = type,
        .textureX = cv->x & 7,
        .textureY = cv->y & 7,
        .subPixelX = 4,
        .subPixelY = 4
    };

    switch(type) {
        default : break;
        #define DEFINE_INIT_CASE(UPPER, LOWER) case TYPE_ ## UPPER : init_ ## LOWER(&cv->cell->el); break;
        FOREACH_ELEMENTTYPE(DEFINE_INIT_CASE)
        #undef DEFINE_INIT_CASE
    }
    updateChunkDirtyRect(cv);
    return RESULT_REPLACED;
}

InteractionResult moveElement(CellView *source, CellView *target) {
    ASSERT(target->cell->el.type == TYPE_EMPTY, "Tried to move element onto occupied cell!");
    ASSERT(!source->invalid, "Tried to move void element!");

    if(target->invalid) {
        return freeElement(source);
    }

    target->cell->el = source->cell->el;
    source->cell->el.type = 0;

    f32 targetTemp = TEMPERATURE(target);
    TEMPERATURE(target) = TEMPERATURE(source);
    TEMPERATURE(source) = targetTemp;

    if(source->chunk != target->chunk) {
        atomic_rmw_add_i32(&source->chunk->elementsInChunk, -1);
        atomic_rmw_add_i32(&target->chunk->elementsInChunk, 1);

        target->chunk->keepAliveTimer = 1;
    }

    updateChunkDirtyRect(source);
    updateChunkDirtyRect(target);
    return RESULT_MOVED;
}

InteractionResult swapElements(CellView *source, CellView *target) {
    ASSERT(target->cell->el.type != TYPE_EMPTY, "Tried to swap empty cell!");
    ASSERT(!source->invalid, "Tried to perform void swap (swaper was void)!");
    ASSERT(!target->invalid, "Tried to perform void swap (swapee was void)!");

    Element targetEl = target->cell->el;
    target->cell->el = source->cell->el;
    source->cell->el = targetEl;

    f32 targetTemp = TEMPERATURE(target);
    TEMPERATURE(target) = TEMPERATURE(source);
    TEMPERATURE(source) = targetTemp;

    updateChunkDirtyRect(source);
    updateChunkDirtyRect(target);
    return RESULT_SWAPPED;
}

void multiStepElement(MultiStepResult *result, CellView *source, i16 x1, i16 y1) {
    ElementInfo *info = &elementLookup[source->cell->el.type];

    i16 x0 = source->x;
    i16 y0 = source->y;

    i16 dx = ABS(x1 - x0), sx = x0 < x1 ? 1 : -1;
    i16 dy = -ABS(y1 - y0), sy = y0 < y1 ? 1 : -1; 
    i16 err = dx + dy, e2;

    ASSERT(ABS(dx) <= SAFE_MOVEMENT_DIST, "multi step dx beyond safe distance!");
    ASSERT(ABS(dy) <= SAFE_MOVEMENT_DIST, "multi step dy beyond safe distance!");

    CellView from = *source;
    CellView to;

    for(;;) {
        e2 = 2 * err;
        if(e2 >= dy) { err += dy; x0 += sx; }
        if(e2 <= dx) { err += dx; y0 += sy; }

        getCellView(&to, source->chunk, x0, y0);
        switch(result->result = info->interact(&from, &to)) {
            case RESULT_MOVED :
            case RESULT_SWAPPED :
                from = to;
                break;
            case RESULT_BLOCKED :
            case RESULT_DELETED :
            case RESULT_REPLACED :
            default :
                goto end;
        }

        if(x0 == x1 && y0 == y1) break;
    }

    end :

    result->destination = from;
}

void setupElementInfo(void) {
    elementLookup[TYPE_EMPTY] = (ElementInfo){
        .name = "Empty",
        .description = "fresh air",

        .color = {{0, 0, 0, 0}},
        .air = {
            .drag = 0.0f
        },
        .temperature = {
            .conductivity = 1.0f
        }
    };

    #define ADDENTRY(UPPER,LOWER) elementLookup[TYPE_##UPPER] = LOWER##_info;
    FOREACH_ELEMENTTYPE(ADDENTRY)
    #undef ADDENTRY
}

i32 getFluidInfo(CellView *cv) {
    u32 x = cv->x + cv->chunk->x * CHUNKSIZE;
    u32 y = cv->y + cv->chunk->y * CHUNKSIZE;

    u32 fx = (u32)((f32)x / camera.tileWidth * N);
    u32 fy = (u32)((f32)y / camera.tileHeight * N);

    if(fx < 0 || fx >= N || fy < 0 || fy >= N) return -1;

    return fy * N + fx;
}

CellViewDeltaDistance getCellViewDeltaXY(CellView *cv0, CellView *cv1) {
    return (CellViewDeltaDistance){
        .x = (cv1->x + (1 << CHUNKSHIFT) * (cv1->chunk->x - cv0->chunk->x)) - cv0->x,
        .y = (cv1->y + (1 << CHUNKSHIFT) * (cv1->chunk->y - cv0->chunk->y)) - cv0->y
    };
}

void applyTemperature(CellView *cv, f32 deltaTemperature) {
    TEMPERATURE(cv) += deltaTemperature;
    cv->chunk->temperatureActive = 1;
}
