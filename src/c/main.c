#include "main.h"
#include "elements/elements.h"
#include "world/world.h"
#include "./threads/threads.h"
#include "fluidsim.h"
#include "camera.h"
#include "memory/memory.h"

export struct GlueInformation glueInfo = {
    .gpuRenderMode = USE_GPU,
    .elementLookupStructSize = sizeof(ElementInfo),
    .numberOfUniqueElementTypes = type_length
};

export CellView glueView;

extern ElementInfo elementLookup[type_length];

#define UPDATE_STEPS 5
export void setup(void) {
    setupElementInfo();
    world.map = hashmap_create();

    for(u8 targetedPriority = PRIORITY_DEALLOCATION_RECOMMENDED; targetedPriority < NUMBER_OF_PRIORITIES; ++targetedPriority) {
        memset(&world.freeChunkList[targetedPriority], targetedPriority > PRIORITY_DEALLOCATION_RECOMMENDED ? 0 : UINT32_MAX, 32 * sizeof(u32));
    }
}

export void schedulerLoop(u32 nThreads) {
    for(;;) {
        atomic_memory_wait_i32(&threadSchedule[0], 0);
        atomic_store_i32(&threadSchedule[0], 0);

        world.tick = !world.tick;

        for(u8 i = 0; i < UPDATE_STEPS; ++i) {
            for(u8 t = 0; t < nThreads; ++t) {
                atomic_store_i32(&threadSchedule[1 + t * UPDATE_STEPS * 2 + i], 1);
                atomic_memory_notify(&threadSchedule[1 + t * UPDATE_STEPS * 2 + i]);
            }

            for(u8 t = 0; t < nThreads; ++t) {
                atomic_memory_wait_i32(&threadSchedule[1 + t * UPDATE_STEPS * 2 + i + UPDATE_STEPS], 0);
                atomic_store_i32(&threadSchedule[1 + t * UPDATE_STEPS * 2 + i + UPDATE_STEPS], 0);
            }
        }

        postUpdateChunkCleanup();

        notify_main_thread();
    }
}

export void slaveLoop(u32 slaveNumber, u32 nThreads) {
    for(;;) {
        for(u8 i = 0; i < UPDATE_STEPS; ++i) {
            atomic_memory_wait_i32(&threadSchedule[1 + slaveNumber * UPDATE_STEPS * 2 + i], 0);
            atomic_store_i32(&threadSchedule[1 + slaveNumber * UPDATE_STEPS * 2 + i], 0);

            switch(i) {
                case 0 :
                case 1 :
                case 2 :
                case 3 :
                    updateChunks(slaveNumber, nThreads, i);
                    break;
                case 4 :
                    if(slaveNumber == 0) stepFluid(&world.fluid);
                    if(nThreads > 1) {
                        if(slaveNumber == 1) draw();
                    } else draw();
                    break;
            }

            atomic_store_i32(&threadSchedule[1 + slaveNumber * UPDATE_STEPS * 2 + i + UPDATE_STEPS], 1);
            atomic_memory_notify(&threadSchedule[1 + slaveNumber * UPDATE_STEPS * 2 + i + UPDATE_STEPS]);
        }
    }
}

