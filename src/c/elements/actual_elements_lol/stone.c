#include "../elements.h"
#include "../../random/random.h"

#define ELEMENT_NAME stone

InteractionResult VARPREF(interact)(CellView *source, CellView *target) {
    if(target->cell->el.type == TYPE_EMPTY) {
        return moveElement(source, target);
    }
    return RESULT_BLOCKED;
}


void VARPREF(update)(UpdateResult *result, CellView *source) {

}

void VARPREF(init)(Element *el) {
    
}

ElementInfo VARPOST(info) = {
    .name = "Stone",
    .description = "",
    .update = VARPREF(update),
    .interact = VARPREF(interact),

    .color = {{0xae,0xae,0xae,0xff}},

    .air = {
        .drag = 10.0f
    },

    .temperature = {
        .conductivity = 1.0f
    }
};


#undef ELEMENT_NAME
