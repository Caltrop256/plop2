import { elements } from "../glue/elements.js";

const getShaderHead = () => `#version 300 es
${Array.from(elements).map(([type, data]) => `#define ${data.name.toUpperCase()} (${type}u)`).join('\n')}
`

export async function instantiateWebGl(canvas: HTMLCanvasElement, exports: PlopExports) {
    const gl = canvas.getContext('webgl2') as WebGL2RenderingContext;

    const [vertShader, fragShader] = (await Promise.all([
        'vert',
        'elements'
    ].map(name => fetch(`./shaders/${name}.glsl`).then(res => res.text()))))
    .map((source, i) => {
        const isVertexShader = i == 0;
        if(!isVertexShader) source = getShaderHead() + source;
        const shader = gl.createShader(isVertexShader ? gl.VERTEX_SHADER : gl.FRAGMENT_SHADER) as WebGLShader;
        gl.shaderSource(shader, source);
        gl.compileShader(shader);

        if(!gl.getShaderParameter(shader, gl.COMPILE_STATUS)) {
            throw new Error(`Failed to compile shader: ${gl.getShaderInfoLog(shader)}`);
        }

        return shader;
    });

    const program = gl.createProgram() as WebGLProgram;
    gl.attachShader(program, vertShader);
    gl.attachShader(program, fragShader);
    gl.linkProgram(program);

    if(!gl.getProgramParameter(program, gl.LINK_STATUS)) {
        throw new Error(`Failed to link program: ${gl.getProgramInfoLog(program)}`);
    }

    const a_position = gl.getAttribLocation(program, 'a_position');
    const a_texCoord = gl.getAttribLocation(program, 'a_texCoord');

    const u_resolution = gl.getUniformLocation(program, 'u_resolution');
    const u_image = gl.getUniformLocation(program, 'u_image');
    const u_textureAtlas = gl.getUniformLocation(program, 'u_textureAtlas');

    const vao = gl.createVertexArray();
    gl.bindVertexArray(vao);

    const positionBuffer = gl.createBuffer();
    gl.enableVertexAttribArray(a_position);
    gl.bindBuffer(gl.ARRAY_BUFFER, positionBuffer);
    gl.vertexAttribPointer(a_position, 2, gl.FLOAT, false, 0, 0);

    const texCoordBuffer = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, texCoordBuffer);
    gl.bufferData(gl.ARRAY_BUFFER, new Float32Array([
        0.0,  0.0,
        1.0,  0.0,
        0.0,  1.0,
        0.0,  1.0,
        1.0,  0.0,
        1.0,  1.0,
    ]), gl.STATIC_DRAW);
    gl.enableVertexAttribArray(a_texCoord);
    gl.vertexAttribPointer(a_texCoord, 2, gl.FLOAT, false, 0, 0);
    
    gl.useProgram(program);
    gl.bindVertexArray(vao);

    const elementTexture = gl.createTexture();
    gl.activeTexture(gl.TEXTURE0 + 1);
    gl.bindTexture(gl.TEXTURE_2D, elementTexture);

    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.NEAREST);

    const textureData: HTMLImageElement = await new Promise(r => {
        const img = new Image();
        img.onload = () => {
            r(img);
        }
        img.src = './atlas.png';
    })
    gl.texStorage2D(gl.TEXTURE_2D, 1, gl.RGBA32F, textureData.width, textureData.height);
    gl.texSubImage2D(gl.TEXTURE_2D, 0, 0, 0, textureData.width, textureData.height, gl.RGBA, gl.FLOAT, textureData);

    let texture: WebGLTexture | null = null;

    gl.uniform1i(u_image, 0);
    gl.uniform1i(u_textureAtlas, 1);

    const sizes = {
        realWidth: 0,
        realHeight: 0,
        simWidth: 0,
        simHeight: 0,
        scale: 0
    };

    return {
        sizes,
        
        setSize(width: number, height: number, scale: number){
            sizes.realWidth = width;
            sizes.realHeight = height;
            sizes.simWidth = Math.ceil(sizes.realWidth / scale);
            sizes.simHeight = Math.ceil(sizes.realHeight / scale);
            sizes.scale = scale;
    
            gl.uniform2f(u_resolution, sizes.simWidth, sizes.simHeight);
    
            gl.bindBuffer(gl.ARRAY_BUFFER, positionBuffer);
            gl.bufferData(gl.ARRAY_BUFFER, new Float32Array([
                0, 0,
                sizes.simWidth, 0,
                0, sizes.simHeight,
                0, sizes.simHeight,
                sizes.simWidth, 0,
                sizes.simWidth, sizes.simHeight,
            ]), gl.STATIC_DRAW);
    
            gl.viewport(0, 0, width, height);
            gl.canvas.width = width;
            gl.canvas.height = height;


            gl.deleteTexture(texture);
            texture = gl.createTexture();
            gl.activeTexture(gl.TEXTURE0 + 0);
            gl.bindTexture(gl.TEXTURE_2D, texture);

            gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
            gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
            gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST);
            gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.NEAREST);

            gl.texStorage2D(gl.TEXTURE_2D, 1, gl.RGBA8UI, sizes.simWidth, sizes.simHeight);

            exports.setScreenSize(sizes.simWidth, sizes.simHeight, 1);
        },

        render(img: Uint8Array) {
            gl.texSubImage2D(gl.TEXTURE_2D, 0, 0, 0, sizes.simWidth, sizes.simHeight, gl.RGBA_INTEGER, gl.UNSIGNED_BYTE, img);
            gl.drawArrays(gl.TRIANGLES, 0, 6);
        }
    }
}