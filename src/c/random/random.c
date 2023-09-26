#include "../main.h"

u8 u8state[4] = {72, 51, 84, 122};
u32 u32state[6] = {123456789, 362436069, 521288629, 88675123, 5783321, 6615241};

export u8 randomU8(void) {
    u8state[0] += 1;
    u8state[1] = u8state[1] ^ u8state[3] ^ u8state[0];
    u8state[2] += u8state[1];
    u8state[3] = u8state[3] + (u8state[2] >> 1) ^ u8state[1];
    return u8state[3];
}

export u32 randomU32(void) {
    u32 t = u32state[4];
    u32 s = u32state[0];
    u32state[4] = u32state[3];
    u32state[3] = u32state[2];
    u32state[2] = u32state[1];
    u32state[1] = s;
    t ^= t >> 2;
    t ^= t << 1;
    t ^= s ^ (s << 4);
    u32state[0] = t;
    u32state[5] += 362437;
    return t + u32state[5];
}

_Bool randomBool(void) {
    return randomU8() & 1;
}

_Bool randEveryU8(u8 n) {
    return randomU8() % n == 0;
}

_Bool randEveryU32(u32 n) {
    return randomU32() % n == 0;
}

f32 randomF32(void) {
    return (randomU32() >> 8) * (1.0f / (1ul << 24));
}
