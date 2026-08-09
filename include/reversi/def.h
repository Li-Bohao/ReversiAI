#if !defined(REVERSI_GUARD)&&!defined(REVERSI_COMPILATION)
#error "Only \"reversi.h\" can be include directly!"
#endif
#ifndef REVERSI_DEF_H
#define REVERSI_DEF_H
#include <stdint.h>
typedef uint8_t Piece;
static constexpr Piece EMPTY = 0, BLACK = 2, WHITE = 3;
typedef Piece Board[8][8];
static constexpr Board INITIAL_BOARD = {
    {EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY},
    {EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY},
    {EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY},
    {EMPTY, EMPTY, EMPTY, WHITE, BLACK, EMPTY, EMPTY, EMPTY},
    {EMPTY, EMPTY, EMPTY, BLACK, WHITE, EMPTY, EMPTY, EMPTY},
    {EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY},
    {EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY},
    {EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY}
};
typedef s_CompressedBoard{
    uint64_t high,low;
} CompressedBoard;
CompressedBoard Compress(Board b);
void Decompress(Board b,CompressedBoard cb);
#endif