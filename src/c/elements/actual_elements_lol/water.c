#include "../elements.h"
#include "../../random/random.h"

#define ELEMENT_NAME water

InteractionResult VARPREF(interact)(CellView *source, CellView *target) {
    switch(target->cell->el.type) {
        case TYPE_EMPTY :
            return moveElement(source, target);
        case TYPE_STEAM :
            return swapElements(source, target);
        default :
            return RESULT_BLOCKED;
    }
}

void VARPREF(update)(UpdateResult *result, CellView *source) {
    if(TRY(0, 1) & RESULT_CONCLUDED) return;

    i8 dir = PROP(source).preferRight ? 1 : -1;
    if(TRY(dir, 1) & RESULT_CONCLUDED) return;
    if(TRY(dir, 0) & RESULT_CONCLUDED) return;

    PROP(source).preferRight = !PROP(source).preferRight;
    
    if(TRY(-dir, 1) & RESULT_CONCLUDED) return;
    if(TRY(-dir, 0) & RESULT_CONCLUDED) return;
}

void VARPREF(init)(Element *el) {
    el->water.preferRight = randomBool();
}

struct data {
    _Bool preferRight;
};

ElementInfo VARPOST(info) = {
    .name = "Water",
    .description = "wwww",
    .update = VARPREF(update),
    .interact = VARPREF(interact),

    .color = {{68,205,249,0xff}},

    .air = {
        .drag = 0.5f
    },

    .temperature = {
        .conductivity = 0.1f,
        .highTemperatureTransition = {
            .occurs = 1,
            .into = TYPE_STEAM,
            .transitionPoint = 100.0f
        }
    }
};

#undef ELEMENT_NAME
