#define REVERSI_COMPILATION
#include<string.h>
#include"def.h"
#include"move.h"
static inline _Bool is_different_piece(Piece next,Piece p){
    /*这东西是不是也能用宏解决？*/
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
    for(;i!=0;--i){
        reverse_piece(b[x+i][y+i],next);
    }
    b[x][y]=next;
    return true;
}
#undef reverse_piece
static _Bool try_move(Board b,Piece next,int x,int y){
    return left(b,next,x,y)||right(b,next,x,y)||up(b,next,x,y)||down(b,next,x,y)||\
    up_left(b,next,x,y)||up_right(b,next,x,y)||down_left(b,next,x,y)||down_right(b,next,x,y);
}
#define set_move_invalid(p) do{p.x=255}while(0)
_Bool IsValidMove(Point p){
    return p.x!=255;
}
void GetMoves(Board b,Piece next,Moves* m) {
    /*TODO：m判空？*/
    size_t size = 0;
    /*++size的时候再铺新的棋盘，避免操作浪费*/
    memcpy(m->b[0],Board,64);
    for(int i=0;i<8;++i){
        for(int j=0;j<8;++j){
            /*上边两层循环是遍历棋盘每个点位，这可能导致嵌套太深，但是算了别在意了*/
            if(b[i][j]!=EMPTY){
                continue;
            }
            if(try_move(m->b[size],next,i,j)==true){
                ++size;
                m->p.x=i;
                m->p.y=j;
                memcpy(m->b[size],Board,64);
            }
        }
    }
    set_move_invalid(m->p[size]);
}
#undef set_move_invalid