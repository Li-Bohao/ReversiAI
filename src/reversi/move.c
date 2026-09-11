#define REVERSI_COMPILATION
#include<string.h>
#include<stdint.h>
#include"reversi/def.h"
#include"reversi/move.h"
static inline _Bool is_different_piece(Piece next,Piece p){
    return p!=EMPTY&&next!=p;
}
static inline _Bool left(Board b,Piece next,int x,int y){
    int i;
    _Bool has_different_piece=false;
    for(i=y-1;i>=0;--i){
        if(is_different_piece(next,b[x][i])){
            has_different_piece=true;
        }else if(b[x][i]==EMPTY){
            return false;
        }else{
            break;
        }
    }
    if(i==-1||!has_different_piece){
        return false;
    }
    return true;
}
static inline _Bool right(Board b,Piece next,int x,int y){
    int i;
    _Bool has_different_piece=false;
    for(i=y+1;i<8;++i){
        if(is_different_piece(next,b[x][i])){
            has_different_piece=true;
        }else if(b[x][i]==EMPTY){
            return false;
        }else{
            break;
        }
    }
    if(i==8||!has_different_piece){
        return false;
    }
    return true;
}
static inline _Bool up(Board b,Piece next,int x,int y){
    int i;
    _Bool has_different_piece=false;
    for(i=x-1;i>=0;--i){
        if(is_different_piece(next,b[i][y])){
            has_different_piece=true;
        }else if(b[i][y]==EMPTY){
            return false;
        }else{
            break;
        }
    }
    if(i==-1||!has_different_piece){
        return false;
    }
    return true;
}
static inline _Bool down(Board b,Piece next,int x,int y){
    int i;
    _Bool has_different_piece=false;
    for(i=x+1;i<8;++i){
        if(is_different_piece(next,b[i][y])){
            has_different_piece=true;
        }else if(b[i][y]==EMPTY){
            return false;
        }else{
            break;
        }
    }
    if(i==8||!has_different_piece){
        return false;
    }
    return true;
}
static inline _Bool up_left(Board b,Piece next,int x,int y){
    int i;
    _Bool has_different_piece=false;
    for(i=1;x-i>=0&&y-i>=0;++i){
        if(is_different_piece(next,b[x-i][y-i])){
            has_different_piece=true;
        }else if(b[x-i][y-i]==EMPTY){
            return false;
        }else{
            break;
        }
    }
    if(x-i==-1||y-i==-1||!has_different_piece){
        return false;
    }
    return true;
}
static inline _Bool up_right(Board b,Piece next,int x,int y){
    int i;
    _Bool has_different_piece=false;
    for(i=1;x-i>=0&&y+i<8;++i){
        if(is_different_piece(next,b[x-i][y+i])){
            has_different_piece=true;
        }else if(b[x-i][y+i]==EMPTY){
            return false;
        }else{
            break;
        }
    }
    if(x-i==-1||y+i==8||!has_different_piece){
        return false;
    }
    return true;
}
static inline _Bool down_left(Board b,Piece next,int x,int y){
    int i;
    _Bool has_different_piece=false;
    for(i=1;x+i<8&&y-i>=0;++i){
        if(is_different_piece(next,b[x+i][y-i])){
            has_different_piece=true;
        }else if(b[x+i][y-i]==EMPTY){
            return false;
        }else{
            break;
        }
    }
    if(x+i==8||y-i==-1||!has_different_piece){
        return false;
    }
    return true;
}
static inline _Bool down_right(Board b,Piece next,int x,int y){
    int i;
    _Bool has_different_piece=false;
    for(i=1;x+i<8&&y+i<8;++i){
        if(is_different_piece(next,b[x+i][y+i])){
            has_different_piece=true;
        }else if(b[x+i][y+i]==EMPTY){
            return false;
        }else{
            break;
        }
    }
    if(x+i==8||y+i==8||!has_different_piece){
        return false;
    }
    return true;
}
static _Bool try_move(Board b,Piece next,int x,int y){
    return left(b,next,x,y)||right(b,next,x,y)||up(b,next,x,y)||down(b,next,x,y)||\
    up_left(b,next,x,y)||up_right(b,next,x,y)||down_left(b,next,x,y)||down_right(b,next,x,y);
}
void GetMoves(Board b,Piece next,Moves* m) {
    m->count=0;
    for(int i=0;i<8;++i){
        for(int j=0;j<8;++j){
            if(b[i][j]!=EMPTY){
                continue;
            }
            if(try_move(b,next,i,j)==true){
                m->p[m->count].x=i;
                m->p[m->count].y=j;
                ++m->count;
            }
        }
    }
}