type ptr = number;

type PlopExports = {
    malloc(size: bigint | number): ptr,
    free(ptr: ptr),

    get_stack_ptr(): ptr,
    set_stack_ptr(stack_ptr: ptr): undefined,
    allocThreadScheduleArray(n: number): number,
    allocStack(): number

    setup(): undefined,
    setScreenSize(width: number, height: number, scale: number): undefined,

    schedulerLoop(nThreads: number): undefined,
    slaveLoop(threadId: number, nThreads: number): undefined,

    getChunk(x: bigint, y: bigint): ptr,
    getCellView(chunk: ptr, cellView: ptr, x: number, y: number): ptr,

    paint(chunkX: bigint, chunkY: bigint, cellX: number, cellY: number, brushSize: number, type: number)

    moveCamera(dx: number, dy: number);

    screen: WebAssembly.Global,
    camera: WebAssembly.Global,
    glueInfo: WebAssembly.Global,
    glueView: WebAssembly.Global,
    elementLookup: WebAssembly.Global
}
