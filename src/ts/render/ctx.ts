export async function instantiateCanvasContext(canvas: HTMLCanvasElement, exports: PlopExports) {
    const ctx = canvas.getContext('2d') as CanvasRenderingContext2D;

    let imageData: ImageData;

    const sizes = {
        realWidth: 0,
        realHeight: 0,
        simWidth: 0,
        simHeight: 0,
        scale: 0
    }

    return {
        sizes,
        setSize(width: number, height: number, scale: number) {
            width = Math.ceil(width);
            height = Math.ceil(height);

            sizes.realWidth = sizes.simWidth = ctx.canvas.width = width;
            sizes.realHeight = sizes.simHeight = ctx.canvas.height = height;
            sizes.scale = scale;

            imageData = new ImageData(width, height);
            exports.setScreenSize(width, height, scale);
        },

        render(img: Uint8Array) {
            imageData.data.set(img);
            ctx.putImageData(imageData, 0, 0);
        }
    };
}