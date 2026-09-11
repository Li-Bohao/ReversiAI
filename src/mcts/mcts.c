#include<stdint.h>
#include<math.h>
#include<stdlib.h>
#include<float.h>
#include"mcts/mcts.h"
#include"reversi/reversi.h"
#include"tools/hash_types.h"
typedef struct{
    uint64_t visit;
    uint64_t win2;
    Piece turn;
    uint8_t child_num;
    uint8_t unvisited_child;
    uint8_t virtual_visit;
    uint8_t virtual_win2;
    HashIndex child_index[MAX_MOVE];
    uint64_t child_visit[MAX_MOVE];
}MCTSNode;
/*当未访问过时，child_visit[i]存储位置信息，*/
/*31到16位为空，15到8位为x，7到0位为y。*/
#define set_point(p,data)do{\
    data=((uint64_t)p.x<<8)|p.y;\
}while(0)
static inline Point get_point(uint64_t data){
    Point p;
    p.x=data>>8;
    p.y=data&0xff;
    return p;
}
static inline uint64_t splitmix64(uint64_t x) {
    x += 0x9e3779b97f4a7c15ULL;
    x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ULL;
    x = (x ^ (x >> 27)) * 0x94d049bb133111ebULL;
    x ^= x >> 31;
    return x;
}
static uint32_t hash(CompressedBoard c){
    uint64_t h = splitmix64(c.high);
    h ^= splitmix64(c.low + 0x9e3779b97f4a7c15ULL);
    h = splitmix64(h);
    return (uint32_t)h;
}
#if defined(MT_BY_PHYSICAL_CORE) || defined(MT_BY_LOGICAL_CORE)
#define MT_HASHMAP
#endif
#define HMKey CompressedBoard
#define HMValue MCTSTree
#define HMName MCTSReversi
#define HMSize (10000019)
#define HMHash(value) (hash(value))
#include"tools/hash.h"
static inline double child_win_rate(const MCTSNode*n){
    if(n->win2<n->virtual_win2){
        return 0;
    }
    return (double)(n->win2-n->virtual_win2)/2/(n->visit+n->virtual_visit);
}
static inline double child_find_rate(const MCTSNode*n,uint32_t index){
    return sqrt(log((double)n->visit)/n->child_visit[index]);
}
static uint32_t choose_by_uct(MCTSNode* n){
    constexpr double C=sqrt(2);
    double uct_val=DBL_MIN,cwr,cfr,new_uct_val;
    uint32_t choice;
    for(int i=0;i<n->child_num;++i){
        cfr=child_find_rate(n,i);
        HMRead(MCTSTreeHashMap,cwr,child_win_rate,n,i);
        new_uct_val=cwr+C*cfr;
        if(uct_val<=new_uct_val){
            choice=i;
            uct_val=new_uct_val;
        }
    }
    return choice;
}
static inline void set_virtual_loss(MCTSNode* n){
    ++n->virtual_visit;
    n->virtual_win2+=2;
}
static inline void unset_virtual_loss(MCTSNode* n){
    --n->virtual_visit;
    n->virtual_win2-=2;
}
static inline void visit(MCTSNode*n){
    set_virtual_loss(n);
    ++n->visit;
}
typedef struct{
    HashIndex hi;
    Piece p;
}SelectResult;
static inline _Bool is_leaf(SelectResult sr){
    return IsValidPiece(sr.p)&&!IsValidHashIndex(sr.hi);
}
static inline _Bool not_all_visited(SelectResult sr){
    return IsValidPiece(sr.p)&&IsValidHashIndex(sr.hi);
}
static inline _Bool all_visited(SelectResult sr){
    return !IsValidPiece(sr.p)&&!IsValidHashIndex(sr.hi);
}
static inline SelectResult make_leaf(Piece p){
    SelectResult sr;
    sr.hi=INVALID_HASH_INDEX;
    sr.p=p;
    return sr;
}
static inline SelectResult make_not_all_visited(HashIndex hi,Piece p){
    SelectResult sr;
    sr.hi=hi;
    sr.p=p;
    return sr;
}
static inline SelectResult make_all_visited(){
    SelectResult sr;
    sr.p=INVALID_PIECE;
    return sr;
}
static SelectResult select_unselected_child(MCTSNode*n,CompressedBoard cb){
    Board b;
    if(0==n->child_num){
        b=Decompress(cb);
        Piece p=Judge(b);
        return make_leaf(p);
    }
    if(0!=n->unvisited_child){
        Decompress(b,cb);
        --n->unvisited_child;
        uint32_t i=n->unvisited_child;
        Point p=get_point(n->child_visit[i]);
        n->child_visit[i]=1;
        Move(b,n->turn,p);
        CompressedBoard child_cb=Compress(b);
        n->child_index[i]=MCTSTreeHashMapFindOrInsert(child_cb);
        return make_not_all_visited(n->child_index[i],n->turn);
    }else{
        return make_all_visited();
    }
}
static SelectResult visit_and_select_unselected_child(MCTSNode*n,CompressedBoard cb){
    visit(n);
    return select_unselected_child(n,cb);
}
static HashIndex select_by_uct(const MCTSNode*n,uint32_t*index){
    uint32_t i=choose_by_uct(n);
    *index=i;
    return n->child_index[i];
}
static void add_child_visit(MCTSNode*n,uint32_t index){
    ++n->child_visit[index];
}
static SelectResult select_once(HashIndex hi){
    SelectResult ret;
    uint32_t index;
    HMWrite(MCTSTreeHashMap,ret,visit_and_select_unselected_child,hi,MCTSTreeGetKey(hi));
    if(all_visited(ret)){
        HMRead(MCTSTreeHashMap,ret.hi,select_by_uct,hi,&index);
        HMWriteNoRet(MCTSTreeHashMap,add_child_visit,hi,index);
    }
    return ret;
}
static SelectResult select(HashIndex hi,HashIndex*history,uint32_t*history_count){
    /*TODO：检验边界情况？非全探索结点和终局（叶子）结点*/
    SelectResult ret;
    ret.hi=hi;
    do{
        history[*history_count]=ret.hi;
        ++*history_count;
        ret=select_once(ret.hi);
    }while(all_visited(ret));
    if(not_all_visited(ret)){
        history[*history_count]=ret.hi;
        ++*history_count;
    }
    return ret;
}
static _Bool do_expanse(MCTSNode* n,Piece prev_turn,CompressedBoard cb){
    n->visit=1;
    n->win2=0;
    n->virtual_visit=0;
    n->virtual_win2=0;
    Board b;
    Decompress(b,cb);
    Moves m;
    ReversePiece(prev_turn);
    GetMoves(b,prev_turn,m);
    if(0==m.count){
        /*无法落子时由对方尝试*/
        ReversePiece(prev_turn);
        GetMoves(b,prev_turn,m);
        if(0==m.count){
            return false;
        }
    }
    n->turn=prev_turn;
    n->child_num=m.count;
    n->unvisited_child=m.count;
    for(int i=0;i<m.count;++i){
        set_point(m.p[i],n->child_visit[i]);
    }
    return true;
}
static _Bool expanse(HashIndex hi,Piece prev_turn){
    _Bool is_end;
    HMWrite(MCTSTreeHashMap,is_end,do_expanse,hi,prev_turn,MCTSTreeGetKey(hi));
    return is_end;
}
static _Bool simulate_once(Board b,Piece turn){
    Moves m;
    /*TODO：主函数记得初始化随机数种子！*/
    GetMoves(b,turn,m);
    if(0==m.count){
        return false;
    }
    Point p=m.p[rand()%m.count];
    Move(b,turn,p);
    return true;
}
static Piece simulate(Board b,Piece prev_turn){
    /*TODO：是不是该像下一行一样，先把prev转一下*/
    ReversePiece(prev_turn);
    _Bool success;
    do{
        success=simulate_once(b,prev_turn);
        if(false==success){
            ReversePiece(prev_turn);
            success=simulate_once(b,prev_turn);
            if(!success){
                return Judge(b);
            }
        }
        ReversePiece(prev_turn);
    }while(1);
}
static void back_propagate_once(MCTSNode*n,Piece winner){
    unset_virtual_loss(n);
    if(winner==n->turn){
        n->win2+=2;
    }else if(EMPTY==winner){
        ++n->win2;
    }
}
static void back_propagate(HashIndex*history,Piece winner,uint32_t history_count){
    for(int i=0;i<history_count;++i){
        HMWriteNoRet(MCTSTreeHashMap,back_propagate_once,history[i],winner);
    }
}
static void init_first_node(Board b,Piece turn){
    /*TODO*/
}
void MCTS_iterate(HashIndex hi_root,HashIndex*history){
    uint32_t history_count=0;
    SelectResult sr=select(hi_root,history,&history_count);
    if(not_all_visited(sr)){
        Board b;
        Decompress(b,MCTSTreeGetKey(sr.hi));
        Piece winner=simulate(b,sr.p);
        back_propagate(history,winner,history_count);
    }else{
        back_propagate(history,sr.p,history_count);
    }
}
void MCTS_one_tree(HashIndex hi_root){
    HashIndex history[64];
    do{
        MCTS_iterate(hi_root,history);
    }while(MCTSTreeGetCount()<100000);
}
void MCTS(){
    
}
#undef set_point