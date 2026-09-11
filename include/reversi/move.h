#if !defined(REVERSI_GUARD)&&!defined(REVERSI_COMPILATION)
#error "Only \"reversi/reversi.h\" can be include directly!"
#endif
#ifndef REVERSI_MOVE_H
#define REVERSI_MOVE_H
#include<stdint.h>
#define REVERSI_COMPILATION
#include"reversi/def.h"
typedef struct{
    uint8_t x, y;
} Point;
constexpr uint8_t MAX_MOVE = 33; /*据研究，合法的棋局最多只能有这么多种移动方法*/
typedef struct{
    Point p[MAX_MOVE];
    uint8_t count;
} Moves;
void GetMoves(Board b,Piece next,Moves* m);
void Move(Board b,Piece next,Point p);
#endif