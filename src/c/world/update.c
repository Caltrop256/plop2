#include "../main.h"
#include "../random/random.h"
#include "world.h"

void updateChunkTemperature(Chunk *chunk) {
    u32 totalSignificantTransfers = 0;

    for(u32 y = 0; y < CHUNKSIZE; ++y) {
        for(u32 x = 0; x < CHUNKSIZE; ++x) {

            CellView source = CELLVIEW_FROM_CHUNK(chunk, x, y);
            Element *el = &source.cell->el;
            ElementInfo *info = &elementLookup[el->type];

            f32 cumulativeTemperatureChange = 0.0f;
            f32 c0 = info->temperature.conductivity;

            for(i8 dy = -1; dy <= 1; ++dy) {
                for(i8 dx = -1; dx <= 1; ++dx) {
                    if(dx == 0 && dy == 0) continue;

                    CellView temperatureTarget;
                    getCellView(&temperatureTarget, chunk, x + dx, y + dy);
                    if(!temperatureTarget.invalid) {
                        f32 c1 = elementLookup[temperatureTarget.cell->el.type].temperature.conductivity;
                        cumulativeTemperatureChange += ((c0 * c1)) * (temperatureTarget.cell->temperature[!world.tick] - source.cell->temperature[!world.tick]);
                    } else {
                        f32 c1 = elementLookup[TYPE_EMPTY].temperature.conductivity;
                        cumulativeTemperatureChange += ((c0 * c1)) * (AMBIENT_TEMPERATURE - source.cell->temperature[!world.tick]);
                    }
                }
            }
            cumulativeTemperatureChange /= 8;

            TEMPERATURE(&source) = source.cell->temperature[!world.tick] + cumulativeTemperatureChange;

            if(fabs32(TEMPERATURE(&source) - AMBIENT_TEMPERATURE) > TEMPERATURE_EPSILON) totalSignificantTransfers += 1;

            if(el->type != TYPE_EMPTY) {
                if(info->temperature.lowTemperatureTransition.occurs && TEMPERATURE(&source) < info->temperature.lowTemperatureTransition.transitionPoint) {
                    freeElement(&source);
                    if(info->temperature.lowTemperatureTransition.into != TYPE_EMPTY) spawnElementOnCellView(&source, info->temperature.lowTemperatureTransition.into);
                } else if(info->temperature.highTemperatureTransition.occurs && TEMPERATURE(&source) >= info->temperature.highTemperatureTransition.transitionPoint) {
                    freeElement(&source);
                    if(info->temperature.highTemperatureTransition.into != TYPE_EMPTY) spawnElementOnCellView(&source, info->temperature.highTemperatureTransition.into);
                }
            }
        }
    }

    if(totalSignificantTransfers < 25) {
        chunk->temperatureActive = 0;
    }
}

void updateChunkTemperatureBoundary(Chunk *chunk) {
    _Bool top = chunk->neighbors[0][1] != NULL && chunk->neighbors[0][1]->temperatureActive;
    _Bool bottom = chunk->neighbors[2][1] != NULL && chunk->neighbors[2][1]->temperatureActive;
    _Bool left = chunk->neighbors[1][0] != NULL && chunk->neighbors[1][0]->temperatureActive;
    _Bool right = chunk->neighbors[1][2] != NULL && chunk->neighbors[1][2]->temperatureActive;

    for(u32 i = 0; i < CHUNKSIZE; ++i) {
        if(top && fabs32(chunk->neighbors[0][1]->cells[CHUNKSIZE - 1][i].temperature[world.tick] - AMBIENT_TEMPERATURE) > TEMPERATURE_EPSILON) { chunk->temperatureActive = 1; return; };
        if(bottom && fabs32(chunk->neighbors[2][1]->cells[0][i].temperature[world.tick] - AMBIENT_TEMPERATURE) > TEMPERATURE_EPSILON) { chunk->temperatureActive = 1; return; };
        if(left && fabs32(chunk->neighbors[1][0]->cells[i][CHUNKSIZE - 1].temperature[world.tick] - AMBIENT_TEMPERATURE) > TEMPERATURE_EPSILON) { chunk->temperatureActive = 1; return; };
        if(right && fabs32(chunk->neighbors[1][2]->cells[i][0].temperature[world.tick] - AMBIENT_TEMPERATURE) > TEMPERATURE_EPSILON) { chunk->temperatureActive = 1; return; };
    }
}

void updateChunkCells(Chunk *chunk) {
    if(chunk->dirtyRect.maxY == 0 && chunk->dirtyRect.minY == CHUNKSIZE - 1) return;

    DirtyRect oldRect = chunk->dirtyRect;

    if(chunk->keepAliveTimer == 0xff) {
        chunk->dirtyRect.maxY = 0;
        chunk->dirtyRect.minY = CHUNKSIZE - 1;
        chunk->dirtyRect.maxX = 0;
        chunk->dirtyRect.minX = CHUNKSIZE - 1;
    } else chunk->keepAliveTimer -= 1;

    for(u32 y = oldRect.minY; y <= oldRect.maxY; ++y) {
        i32 xStart;
        i32 xEnd;
        i8 xDir;
        if(randomBool()) {
            xStart = oldRect.minX;
            xEnd = oldRect.maxX;
            xDir = 1;
        } else {
            xStart = oldRect.maxX;
            xEnd = oldRect.minX;
            xDir = -1;
        }
        for(i32 x = xStart; x != xEnd + xDir; x += xDir) {
            CellView source = CELLVIEW_FROM_CHUNK(chunk, x, y);

            if(source.cell->el.type != TYPE_EMPTY && world.tick == source.cell->el.tick) {
                Element *el = &source.cell->el;
                el->tick = !el->tick;

                ElementInfo *info = &elementLookup[el->type];

                i16 sx = 0, sy = 0;
                i32 fluidInd = getFluidInfo(&source);
                if(fluidInd != -1) {
                    f32 fvx = world.fluid.vx[fluidInd];
                    f32 fvy = world.fluid.vy[fluidInd];
                    i8 sfvx = SIGN(fvx);
                    i8 sfvy = SIGN(fvy);

                    if(fvx * fvx + fvy * fvy >= info->air.drag * info->air.drag) {
                        sx = SUBPIXEL_FP_TO_I8(el->subPixelX);
                        sy = SUBPIXEL_FP_TO_I8(el->subPixelY);

                        sx += FLOAT_TO_SUBPIXEL_FP(fvx);
                        sy += FLOAT_TO_SUBPIXEL_FP(fvy);

                        el->subPixelX = SUBPIXEL_FP_TO_U4(SUBPIXEL_FP_GET_DECIMAL(sx));
                        el->subPixelY = SUBPIXEL_FP_TO_U4(SUBPIXEL_FP_GET_DECIMAL(sy));

                        sx = SUBPIXEL_FP_FLOOR(sx);
                        sy = SUBPIXEL_FP_FLOOR(sy);
                    }

                    if(info->air.drag > 0.0f) {
                        world.fluid.vx[fluidInd] -= info->air.drag * sfvx * 0.0002;
                        world.fluid.vy[fluidInd] -= info->air.drag * sfvy * 0.0002;
                    }
                }

                UpdateResult result = (UpdateResult){.result = RESULT_NOTARGET};
                elementLookup[el->type].update(&result, &source);
                
                CellView *postUpdateView = NULL;
                switch(result.result) {
                    case RESULT_NOTARGET :
                    case RESULT_BLOCKED :
                    case RESULT_REPLACED :
                        postUpdateView = &source;
                        break;
                    case RESULT_MOVED :
                    case RESULT_SWAPPED :
                        postUpdateView = &result.target;
                        break;
                    case RESULT_DELETED :
                    default :
                        postUpdateView = NULL;
                        break;
                }

                if(postUpdateView != NULL) {
                    el = &postUpdateView->cell->el;
                    info = &elementLookup[el->type];

                    if(sx || sy) {
                        source.keepAlive = 1;
                        MultiStepResult r;
                        multiStepElement(&r, postUpdateView, postUpdateView->x + sx, postUpdateView->y + sy);
                        //postUpdateView = &r.destination;
                    }

                    fluidInd = getFluidInfo(postUpdateView);
                    if(fluidInd != -1 && postUpdateView != &source) {
                        CellViewDeltaDistance d = getCellViewDeltaXY(&source, postUpdateView);

                        if(ABS(world.fluid.vx[fluidInd]) < info->air.drag) world.fluid.vx[fluidInd] += d.x * info->air.drag * 0.1f;
                        if(ABS(world.fluid.vy[fluidInd]) < info->air.drag) world.fluid.vy[fluidInd] += d.y * info->air.drag * 0.1f;
                    }
                }
            }

            if(source.keepAlive) updateChunkDirtyRect(&source);
        }
    }
}

void updateChunks(u32 threadId, u32 nThreads, u8 i) {
    u32 chunksPerThread = world.chunksInList[i] / nThreads;
    u32 chunkStart = threadId * chunksPerThread;
    u32 chunkEnd = chunkStart + chunksPerThread;

    if(threadId == nThreads - 1) chunkEnd = world.chunksInList[i];

    u32 j = 0;
    for(Chunk *chunk = world.chunkThreadingList[i]; j < chunkEnd; chunk = chunk->next, ++j) {
        if(j < chunkStart) continue;

        ASSERT(chunk->awesomePuwaDebugReturns == 0, "whoops 2!");
        chunk->awesomePuwaDebugReturns = 1;

        if(chunk->temperatureActive) updateChunkTemperature(chunk);
        else updateChunkTemperatureBoundary(chunk);
        updateChunkCells(chunk);
    }
}
