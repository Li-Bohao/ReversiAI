#define REVERSI_COMPILATION
#include"def.h"
#include"judge.h"
Piece judge(Board b){
    int black=0,white=0;
    for(int i=0;i<8;++i){
        for(int j=0;j<8;++j){
            if(b[i][j]==BLACK){
                ++black;
            }else if(b[i][j]==WHITE){
                ++white;
            }
        }
    }
    if(black>white){
        return BLACK;
    }else if(black<white){
        return white;
    }else{
        return EMPTY;
    }
}