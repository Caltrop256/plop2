#ifndef WORLDGEN_H
#define WORLDGEN_H

#include "elements/elements.h"

u32 generateChunk(Cell cells[CHUNKSIZE][CHUNKSIZE], i64 cx, i64 cy);

#endif
