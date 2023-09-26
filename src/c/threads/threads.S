.globaltype __stack_pointer, i32

.global	get_stack_ptr
.type	get_stack_ptr, @function
get_stack_ptr:
    .functype get_stack_ptr() -> (i32)
    global.get __stack_pointer
    end_function

.global	set_stack_ptr
.type	set_stack_ptr, @function
set_stack_ptr:
    .functype set_stack_ptr(i32) -> ()
    local.get 0
    global.set __stack_pointer
    end_function

.global atomic_load_i32
.type atomic_load_i32, @function
atomic_load_i32:
    .functype atomic_load_i32(i32) -> (i32)
    local.get 0
    i32.atomic.load 0
    end_function

.global atomic_store_i32
.type atomic_store_i32, @function
atomic_store_i32:
    .functype atomic_store_i32(i32, i32) -> ()
    local.get 0
    local.get 1
    i32.atomic.store 0
    end_function

.global atomic_rmw_add_i32
.type atomic_rmw_add_i32, @function
atomic_rmw_add_i32:
    .functype atomic_rmw_add_i32(i32, i32) -> (i32)
    local.get 0
    local.get 1
    i32.atomic.rmw.add 0
    end_function

.global atomic_rmw_cmpxchg_i32
.type atomic_rmw_cmpxchg_i32, @function
atomic_rmw_cmpxchg_i32:
    .functype atomic_rmw_cmpxchg_i32(i32, i32, i32) -> (i32)
    local.get 0
    local.get 1
    local.get 2
    i32.atomic.rmw.cmpxchg 0
    end_function

.global atomic_memory_notify
.type atomic_memory_notify, @function
atomic_memory_notify:
    .functype atomic_memory_notify(i32) -> (i32)
    local.get 0
    i32.const 1
    memory.atomic.notify 0
    end_function

.global atomic_memory_wait_i32
.type atomic_memory_wait_i32, @function
atomic_memory_wait_i32:
    .functype atomic_memory_wait_i32(i32, i32) -> (i32)
    local.get 0
    local.get 1
    i64.const -1
    memory.atomic.wait32 0
    end_function
