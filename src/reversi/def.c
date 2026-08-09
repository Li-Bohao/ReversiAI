#define REVERSI_COMPILATION
#include"def.h"
#include <stdint.h>
/*TODO：这儿其实可以用SIMD加速*/
CompressedBoard Compress(Board b){
    CompressedBoard cb={0,0};
    for(int i=0;i<32;++i){
        /*TODO：cache会不会被破坏？*/
        /*TODO：直接用最低维度访问所有的元素，太丑陋了，太丑陋了*/
        cb.high|=(((uint64_t)b[0][i])<<(2*i));
        cb.low|=(((uint64_t)b[0][i+32])<<(2*i));
    }
    return cb;
}
void Decompress(Board b,CompressedBoard cb){
    for(int i=0;i<32;++i){
        /*TODO：cache！*/
        /*TODO：又是直接用最低维度访问所有的元素！*/
        b[0][i]=(cb.high>>(2*i))&0b11;
        b[0][i+32]=(cb.low>>(2*i))&0b11;
    }
}