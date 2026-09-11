#if !defined(REVERSI_GUARD)&&!defined(REVERSI_COMPILATION)
#error "Only \"reversi/reversi.h\" can be include directly!"
#endif
#ifndef REVERSI_DEF_H
#define REVERSI_DEF_H
#include <stdint.h>
typedef uint8_t Piece;
static constexpr Piece EMPTY = 0, BLACK = 2, WHITE = 3, INVALID_PIECE = 1;
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
typedef{
    uint64_t high,low;
}CompressedBoard;
CompressedBoard Compress(Board b);
void Decompress(Board b,CompressedBoard cb);
_Bool IsValidPiece(Piece p);
#define ReversePiece(p) do{\
    p^=1;\
}while(0)
#endif