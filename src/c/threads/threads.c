#include "../main.h"
#include "../walloc.h"
#include "threads.h"

i32 *threadSchedule;

export u32 allocThreadScheduleArray(u32 n) {
    threadSchedule = calloc(n, sizeof(u32));
    return (u32)threadSchedule;
}

export u32 allocStack(void) {
    void *stackEnd = calloc(1, STACK_SIZE);
    return (u32)stackEnd + STACK_SIZE;
}
