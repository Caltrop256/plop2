self.addEventListener('message', e => {
    const {module, threadId, nThreads, stackPtr, threadSchedulePtr, memory} = e.data as {
        module: WebAssembly.Module, 
        threadId: number, 
        nThreads: number,
        stackPtr: number,
        threadSchedulePtr: number,
        memory: WebAssembly.Memory
    };

    const readStr = (ptr: number) => {
        const view = new Uint8Array(memory.buffer, ptr);
        let str = '';
        for(let i = 0; view[i] != 0; ++i) {
            str += String.fromCharCode(view[i]);
        }
        return str;
    }
    
    const env = {
        env: {memory},
        js: {
            log(ptr: number, ...values: number[]){
                let i = 0;
                console.log(readStr(ptr).replace(/(%.)/g, (_, t) => (t == '%s' ? readStr(values[i++]) : values[i++]).toString()));
            },
            throw: (ptr: number, ...values: number[]) => {
                let i = 0;
                throw new Error(readStr(ptr).replace(/(%.)/g, (_, t) => (t == '%s' ? readStr(values[i++]) : values[i++]).toString()));
            },
            notify_main_thread() {
                self.postMessage(threadId);
            }
        }
    }

    WebAssembly.instantiate(module, env).then(instance => {
        const exports = instance.exports as PlopExports;

        exports.set_stack_ptr(stackPtr);
        self.postMessage(threadId);

        if(threadId != -1) {
            exports.slaveLoop(threadId, nThreads);
        } else {
            exports.schedulerLoop(nThreads);
        }
    });
})