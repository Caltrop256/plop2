precision highp float;

uniform highp usampler2D u_image;
uniform highp sampler2D u_textureAtlas;
in vec2 v_texCoord;
in vec2 v_resolution;

out vec4 outColor;

const vec4 bgColorBottom = vec4(2.0, 1.0, 19.0, 255.0) / 255.0;
const vec4 bgColorTop = vec4(6.0, 4.0, 36.0, 255.0) / 255.0;

vec4 getTexture(uint type, uvec2 location) {
    if(type == 0u) return mix(bgColorTop, bgColorBottom, v_texCoord.y);

    uvec2 textureOffset = uvec2(0u, (type - 1u) * 8u);

    return texture(
        u_textureAtlas, 
        vec2(location % uvec2(8u) + textureOffset) / vec2(8.0, 64.0)
    );
}

vec4 RGB332toRGBA(uint c) {
    return vec4(
        float((c & 224u) >> 5u) / 7.0,
        float((c & 23u) >> 2u) / 7.0,
        float(c & 3u) / 3.0,
        1.0
    );
}

uvec4 getCell(ivec2 pos) {
    return texture(u_image, v_texCoord + vec2(pos) / v_resolution);
}

#define KERNEL_SIZE 1
#define KERNEL_DIVISOR (float(KERNEL_SIZE) * float(KERNEL_SIZE) + float(KERNEL_SIZE) * float(KERNEL_SIZE))

void main() {
    vec4 clr = vec4(0.0);

    for(int y = -KERNEL_SIZE; y <= KERNEL_SIZE; ++y) {
        for(int x = -KERNEL_SIZE; x <= KERNEL_SIZE; ++x) {
            uvec4 data = getCell(ivec2(x, y));

            bool applyBloom = (x == 0 && y == 0) || ((data.x == FIRE || data.x == STEAM));

            if(applyBloom) {
                float xxyy = float(x) * float(x) + float(y) * float(y);
                clr += getTexture(data.x, data.yz) * (1.0 - xxyy / KERNEL_DIVISOR);
            }
        }
    }

    uvec4 data = getCell(ivec2(0, 0));
    clr += vec4(float(data.w) / 255.0, float(data.w) / 512.0, 0.0, 0.0);

    switch(data.x) {
        case 6u :
            outColor = mix(clr, mix(bgColorTop, bgColorBottom, v_texCoord.y), 0.5);
            break;
        default :
            outColor = clr;
            break;
    }
    
}
