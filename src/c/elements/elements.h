#ifndef ELEMENTS_H
#define ELEMENTS_H

#include "../main.h"
#include "../color.h"
#include "../map.h"
#include "../fluidsim.h"
#include "elementdata.h"

#define SUBPIXEL_FP_SIZE 4
#define SUBPIXEL_FP_HSIZE (SUBPIXEL_FP_SIZE >> 1)

#define SUBPIXEL_FP_TO_I8(FP) ((i8)(FP - (1 << SUBPIXEL_FP_HSIZE)))
#define SUBPIXEL_FP_TO_U4(FP) (FP + (1 << SUBPIXEL_FP_HSIZE))

#define FLOAT_TO_SUBPIXEL_FP(x) ((x * (1 << SUBPIXEL_FP_HSIZE)))
#define SUBPIXEL_FP_TO_FLOAT(FP) ((f32)(FP - (1 << SUBPIXEL_FP_HSIZE)) / (1 << SUBPIXEL_FP_HSIZE))

#define SUBPIXEL_FP_GET_DECIMAL(FP) (FP & ((1 << SUBPIXEL_FP_HSIZE) - 1))
#define SUBPIXEL_FP_FLOOR(FP) (FP >> SUBPIXEL_FP_HSIZE)


typedef struct Element {
    ElementType type;
    u8 tick: 1;
    u8 unusedFlag: 1;
    u8 textureX: 3;
    u8 textureY: 3;
    RGB332 color;
    u8 subPixelX: SUBPIXEL_FP_SIZE;
    u8 subPixelY: SUBPIXEL_FP_SIZE;

    union {
        #define GENERATE_UNION(UPPER, LOWER) struct LOWER ## _data LOWER;
        FOREACH_ELEMENTTYPE_WITH_STRUCT_DATA(GENERATE_UNION)
        #undef GENERATE_UNION

        union {
            struct {
                u8 ra;
                u8 rb;
                u8 rc;
                u8 rd;
            };
            u32 r;
        } registers;
    };
} Element;
STATIC_ASSERT(sizeof(Element) == sizeof(u64), "Element struct not 64 bits!");

typedef struct GPUElement {
    ElementType type;
    u8 textureX;
    u8 textureY;
    u8 color;
} GPUElement;
STATIC_ASSERT(sizeof(GPUElement) == sizeof(RGBA8888), "GPUElement / RGBA8888 struct size mismatch!");

#define CHUNKSHIFT 6
#define CHUNKSIZE (1 << CHUNKSHIFT)
#define SAFE_MOVEMENT_DIST (CHUNKSIZE >> 1)

typedef struct Cell {
    Element el;
    f32 temperature[2];
} Cell;

typedef union DirtyRect {
    struct {
        u8 minX;
        u8 maxX;
        u8 minY;
        u8 maxY;
    };
    i32 value;
} DirtyRect;

typedef enum AllocationPriority {
    PRIORITY_DEALLOCATION_RECOMMENDED,

    PRIORITY_VERY_LOW, // adjacent to higher priority chunk, active temperature
    PRIORITY_LOW, // active cells
    PRIORITY_MEDIUM, // active cells, |chunk.xy - camera.xy| < N, 
    PRIORITY_HIGH, // in view, active temperature OR elementsInChunk > 0
    PRIORITY_VERY_HIGH, // in view, active cells

    NUMBER_OF_PRIORITIES

} AllocationPriority;

typedef struct Chunk {
    DirtyRect dirtyRect;
    i64 x;
    i64 y;
    struct Chunk *neighbors[3][3];
    struct Chunk *prev;
    struct Chunk *next;
    struct {
        struct Chunk *child[2];
        u32 height;
    } AVL;

    Cell cells[CHUNKSIZE][CHUNKSIZE];

    u32 elementsInChunk;
    _Bool active;
    u8 activeNeighbors;
    u8 keepAliveTimer;
    _Bool temperatureActive;
    AllocationPriority priority;

    _Bool awesomePuwaDebugReturns;
} Chunk;

typedef struct CellView {
    Cell *cell;
    Chunk *chunk;
    u16 x;
    u16 y;
    _Bool invalid;

    _Bool keepAlive;
} CellView;

#define TEMPERATURE(cv) ((cv)->cell->temperature[world.tick])
#define CELLVIEW_FROM_CHUNK(CHUNK, X, Y) ((CellView){.cell = &(CHUNK)->cells[Y][X], .chunk = CHUNK, .x = (X), .y = (Y), .invalid = 0})

typedef struct CellViewDeltaDistance {
    i16 x;
    i16 y;
} CellViewDeltaDistance;

typedef enum InteractionResult {
    RESULT_NOTARGET = 0,
    RESULT_BLOCKED = (1 << 0),

    RESULT_MOVED = (1 << 1),
    RESULT_SWAPPED = (1 << 2),
    RESULT_DELETED = (1 << 3),
    RESULT_REPLACED = (1 << 4),

    RESULT_CONCLUDED = RESULT_MOVED | RESULT_SWAPPED | RESULT_DELETED | RESULT_REPLACED,
    RESULT_NOTCONCLUDED = RESULT_BLOCKED
} InteractionResult;

void spawnElementOnCellView(CellView *cv, ElementType type);
InteractionResult freeElement(CellView *cv);
InteractionResult replaceElement(CellView *cv, ElementType type);

CellView *getCellView(CellView *cv, Chunk *chunk, i16 x, i16 y);
CellViewDeltaDistance getCellViewDeltaXY(CellView *cv0, CellView *cv1);
InteractionResult moveElement(CellView *source, CellView *target);
InteractionResult swapElements(CellView *source, CellView *target);
void updateChunkDirtyRect(CellView *cv);
void applyTemperature(CellView *cv, f32 deltaTemperature);

i32 getFluidInfo(CellView *cv);

typedef struct UpdateResult {
    CellView target;
    InteractionResult result;
} UpdateResult;

typedef struct MultiStepResult {
    CellView destination;
    InteractionResult result;
} MultiStepResult;

void multiStepElement(MultiStepResult *result, CellView *source, i16 dx, i16 dy);

typedef struct PhaseTransition {
    _Bool occurs;
    ElementType into;
    f32 transitionPoint;
} PhaseTransition;

typedef struct ElementInfo {
    const char *name;
    const char *description;
    void (*update)(UpdateResult *, CellView *);
    InteractionResult (*interact)(CellView *, CellView *);

    RGBA8888 color;

    struct {
        f32 drag;
    } air;

    struct {
        f32 conductivity;
        PhaseTransition lowTemperatureTransition;
        PhaseTransition highTemperatureTransition;
    } temperature;
} ElementInfo;

void setupElementInfo(void);
export extern ElementInfo elementLookup[type_length];

#define GENERATE_EXTERN_ELEMENTINFO(UPPER, LOWER) extern ElementInfo LOWER##_info;
FOREACH_ELEMENTTYPE(GENERATE_EXTERN_ELEMENTINFO)
#undef GENERATE_EXTERN_ELEMENTINFO

#define VARJOINERINNER(A, B) A ## _ ## B
#define VARJOINER(A, B) VARJOINERINNER(A, B)
#define EXPAND_MACRO(x) x
#define VARPREF(PREFIX) VARJOINER(PREFIX, EXPAND_MACRO(ELEMENT_NAME))
#define VARPOST(POSTFIX) VARJOINER(EXPAND_MACRO(ELEMENT_NAME), POSTFIX)

#define TRY(OFFSET_X, OFFSET_Y) \
    (result->result = VARPREF(interact)(source, getCellView(&result->target, source->chunk, source->x + (OFFSET_X), source->y + (OFFSET_Y))))

#define PROP(cv) ((cv)->cell->el.EXPAND_MACRO(ELEMENT_NAME))

#define AMBIENT_TEMPERATURE 20.0f

typedef struct World {
    _Bool tick;

    Chunk *chunkThreadingList[4];
    u32 chunksInList[4];

    Chunk chunks[1024];
    u32 freeChunkList[NUMBER_OF_PRIORITIES][32];

    Chunk *root;

    hashmap *map;
    FluidSim fluid;
} World;

extern World world;

#endif
