#include "../elements.h"
#include "../../random/random.h"

#define ELEMENT_NAME puwa

InteractionResult VARPREF(interact)(CellView *source, CellView *target) {
    ElementType type = randomU8() % (type_length - 1) + 1;
    switch(target->cell->el.type) {
        case TYPE_EMPTY : 
            return moveElement(source, target);
        default :
            replaceElement(target, type);
        case TYPE_PUWA :
            return RESULT_BLOCKED;
    }
}


void VARPREF(update)(UpdateResult *result, CellView *source) {
    if(PROP(source).health == 0) {
        result->result = RESULT_DELETED;
        freeElement(source);
        return;
    }
    PROP(source).health -= 1;

    i8 dx = LORDIR;
    i8 dy = LORDIR;

    TRY(dx, dy);
}

void VARPREF(init)(Element *el) {
    el->puwa.health = randomU8() % 128;
}

struct data {
    u8 health;
};

ElementInfo VARPOST(info) = {
    .name = "Puwa",
    .description = "",
    .update = VARPREF(update),
    .interact = VARPREF(interact),

    .color = {{0xae,0xae,0xae,0xff}},

    .air = {
        .drag = 2.0f
    },

    .temperature = {
        .conductivity = 1.0f
    }
};


#undef ELEMENT_NAME
