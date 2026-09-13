#include<stdint.h>
#include<math.h>
#include<stdlib.h>
#include<float.h>
#include<pthread.h>
#include<inttypes.h>
#include"mcts/mcts.h"
#include"reversi/reversi.h"
#include"tools/hash_types.h"
typedef struct{
    uint32_t visit;
    uint32_t win2;
    Piece turn;
    uint8_t child_num;
    uint8_t unvisited_child;
    uint8_t virtual_visit;
    uint8_t virtual_win2;
    HashIndex child_index[MAX_MOVE];
    uint32_t child_visit[MAX_MOVE];
}MCTSNode;
/*当未访问过时，child_visit[i]存储位置信息，*/
/*31到16位为空，15到8位为x，7到0位为y。*/
#define set_point(p,data)do{\
    data=((uint32_t)p.x<<8)|p.y;\
}while(0)
static inline Point get_point(uint32_t data){
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
#define HMHash(value) (hash(value))
#include"tools/hash.h"

/*选择*/

static inline float child_win_rate(const MCTSNode*n){
    if(n->win2<n->virtual_win2){
        return 0;
    }
    return (float)(n->win2-n->virtual_win2)/2/(n->visit+n->virtual_visit);
}
static inline float child_find_rate(const MCTSNode*n,uint32_t index){
    return sqrt(log((float)n->visit)/n->child_visit[index]);
}
static uint32_t choose_by_uct(MCTSNode* n){
    const float C=sqrt(2);
    float uct_val=FLT_MIN,cwr,cfr,new_uct_val;
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
static inline SelectResult make_all_visited(HashIndex hi){
    SelectResult sr;
    sr.p=INVALID_PIECE;
    sr.hi=hi;
    return sr;
}
static SelectResult do_select_once(MCTSNode*n,CompressedBoard cb){
    set_virtual_loss(n);
    ++n->visit;
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
        n->child_index[i]=MCTSTreeFindOrInsert(child_cb);
        return make_not_all_visited(n->child_index[i],GetReversedPiece(n->turn));
    }else{
        uint32_t i=choose_by_uct(n);
        ++n->child_visit[i];
        return make_all_visited(n->child_index[i]);
    }
}
static inline SelectResult select_once(HashIndex hi){
    SelectResult sr;
    HMWrite(MCTSTreeHashMap,sr,do_select_once,hi,MCTSTreeGetKey(hi));
    return sr;
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

/*扩展*/

static void do_expanse(MCTSNode* n,Piece turn,CompressedBoard cb){
    n->visit=1;
    n->win2=0;
    n->virtual_visit=0;
    n->virtual_win2=0;
    Board b;
    Decompress(b,cb);
    Moves m;
    GetMoves(b,turn,m);
    if(0==m.count){
        /*无法落子时由对方尝试*/
        ReversePiece(turn);
        GetMoves(b,turn,m);
    }
    n->turn=turn;
    n->child_num=m.count;
    n->unvisited_child=m.count;
    for(int i=0;i<m.count;++i){
        set_point(m.p[i],n->child_visit[i]);
    }
}
static void expanse(HashIndex hi,Piece turn){
    HMWriteNoRet(MCTSTreeHashMap,is_end,do_expanse,hi,turn,MCTSTreeGetKey(hi));
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
static Piece simulate(Board b,Piece turn){
    _Bool success;
    do{
        success=simulate_once(b,turn);
        if(false==success){
            ReversePiece(turn);
            success=simulate_once(b,turn);
            if(!success){
                return Judge(b);
            }
        }
        ReversePiece(turn);
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

static HashIndex init_first_node(Board b_root,Piece turn_root){
    CompressedBoard cb_root=Compress(b_root);
    HashIndex hi_root=MCTSTreeFindOrInsert(cb_root);
    expanse(hi_root,turn_root);
    return hi_root;
}
static void MCTS_iterate(HashIndex hi_root,HashIndex*history){
    uint32_t history_count=0;
    SelectResult sr=select(hi_root,history,&history_count);
    if(not_all_visited(sr)){
        expanse(sr.hi,sr.p);
        Board b;
        Decompress(b,MCTSTreeGetKey(sr.hi));
        Piece winner=simulate(b,sr.p);
        back_propagate(history,winner,history_count);
    }else{
        back_propagate(history,sr.p,history_count);
    }
}
typedef struct{
    HashIndex hi;
    uint32_t max;
}MCTS_args;
static inline uint32_t do_get_root_visit(const MCTSNode*n){
    return n->visit;
}
static inline uint32_t get_root_visit(HashIndex hi_root){
    uint32_t ret;
    HMRead(MCTSTreeHashMap,ret,do_get_root_visit,hi);
    return ret;
}
static void MCTS_one_tree(MCTS_args*args){
    HashIndex hi_root=args->hi;
    uint32_t max=args->max;
    HashIndex history[64];
    do{
        MCTS_iterate(hi_root,history);
    }while(get_root_visit(hi_root)<max);
}
typedef struct{
    CompressedBoard cb;
    uint32_t child_visit[MAX_MOVE];
    uint8_t child_num;
}Sample;
static inline Sample do_get_sample(const MCTSNode*n){
    Sample sp;
    memcpy(sp.child_visit,n->child_visit,sizeof(uint32_t[MAX_MOVE]));
    sp.child_num=n->child_num;
    return sp;
}
static inline Sample get_sample(HashIndex hi){
    Sample sp;
    HMRead(MCTSTreeHashMap,sp,do_get_sample,hi);
    sp.cb=MCTSTreeGetKey(hi);
    return sp;
}
static void save_sample(const char*name,Sample*sp){
    /*TODO：创建samples目录！*/
    char filepath[200]="./samples";
    strcat(filepath,name);
    FILE *fp=fopen(filepath,"wb");
    if(!fp){
        perror("save sample open file failed!");
        return;
    }
    if(1!=fwrite(sp,sizeof(*sp),1,fp)){
        perror("save sample write file failed!");
        return;
    }
    fclose(fp);
}
static void MCTS(uint32_t hash_size,uint32_t max,uint32_t thread_count,uint32_t sample_count){
    uint32_t map_size=max+64*thread_count;
    MCTSTreeInit(hash_size,map_size);
    Board b_root;
    Piece p_root=BLACK;
    MCTS_args m_args;
    GetInitialBoard(b_root);
    HashIndex hi_root;
    save_sample("test");
    char name[100];
    uint32_t sample_order=0;
    Sample sp;
    do{
        hi_root=init_first_node(b_root,p_root);
        m_args.hi=hi_root;
        m_args.max=max;
        pthread_t *MCTS_threads=(pthread_t*)malloc(thread_count*sizeof(pthread_t));
        for(int i=0;i<thread_count;++i){
            if(0!=pthread_create(&MCTS_threads[i],NULL,MCTS_one_tree,&m_args)){
                perror("MCTS thread create failed!");
            }
        }
        for(int i=0;i<thread_count;++i){
            pthread_join(MCTS_threads[i],NULL);
        }
        sp=get_sample(hi_root);
        int n=snprintf(name,100,"%" PRIu32,sample_order);
        if(n<0){
            perror("sample_order to string failed!");
        }else{
            save_sample(name,&sp);
        }
        ++sample_order;
    }while(sample_order<sample_count);
}
#undef set_point