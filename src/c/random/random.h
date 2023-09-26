#ifndef RANDOM_H
#define RANDOM_H

#include "../main.h"

#define LORDIR (randomBool() ? -1 : 1)

u8 randomU8(void);
u32 randomU32(void);
_Bool randomBool(void);
_Bool randEveryU8(u8 n);
_Bool randEveryU32(u32 n);
f32 randomF32(void);

#endif
