#ifndef TOOLS_HASH_TYPES_H
#define TOOLS_HASH_TYPES_H
typedef uint32_t HashIndex;
constexpr HashIndex INVALID_HASH_INDEX=UINT32_MAX;
inline _Bool IsValidHashIndex(HashIndex hi){
    return hi!=INVALID_HASH_INDEX;
}
#endif