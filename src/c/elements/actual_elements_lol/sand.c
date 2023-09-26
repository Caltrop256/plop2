#include "../elements.h"
#include "../../random/random.h"

#define ELEMENT_NAME sand

InteractionResult VARPREF(interact)(CellView *source, CellView *target) {
    switch(target->cell->el.type) {
        case TYPE_EMPTY :
            return moveElement(source, target);
        case TYPE_WATER :
            source->keepAlive = 1;

            if(randEveryU8(4)) {
                return swapElements(source, target);
            }
            // fall through
        default :
            return RESULT_BLOCKED;
    }
}

void VARPREF(update)(UpdateResult *result, CellView *source) {
    if(TRY(0, 1) & RESULT_CONCLUDED) return;
    i8 dir = LORDIR;
    if(TRY(dir, 1) & RESULT_CONCLUDED) return;
    if(TRY(-dir, 1) & RESULT_CONCLUDED) return;
}

void VARPREF(init)(Element *el) {
    
}

ElementInfo VARPOST(info) = {
    .name = "Sand",
    .description = "the the the",
    .update = VARPREF(update),
    .interact = VARPREF(interact),

    .color = {{249,244,67,0xff}},

    .air = {
        .drag = 2.5f
    }
};


#undef ELEMENT_NAME
