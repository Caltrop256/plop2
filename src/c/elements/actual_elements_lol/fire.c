#include "../elements.h"
#include "../../random/random.h"

#define ELEMENT_NAME fire

InteractionResult VARPREF(interact)(CellView *source, CellView *target) {
    switch(target->cell->el.type) {
        case TYPE_EMPTY : 
            return moveElement(source, target);
        case TYPE_WATER :
            replaceElement(target, TYPE_STEAM);
            return freeElement(source);
        default :
            return RESULT_BLOCKED;
    }
}

void VARPREF(update)(UpdateResult *result, CellView *source) {
    if(PROP(source).health == 0) {
        result->result = freeElement(source);
        return;
    }
    PROP(source).health -= 1;
    source->keepAlive = 1;

    applyTemperature(source, 2.0f);
    i32 fluidInd = getFluidInfo(source);
    if(fluidInd != -1) {
        APPROACHIFMORE(world.fluid.vy[fluidInd], -1.3f, 0.4f);
    }

    i32 dirY = LORDIR;
    i32 dirX = LORDIR;

    if(TRY(dirX, dirY) & RESULT_CONCLUDED) return;
}

void VARPREF(init)(Element *el) {
    el->fire.health = randomU8();
}

struct data {
    u8 health;
};

ElementInfo VARPOST(info) = {
    .name = "Fire",
    .description = "burns",
    .update = VARPREF(update),
    .interact = VARPREF(interact),

    .color = {{255,118,54,0xff}},

    .air = {
        .drag = 0.0f
    }
};


#undef ELEMENT_NAME
