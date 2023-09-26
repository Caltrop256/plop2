// DONT EDIT THIS CODE DIRECTLY!

#ifndef ELEMENTDATA_H
#define ELEMENTDATA_H

#include "../main.h"

#define FOREACH_ELEMENTTYPE(M)\
    M(CLONER,cloner)\
    M(FIRE,fire)\
    M(PUWA,puwa)\
    M(SAND,sand)\
    M(STEAM,steam)\
    M(STONE,stone)\
    M(WATER,water)\

typedef enum ElementType {
    TYPE_EMPTY,
    #define GENERATE_ENUM(ENUM,_) TYPE_##ENUM,
    FOREACH_ELEMENTTYPE(GENERATE_ENUM)
    #undef GENERATE_ENUM
    type_length
} __attribute__((__packed__)) ElementType;

#define FOREACH_ELEMENTTYPE_WITH_STRUCT_DATA(M)\
    M(CLONER,cloner)\
    M(FIRE,fire)\
    M(PUWA,puwa)\
    M(WATER,water)

struct cloner_data {
    ElementType type;
};
struct fire_data {
    u8 health;
};
struct puwa_data {
    u8 health;
};
struct water_data {
    _Bool preferRight;
};

#endif
