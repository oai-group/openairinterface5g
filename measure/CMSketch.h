#ifndef _CMSKETCH_H
#define _CMSKETCH_H

#include "hash.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CMSKETCH_D 3
#define CMSKETCH_MEMORY_IN_BYTES (600*1024)
#define CMSKETCH_KEY_LEN 13

//CMSketch直接写死
 typedef struct CMSketch
 {
    int d;
    int w;
    int memory_in_byte;
    //counters应该对应一个d*w的二维数组，目前counters指向的每一个值都是一个w维数组的指针
    uint32_t** counters;
    uint32_t* hash_prime_nums;
 }CMSketch;
 
//初始化CMSketch，主要是设置参数，开辟空间，初始化
void init_CMSketch(CMSketch* sketch, int d, int memory_in_byte);
//由于调用了malloc申请空间，结束后需要释放空间
void free_CMSketch(CMSketch* sketch);
//插入sketch
void insert_to_CMSketch(CMSketch* sketch, uint8_t* key, int cnt);
//从sketch中查询
int query_from_CMSketch(CMSketch* sketch, uint8_t* key);
void reset_CMSketch(CMSketch* sketch);

int test_hash(CMSketch* sketch, uint8_t* key, int i);

#endif