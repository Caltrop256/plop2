#include "worldgen.h"
#include "memory/memory.h"


u32 generateChunk(Cell cells[CHUNKSIZE][CHUNKSIZE], i64 cx, i64 cy) {
    u32 nElements = 0;

    for(u32 y = 0; y < CHUNKSIZE; ++y) {
        for(u32 x = 0; x < CHUNKSIZE; ++x) {
            cells[y][x].temperature[0] = AMBIENT_TEMPERATURE;
            cells[y][x].temperature[1] = AMBIENT_TEMPERATURE;

            if(cy > 0) {
                cells[y][x].el = (Element){.type = TYPE_STONE};
                nElements += 1;
            }
        }
    }

    return nElements;
}

