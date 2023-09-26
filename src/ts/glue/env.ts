export const memory = new WebAssembly.Memory({
    initial: 1500,
    maximum: 1500,
    shared: true
});

export const read_c_str = (ptr: number) => {
    const view = new Uint8Array(memory.buffer, ptr);
    let str = '';
    for(let i = 0; view[i] != 0; ++i) {
        str += String.fromCharCode(view[i]);
    }
    return str;
}

export const env = {
    env: {memory},
    js: {
        log(ptr: number, ...values: number[]){
            let i = 0;
            console.log(read_c_str(ptr).replace(/(%.)/g, (_, t) => (t == '%s' ? read_c_str(values[i++]) : values[i++]).toString()));
        },
        throw: (ptr: number, ...values: number[]) => {
            let i = 0;
            throw new Error(read_c_str(ptr).replace(/(%.)/g, (_, t) => (t == '%s' ? read_c_str(values[i++]) : values[i++]).toString()));
        },
        notify_main_thread() {
            throw new Error('notify main thread called from main thread invoked function!');
        }
    }
}