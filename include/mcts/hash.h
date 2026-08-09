#ifndef MCTS_HASH_H
#define MCTS_HASH_H
#include"reversi.h"
static constexpr int HASH_SIZE = 10000019;
static constexpr int HASH_FIND_FAIL = -1;
typedef struct{
    Point p[MAX_MOVE];
    CompressedBoard cb[MAX_MOVE];
} HashMoves;
typedef struct{
    Board b;
    HashMoves hm;
} HashNode;
#endif