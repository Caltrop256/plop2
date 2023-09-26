import { instantiateWebGl } from "./render/gl.js";
import { env, memory } from "./glue/env.js";
import { instantiateCanvasContext } from "./render/ctx.js";
import { elements, readElementData } from "./glue/elements.js";

const mouse = {
    x: -1,
    y: -1,
    holding: false
}
const chunkSize = 64;

const {instance, module} = await fetch('./game.wasm').then(res => WebAssembly.instantiateStreaming(res, env));
const exports = instance.exports as PlopExports;
exports.setup();
const glueInfo = {
    gpuRenderMode: new Uint32Array(memory.buffer, exports.glueInfo.value, 1)[0]
};
readElementData(exports, memory);

const threads = Array.from({length: navigator.hardwareConcurrency + 1}, ()  => new Worker('thread.js', {type: 'module'}));
const threadSchedulePtr = exports.allocThreadScheduleArray(1 + (threads.length - 1) * 5 * 2);
await new Promise((r) => {
    let threadsReady = 0;
    threads.forEach((t, ind) => {
        const setup = (e: MessageEvent) => {
            console.log(`thread #${e.data} ready!`);
            t.removeEventListener('message', setup);
            threadsReady += 1;
            if(threadsReady == threads.length) r(void 0);
        }
        t.addEventListener('message', setup)
        t.postMessage({
            module,
            threadId: ind - 1,
            nThreads: threads.length - 1,
            stackPtr: exports.allocStack(),
            threadSchedulePtr,
            memory
        })
    })
})

const canvas = document.getElementById('plop') as HTMLCanvasElement;
const renderer = await (glueInfo.gpuRenderMode ? instantiateWebGl(canvas, exports) : instantiateCanvasContext(canvas, exports));

export const views = {
    screen: new Uint8Array(),
    camera: {
        chunkXY: new BigInt64Array(memory.buffer, exports.camera.value, 2),
        cellXY: new Float64Array(memory.buffer, exports.camera.value + 16, 2)
    },
    threadScheduleI32: new Int32Array(memory.buffer, threadSchedulePtr, threads.length),
    glueView: {
        cell: new Uint32Array(memory.buffer, exports.glueView.value, 1) 
    }
}

let desiredScale = 4
let resizeTriggered = false;
window.addEventListener('resize', () => resizeTriggered = true);
const resize = () => {
    resizeTriggered = false;

    if(desiredScale != renderer.sizes.scale && renderer.sizes.scale) {
        exports.moveCamera(
            ((mouse.x / renderer.sizes.scale) - (mouse.x / desiredScale)) / chunkSize,
            ((mouse.y / renderer.sizes.scale) - (mouse.y / desiredScale)) / chunkSize
        );
    }

    renderer.setSize(window.innerWidth, window.innerHeight, desiredScale);
    views.screen = new Uint8Array(memory.buffer, new Uint32Array(memory.buffer, exports.screen.value, 1)[0], renderer.sizes.simWidth * renderer.sizes.simHeight * 4);
}
resize();

const mouseInfo = {
    box: document.getElementById('mouse-info') as HTMLElement,
    tileX: document.getElementById('tileX') as HTMLElement,
    tileY: document.getElementById('tileY') as HTMLElement,
    type: document.getElementById('hoveredTileType') as HTMLElement,
    temp: document.getElementById('hoveredCellTemp') as HTMLElement
}

const mouseToTiles = ([x, y]: [number, number]) => [
    views.camera.chunkXY[0] * BigInt(chunkSize) + BigInt(Math.floor(views.camera.cellXY[0] * chunkSize + x / renderer.sizes.scale)),
    views.camera.chunkXY[1] * BigInt(chunkSize) + BigInt(Math.floor(views.camera.cellXY[1] * chunkSize + y / renderer.sizes.scale))
];

const updateMouseOverInfo = () => {
    mouseInfo.box.style.left = `${mouse.x + 4}px`;
    mouseInfo.box.style.top = `${mouse.y + 4}px`;

    const [tileX, tileY] = mouseToTiles([mouse.x, mouse.y]);
    mouseInfo.tileX.innerHTML = tileX.toString();
    mouseInfo.tileY.innerHTML = tileY.toString();

    const chunkX = floorDiv(tileX, BigInt(chunkSize));
    const chunkY = floorDiv(tileY, BigInt(chunkSize));

    const chunkPtr: ptr = exports.getChunk(chunkX, chunkY);
    if(chunkPtr) {
        let cellX = tileX % BigInt(chunkSize);
        let cellY = tileY % BigInt(chunkSize);
        if(cellX < 0) cellX += BigInt(chunkSize);
        if(cellY < 0) cellY += BigInt(chunkSize);
    
        exports.getCellView(exports.glueView.value, chunkPtr, Number(cellX), Number(cellY));
        const cellPtr = views.glueView.cell[0];
        const typeEnum = new Uint8Array(memory.buffer, cellPtr, 1)[0];
        const temperature = new Float32Array(memory.buffer, cellPtr + 8, 1)[0];
        mouseInfo.type.innerHTML = String(elements.get(typeEnum)?.name);
        mouseInfo.temp.innerHTML = `${temperature.toFixed(1)}°C`;
    } else {
        mouseInfo.type.innerHTML = 'Empty';
        mouseInfo.temp.innerHTML = '20.0°C';
    }
}

window.addEventListener('mousemove', e => {
    mouse.x = e.clientX;
    mouse.y = e.clientY;

    updateMouseOverInfo();
});
canvas.addEventListener('mousedown', e => { mouse.holding = true });
window.addEventListener('mouseup', e => { mouse.holding = false });

window.addEventListener('wheel', e => {
    desiredScale = desiredScale + (e.deltaY < 0 ? 1 : -1);
    desiredScale = Math.max(1, Math.min(64, glueInfo.gpuRenderMode ? desiredScale : Math.ceil(desiredScale)));
})

const floorDiv = (a: bigint, b: bigint) => {
    if(!((a < 0n) != (b < 0n))) return a / b;
    if(a % b == 0n) return a / b;

    return (a / b) - 1n;
}

(globalThis as any).type = 2;

const keysPressed = new Set();
const moveSpeed = 0.05;

let startOfFrame = performance.now();
let endOfLastFrame = performance.now();
let frames = 0;
const fpsCounter = document.getElementById('fps') as HTMLElement;

threads[0].addEventListener('message', e => {
    renderer.render(views.screen);

    frames += 1;
    const now = performance.now();
    const ms = now - endOfLastFrame;
    if(now >= endOfLastFrame + 1000) {
        const fps = (frames * 1000) / ms;
        endOfLastFrame = now;
        frames = 0;
        fpsCounter.innerHTML = `${fps.toFixed(1)}fps`;

        if(resizeTriggered) resize();
    }

    if(desiredScale != renderer.sizes.scale) {
        resize();
    }

    updateMouseOverInfo();
    requestAnimationFrame(tick);
    //tick();
})

function tick() {
    startOfFrame = performance.now();

    if(keysPressed.has('w')) exports.moveCamera(0, -moveSpeed);
    if(keysPressed.has('a')) exports.moveCamera(-moveSpeed, 0);
    if(keysPressed.has('s')) exports.moveCamera(0, moveSpeed);  
    if(keysPressed.has('d')) exports.moveCamera(moveSpeed, 0);

    if(mouse.holding) {
        const [tileX, tileY] = mouseToTiles([mouse.x, mouse.y]);
        const chunkX = floorDiv(tileX, BigInt(chunkSize));
        const chunkY = floorDiv(tileY, BigInt(chunkSize));
        
        let cellX = tileX % BigInt(chunkSize);
        let cellY = tileY % BigInt(chunkSize);
        if(cellX < 0) cellX += BigInt(chunkSize);
        if(cellY < 0) cellY += BigInt(chunkSize);
    
        exports.paint(chunkX, chunkY, Number(cellX), Number(cellY), 4, (globalThis as any).type);
    }
    
    Atomics.store(views.threadScheduleI32, 0, 1);
    Atomics.notify(views.threadScheduleI32, 0);
};
tick();

window.addEventListener('keydown', e => keysPressed.add(e.key.toLowerCase()));
window.addEventListener('keyup', e => keysPressed.delete(e.key.toLowerCase()));