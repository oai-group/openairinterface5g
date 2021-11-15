#ifndef _BOBHASH32_H_
#define _BOBHASH32_H_

#include "stdint.h"

#define MAX_PRIME32 1229
#define MAX_BIG_PRIME32 50

#define mix(a,b,c) \
{ \
  a -= b; a -= c; a ^= (c>>13); \
  b -= c; b -= a; b ^= (a<<8); \
  c -= a; c -= b; c ^= (b>>13); \
  a -= b; a -= c; a ^= (c>>12);  \
  b -= c; b -= a; b ^= (a<<16); \
  c -= a; c -= b; c ^= (b>>5); \
  a -= b; a -= c; a ^= (c>>3);  \
  b -= c; b -= a; b ^= (a<<10); \
  c -= a; c -= b; c ^= (b>>15); \
}
extern uint32_t big_prime32[MAX_BIG_PRIME32];

extern uint32_t prime32[MAX_PRIME32];

typedef struct BoBHash32{
    uint32_t prime32Num;

    void (*Constructor)(struct BoBHash32 *, uint32_t);  //构造函数
    void (*Destructor)(struct BoBHash32 *);   //析构函数
    uint32_t (*Run)(struct BoBHash32 *, const char *, uint32_t);  //运行
}BoBHash32;



void _constructor(BoBHash32 * bob, uint32_t prime32Num);

void _destructor(BoBHash32 * bob);

// void _initialize(BoBHash32 * bob, uint32_t prime32Num){
//     bob->prime32Num = prime32Num;
// }

uint32_t _run(struct BoBHash32 * bob, const char * str, uint32_t len);

//初始化，运行时首先调用
void Init_BobHash32(BoBHash32 *bob, uint32_t prime32Num);


#endif