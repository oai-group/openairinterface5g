#ifndef _HEAVYPART_H_
#define _HEAVYPART_H_

#include "params.h"


typedef struct Bucket {
  KEY_TYPE key[COUNTER_PER_BUCKET];
  VAL_TYPE val[COUNTER_PER_BUCKET];
}Bucket_t;


typedef struct HeavyPart
{
    int bucket_num;
    // alignas(64) Bucket_t* buckets;  
    Bucket_t* buckets;
    // alignas(64) Bucket_t* buckets;  

    void (*Constructor)(struct HeavyPart *, int );    //构造函数
    void (*Destructor)(struct HeavyPart *);     //析构函数
    void (*Clear)(struct HeavyPart *);          //清除内存
    
    int (*CalculateFP)(struct HeavyPart *, uint32_t *, uint32_t *); 

    int (*Insert)(struct HeavyPart * , KEY_TYPE * , KEY_TYPE * , VAL_TYPE * , PACKET_INFO *);   //插入
    VAL_TYPE (*Query)(struct HeavyPart *, KEY_TYPE *);         //查询
}HeavyPart;


void _constructor_HeavyPart(struct HeavyPart * heavy_part_p, int bucket_num);

void _destructor_HeavyPart(struct HeavyPart * heavy_part_p);

void _clear_HeavyPart(struct HeavyPart * heavy_part_p);

int _calculateFP_HeavyPart(struct HeavyPart * heavy_part_p, uint32_t * key_p, uint32_t* fp);

int _insert_HeavyPart_noSIMD(struct HeavyPart * heavy_part_p, KEY_TYPE * key_p, KEY_TYPE * swap_key_p, VAL_TYPE * swap_val_p, PACKET_INFO * packet_p);

VAL_TYPE _query_HeavyPart(struct HeavyPart * heavy_part_p, KEY_TYPE * key_p);


void Init_HeavyPart(struct HeavyPart * heavy_part_p, int bucket_num);


#endif