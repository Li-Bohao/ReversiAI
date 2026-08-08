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
_Bool IsValidBoard(Board b){
    return b[3][3]!=EMPTY;
}
void SetBoardInvalid(Board b){
    b[3][3]=EMPTY;
}
#endif