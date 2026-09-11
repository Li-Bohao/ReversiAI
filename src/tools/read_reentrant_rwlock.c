#include<stdatomic.h>
#include<sys/types.h>
#include<stdint.h>
#include<immintrin.h>
#include<stdlib.h>
#include<sys/syscall.h>
#include<unistd.h>
#include"tools/read_reentrant_rwlock.h"
constexpr int FREE=0,PROTECTED=1;
/*TODO：深入分析protect的内存序！*/
void Init(read_reentrant_rwlock*lock,uint32_t max_thread){
    lock->max_thread=max_thread;
    atomic_init(&lock->protect,0);
    lock->reader=0;
    lock->writing=false;
    lock->writer_wait=0;
    atomic_init(&lock->protect_readers,0);
    lock->readers=(pid_t*)malloc(max_thread*sizeof(pid_t));
    lock->counts=(uint32_t*)malloc(max_thread*sizeof(uint32_t));
}
void ReadLock(read_reentrant_rwlock*lock){
    pid_t tid = syscall(SYS_gettid);
    int wait=FREE;
    while(!atomic_compare_exchange_weak(lock->protect_readers,&wait,PROTECTED)){
        __mm_pause();
    }
    for(int i=0;i<lock->reader;++i){
        if(lock->readers[i]==tid){
            ++lock->counts[i];
            atomic_store(lock->protect_readers,FREE);
            return;
        }
    }
    atomic_store(lock->protect_readers,FREE);
    wait=FREE;
    while(1){
        while(!atomic_compare_exchange_weak(lock->protect,&wait,PROTECTED)){
            __mm_pause();
        }
        if(!lock->writing&&0==lock->writer_wait){
            lock->readers[lock->reader]=tid;
            lock->counts[lock->reader]=1;
            ++lock->reader;
            atomic_store(lock->protect,FREE);
            return;
        }
        atomic_store(lock->protect,FREE);
    }
}
void WriteLock(read_reentrant_rwlock*lock){
    int wait=FREE;
    _Bool added_writer_wait=false;
    while(1){
        while(!atomic_compare_exchange_weak(lock->protect,&wait,PROTECTED)){
            __mm_pause();
        }
        if(!lock->writing&&0==lock->reader){
            lock->writing=true;
            if(added_writer_wait){
                --lock->writer_wait;
            }
            atomic_store(lock->protect,FREE);
            return;
        }else if(!added_writer_wait){
            ++lock->writer_wait;
            added_writer_wait=true;
        }
        atomic_store(lock->protect,FREE);
    }
}
void Unlock(read_reentrant_rwlock*lock){
    int wait=FREE;
    pid_t tid = syscall(SYS_gettid);
    while(!atomic_compare_exchange_weak(lock->protect,&wait,PROTECTED)){
        __mm_pause();
    }
    if(lock->writing){
        lock->writing=false;
    }else{
        int wait_readers=FREE;
        while(!atomic_compare_exchange_weak(lock->protect_readers,&wait_readers,PROTECTED)){
            __mm_pause();
        }
        for(int i=0;i<lock->reader;++i){
            if(lock->readers[i]==tid){
                --lock->counts[i];
                if(0==lock->counts[i]){
                    --lock->reader;
                    lock->readers[i]=lock->readers[lock->reader];
                    lock->counts[i]=lock->counts[lock->reader];
                }
                atomic_store(lock->protect_readers,FREE);
                break;
            }
        }
    }
    atomic_store(lock->protect,FREE);
}
void Destroy(read_reentrant_rwlock*lock){
    free(lock->readers);
    free(lock->counts);
}