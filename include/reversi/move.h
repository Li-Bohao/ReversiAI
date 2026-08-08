#ifndef REVERSI_MOVE_H
#define REVERSI_MOVE_H
#include<stdint.h>
#include<string.h>
typedef struct s_point{
    uint8_t x, y;
} point;
constexpr uint8_t MAX_MOVE = 36; /*据研究，合法的棋局最多只能有这么多种移动方法*/
typedef struct s_Moves{
    point p[MAX_MOVE];
    Board b[MAX_MOVE];
} Moves;
static inline _Bool is_different_piece(Piece next,Piece p){
    return p!=EMPTY&&next!=p;
}
#ifdef XOR_REVERSING
#define reverse_piece(old_p,new_p) do{old_p^=1;}while(0)
#else
#define reverse_piece(old_p,new_p) do{old_p=new_p;}while(0)
#endif
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
    for(;i!=y;++i){
        reverse_piece(b[x][i],next);
    }
    b[x][y]=next;
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
    for(;i!=y;--i){
        reverse_piece(b[x][i],next);
    }
    b[x][y]=next;
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
    for(;i!=x;++i){
        reverse_piece(b[i][y],next);
    }
    b[x][y]=next;
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
    for(;i!=x;--i){
        reverse_piece(b[i][y],next);
    }
    b[x][y]=next;
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
    for(;i!=0;--i){
        reverse_piece(b[x-i][y-i],next);
    }
    b[x][y]=next;
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
    for(;i!=0;--i){
        reverse_piece(b[x-i][y+i],next);
    }
    b[x][y]=next;
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
    for(;i!=0;--i){
        reverse_piece(b[x+i][y-i],next);
    }
    b[x][y]=next;
    return true;
}
static inline _Bool down_right(Board b,Piece next,int x,int y){
    int i;
    _Bool has_different_piece=false;
    for(i=1;x+i<8&&y+i<0;++i){
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
    for(;i!=0;--i){
        reverse_piece(b[x+i][y+i],next);
    }
    b[x][y]=next;
    return true;
}
#undef reverse_piece
_Bool try_move(Board b,Piece next,int x,int y){

}
void GetMoves(Board b,Piece next,Moves* m) {
    /*TODO：m判空？*/
    _Bool valid;
    size_t size = 0;
    
    memcpy(m->b[0],Board,64);
    /*++size的时候再铺新的棋盘，避免操作浪费*/
    for(int i=0;i<8;++i){
        for(int j=0;j<8;++j){
            /*上边两层循环是遍历棋盘每个点位，这可能导致嵌套太深，但是算了别在意了*/
            if(b[i][j]!=EMPTY){
                continue;
            }
            valid=false;

            if(valid==true){
                ++size;
                memcpy(m->b[size],Board,64);
            }
        }
    }
    SetBoardInvalid(m->b[size]);
}
#endif