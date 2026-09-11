/*因为没有删除的需求，所以逻辑简化了不少*/
#include<stdlib.h>
#include<stdint.h>
#include<pthread.h>
#include<string.h>
#include"tools/hash_types.h"
/*宏实现的泛型哈希表也许无法单独拆分实现到.c文件了，不然被include的时候不能加载以下宏。*/
#ifndef HMKey/*键类型*/
#error "HMKey undefined!"
#endif
#ifndef HMValue/*值类型*/
#error "HMValue undefined!"
#endif
#ifndef HMName/*表名*/
#error "HMName undefined!"
#endif
#ifndef HM_SIZE/*容量*/
#error "HM_SIZE undefined!"
#endif
#ifndef HMHash/*哈希函数*/
#error "HMHash undefined!"
#endif
#define hm_concat_(a,b) a##b
#define hm_concat(a,b) hm_concat_(a,b)
#define HM(Name) hm_concat(HMName,Name)
#define set_no_next(hn) do{hn.next=INVALID_HASH_INDEX;}while(0)
typedef struct{
    HMKey key;
    HMValue value;
    HashIndex next;
    uint32_t hash_key;
}HM(Node);
static HashIndex* HM(Map);
static HM(Node)* HM(Pool);
#ifdef MT_HASHMAP
constexpr static HM(LockLg)=4;
static pthread_rwlock_t HM(InsertLocks)[1<<HM(LockLg)];
static pthread_rwlock_t HM(ValueLocks)[1<<HM(LockLg)];
static inline uint32_t HM(hash_key_to_lock_index)(uint32_t hk){
    /*TODO：注意看这种方法有没有性能问题*/
    return hk&((1<<HM(LockLg))-1);
}
static inline void HM(InitLocks)(){
    for(int i=0;i<(1<<HM(LockLg));++i){
        pthread_rwlock_init(&HM(InsertLocks)[i]);
        pthread_rwlock_init(&HM(ValueLocks)[i]);
    }
}
static inline void HM(FreeLocks)(){
    for(int i=0;i<(1<<HM(LockLg));++i){
        pthread_rwlock_destroy(&HM(InsertLocks)[i]);
        pthread_rwlock_destroy(&HM(ValueLocks)[i]);
    }
}
static inline void HM(read_lock)(uint32_t hk){
    pthread_rwlock_rdlock(&HM(ValueLocks)[HM(hash_key_to_lock_index)(hk)]);
}
static inline void HM(write_lock)(uint32_t hk){
    pthread_rwlock_wrlock(&HM(ValueLocks)[HM(hash_key_to_lock_index)(hk)]);
}
static inline void HM(read_lock_insert)(uint32_t hk){
    pthread_rwlock_rdlock(&HM(InsertLocks)[HM(hash_key_to_lock_index)(hk)]);
}
static inline void HM(write_lock_insert)(uint32_t hk){
    pthread_rwlock_wrlock(&HM(InsertLocks)[HM(hash_key_to_lock_index)(hk)]);
}
static inline void HM(unlock)(uint32_t hk){
    pthread_rwlock_unlock(&HM(ValueLocks)[HM(hash_key_to_lock_index)(hk)]);
}
static inline void HM(unlock_insert)(uint32_t hk){
    pthread_rwlock_unlock(&HM(InsertLocks)[HM(hash_key_to_lock_index)(hk)]);
}
static _Atomic uint32_t HM(Count);
#else
static uint32_t HM(Count);
#endif
#define HM_OVER_SIZE (1000)
static void HM(Init)() {
    HM(Map)=(HashIndex*)malloc(HM_SIZE*sizeof(HashIndex));
    /*TODO：余量设为多少合适？现在是1000*/
    HM(Pool)=(HM(Node)*)malloc((HM_SIZE+HM_OVER_SIZE)*sizeof(HM(Node)));
    for(uint32_t i=0;i<HM_SIZE;++i){
        HM(Map)[i]=INVALID_HASH_INDEX;
    }
    for(uint32_t i=0;i<HM_SIZE+HM_OVER_SIZE;++i){
        set_no_next(HM(Pool)[i]);
    }
#ifdef MT_HASHMAP
    HM(InitLocks)();
    atomic_init(&HM(Count),0);
#else
    HM(Count)=0;
#endif
}
static HashIndex HM(Find)(HMKey k) {
    uint32_t hash_key=HMHash(k)%HM_SIZE;
#ifdef MT_HASHMAP
    HM(read_lock_insert)(hash_key);
#endif
    HashIndex hi=HM(Map)[hash_key];
    while(IsValidHashIndex(hi)){
        if(0==memcmp(&HM(Pool)[hi].key,&k,sizeof(HMKey))){
#ifdef MT_HASHMAP
            HM(unlock_insert)(hash_key);
#endif
            return hi;
        }
        hi=HM(Pool)[hi].next;
    }
#ifdef MT_HASHMAP
    HM(unlock_insert)(hash_key);
#endif
    return hi;
}
static inline HashIndex HM(insert)(HashIndex prev,HMKey k,uint32_t hash_key,_Bool is_first){
    HashIndex hi;
#ifdef MT_HASHMAP
    /*多线程竞争写锁可能导致表被修改，需要重新检查真正的尾元素*/
    if(is_first){
        hi=HM(Map)[hash_key];
        if(IsValidHashIndex(hi)){
            if(0==memcmp(&HM(Pool)[hi].key,&k,sizeof(HMKey))){
                return hi;
            }
            is_first=false;
            prev=hi;
        }else{
            hi=atomic_fetch_add_explicit(&HM(Count),1,memory_order_relaxed);
            HM(Pool)[hi].key=k;
            HM(Pool)[hi].hash_key=hash_key;
            HM(Map)[hash_key]=hi;
            return hi;
        }
    }
    hi=HM(Pool)[prev].next;
    while(IsValidHashIndex(hi)){
        if(0==memcmp(&HM(Pool)[hi].key,&k,sizeof(HMKey))){
            return hi;
        }
        prev=hi;
        hi=HM(Pool)[hi].next;
    }
    hi=atomic_fetch_add_explicit(&HM(Count),1,memory_order_relaxed);
#else
    hi=HM(Count);
    ++HM(Count);
#endif
    HM(Pool)[hi].key=k;
    HM(Pool)[hi].hash_key=hash_key;
#ifdef MT_HASHMAP
    HM(Pool)[prev].next=hi;
#else
    if(is_first){
        HM(Map)[hash_key]=hi;
    }else{
        HM(Pool)[prev].next=hi;
    }
#endif
    return hi;
}
static HashIndex HM(FindOrInsert)(HMKey k) {
    uint32_t hash_key=HMHash(k)%HM_SIZE;
#ifdef MT_HASHMAP
    HM(read_lock_insert)(hash_key);
#endif
    HashIndex hi=HM(Map)[hash_key];
    if(!IsValidHashIndex(hi)){
#ifdef MT_HASHMAP
        HM(unlock_insert)(hash_key);
        HM(write_lock_insert)(hash_key);
#endif
        hi=HM(insert)(hash_key,k,hash_key,true);
#ifdef MT_HASHMAP
        HM(unlock_insert)(hash_key);
#endif
        return hi;
    }
    HashIndex prev;
    while(IsValidHashIndex(hi)){
        if(0==memcmp(&HM(Pool)[hi].key,&k,sizeof(HMKey))){
#ifdef MT_HASHMAP
            HM(unlock_insert)(hash_key);
#endif
            return hi;
        }
        prev=hi;
        hi=HM(Pool)[hi].next;
    }
#ifdef MT_HASHMAP
    HM(unlock_insert)(hash_key);
    HM(write_lock_insert)(hash_key);
#endif
    hi=HM(insert)(prev,k,hash_key,false);
#ifdef MT_HASHMAP
    HM(unlock_insert)(hash_key);
#endif
    return hi;
}
static void HM(Free)(){
    free(HM(Map));
    free(HM(Pool));
#ifdef MT_HASHMAP
    HM(FreeLocks)();
#endif
}
static inline uint32_t HM(GetCount)(){
#ifdef MT_HASHMAP
    return atomic_load_explicit(&HM(Count),memory_order_relaxed);
#else
    return HM(Count);
#endif
}
static inline void HM(Read)(HashIndex hi,HMValue *dest){
#ifdef MT_HASHMAP
    HM(read_lock)(HM(Pool)[hi].hash_key);
    /*幸运的是，next字段一经写入永远不会变化，并且只有存在的HashIndex才会被读取，所以可以直接读取，无需写锁*/
    /*TODO：注意next内存序问题*/
#endif
    memcpy(dest,&HM(Pool)[hi].value,sizeof(HMValue));
#ifdef MT_HASHMAP
    HM(unlock)(HM(Pool)[hi].hash_key);
#endif
}
static inline HMKey HM(GetKey)(HashIndex hi){
    /*幸运的是，key字段写完之后也不会变化*/
    return HM(Pool)[hi].key;
}
#ifndef HMAPIS
/*没办法，这些只能设为全局唯一了，设成宏能接受不同的表名还能在展开后可变参数，如果是函数，接收可变参数得乱七八糟*/
#define HMRead(map_name,ret,call_back,index,...) do{\
#ifdef MT_HASHMAP
    HM(read_lock)(HM(Pool)[index].hash_key);\
#endif
    ret=call_back((const HMValue*)&map_name[index].value __VA_OPT__(,) __VA_ARGS__);\
#ifdef MT_HASHMAP
    HM(unlock)(HM(Pool)[index].hash_key);\
#endif
}while(0)
#define HMWrite(map_name,ret,call_back,index,...) do{\
#ifdef MT_HASHMAP
    HM(write_lock)(HM(Pool)[index].hash_key);\
#endif
    ret=call_back(&map_name[index].value __VA_OPT__(,) __VA_ARGS__);\
#ifdef MT_HASHMAP
    HM(unlock)(HM(Pool)[index].hash_key);\
#endif
}while(0)
#define HMWriteNoRet(map_name,call_back,index,...) do{\
#ifdef MT_HASHMAP
    HM(write_lock)(HM(Pool)[index].hash_key);\
#endif
    call_back(&map_name[index].value __VA_OPT__(,) __VA_ARGS__);\
#ifdef MT_HASHMAP
    HM(unlock)(HM(Pool)[index].hash_key);\
#endif
}while(0)
#endif
#undef set_no_next
#undef HM
/*取消HMKey、HMValue、HMName、HM_SIZE、HMHash、HM_comp，避免被多次包含时出现重定义情况*/
#undef HMKey
#undef HMValue
#undef HMName
#undef HM_SIZE
#undef HMHash
#ifdef MT_HASHMAP
#undef MT_HASHMAP
#endif