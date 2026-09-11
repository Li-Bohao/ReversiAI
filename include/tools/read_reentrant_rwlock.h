#ifndef TOOLS_READ_REENTRANT_RWLOCK_H
#define TOOLS_READ_REENTRANT_RWLOCK_H
#include<stdatomic.h>
#include<sys/types.h>
#include<stdint.h>
#include<immintrin.h>
typedef struct{
    uint32_t max_thread;
    atomic_int protect;
    uint32_t reader;
    _Bool writing;
    uint32_t writer_wait;
    atomic_int protect_readers;
    pid_t *readers;
    uint32_t *counts;
}read_reentrant_rwlock;
void Init(read_reentrant_rwlock*lock,uint32_t max_thread);
void ReadLock(read_reentrant_rwlock*lock);
void WriteLock(read_reentrant_rwlock*lock);
void Unlock(read_reentrant_rwlock*lock);
void Destroy(read_reentrant_rwlock*lock);
#endif