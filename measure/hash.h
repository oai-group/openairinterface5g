#ifndef _HASH_H
#define _HASH_H

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long uint64_t;

#define MAX_PRIME32 1229
#define MAX_BIG_PRIME32 50

extern uint32_t big_prime32[MAX_BIG_PRIME32];

extern uint32_t prime32[MAX_PRIME32];

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

uint32_t Bob_hash32(uint32_t prime32_num, const char* str, uint32_t len);


#endif

