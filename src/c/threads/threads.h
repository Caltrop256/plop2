#ifndef THREADS_H
#define THREADS_H

#define STACK_SIZE 65536

i32 atomic_rmw_add_i32(void *addr, i32 value);
i32 atomic_load_i32(void *addr);
void atomic_store_i32(void *addr, i32 value);
i32 atomic_rmw_cmpxchg_i32(void *addr, i32 expected, i32 value);

i32 atomic_memory_notify(void *addr);
enum AtomicWaitResult {
    OK,
    NOT_EQUAL,
    TIMED_OUT
};
enum AtomicWaitResult atomic_memory_wait_i32(void *addr, i32 expected);

extern i32 *threadSchedule;

#endif
