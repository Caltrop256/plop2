import { read_c_str } from "./env.js";


export const elements = new Map<number, {name: string, description: string}>();

export const readElementData = (exports: PlopExports, memory: WebAssembly.Memory) => {
    const [, structSize, numElements] = new Uint32Array(memory.buffer, exports.glueInfo.value, 3);
    const arrayU32 = new Uint32Array(memory.buffer, exports.elementLookup.value, (structSize / 4) * numElements);

    for(let i = 0; i < numElements; ++i) {
        const offset = i * (structSize / 4);  
        elements.set(i, {
            name: read_c_str(arrayU32[offset]) || 'Empty',
            description: read_c_str(arrayU32[offset + 1])
        })
    }

    const elementList = document.querySelector('.debug-info') as HTMLElement;
    for(const [type, elementInfo] of elements) {
        elementList.innerHTML += `<button onclick="globalThis.type=${type}">${elementInfo.name}</button>`
    }
}