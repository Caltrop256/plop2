#include "../elements.h"
#include "../../random/random.h"

#define ELEMENT_NAME steam

InteractionResult VARPREF(interact)(CellView *source, CellView *target) {
    switch(target->cell->el.type) {
        case TYPE_EMPTY :
            return moveElement(source, target);
        default :
            return RESULT_BLOCKED;
    }
}

void VARPREF(update)(UpdateResult *result, CellView *source) {
    source->keepAlive = 1;

    i32 dirX = randomU8() % 3 - 1;

    if(TRY(dirX, -1) & RESULT_CONCLUDED) return;
}

void VARPREF(init)(Element *el) {
    
}

ElementInfo VARPOST(info) = {
    .name = "Steam",
    .description = "watr",
    .update = VARPREF(update),
    .interact = VARPREF(interact),

    .color = {{182,182,182,0xff}},

    .temperature = {
        .conductivity = 0.1f,
        .lowTemperatureTransition = {
            .occurs = 1,
            .into = TYPE_WATER,
            .transitionPoint = 90.0f
        }
    }
};


#undef ELEMENT_NAME
