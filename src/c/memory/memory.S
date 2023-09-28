.global memcpy
.type   memcpy, @function
memcpy:
    .functype memcpy(i32, i32, i32) -> (i32)
    local.get 0
    local.get 1
    local.get 2
    memory.copy 0, 0
    local.get 0
    end_function

.global memset
.type   memset, @function
memset:
    .functype memset(i32, i32, i32) -> (i32)
    local.get 0
    local.get 1
    local.get 2
    memory.fill 0
    local.get 0
    end_function
