#include "../elements.h"
#include "../../random/random.h"

#define ELEMENT_NAME cloner

InteractionResult VARPREF(interact)(CellView *source, CellView *target) {
    return RESULT_BLOCKED;
}


void VARPREF(update)(UpdateResult *result, CellView *source) {
    if(PROP(source).type != TYPE_EMPTY) {
        for(i8 y = -1; y <= 1; ++y) {
            for(i8 x = -1; x <= 1; ++x) {
                if(x == 0 && y == 0) continue;
                
                getCellView(&result->target, source->chunk, source->x + x, source->y + y);
                if(!result->target.invalid && result->target.cell->el.type == TYPE_EMPTY) {
                    source->keepAlive = 1;
                    if(randEveryU8(4)) spawnElementOnCellView(&result->target, PROP(source).type);
                }
            }
        }
    } else {
        for(i8 y = -1; y <= 1; ++y) {
            for(i8 x = -1; x <= 1; ++x) {
                if(x == 0 && y == 0) continue;
                getCellView(&result->target, source->chunk, source->x + x, source->y + y);
                if(result->target.invalid) continue;
                ElementType foreignType = result->target.cell->el.type;
                if(foreignType != TYPE_EMPTY && foreignType != TYPE_CLONER) {
                    PROP(source).type = foreignType;
                    source->keepAlive = 1;
                    break;
                }
            }
        }
    }
}

void VARPREF(init)(Element *el) {
    
}

struct data {
    ElementType type;
};

ElementInfo VARPOST(info) = {
    .name = "Cloner",
    .description = "",
    .update = VARPREF(update),
    .interact = VARPREF(interact),

    .color = {{255,23,139,0xff}}
};


#undef ELEMENT_NAME
