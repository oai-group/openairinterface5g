#ifndef _ELASTICSKETCH_H_
#define _ELASTICSKETCH_H_

#include "HeavyPart.h"
#include "LightPart.h"
#include <time.h>
#include <sys/socket.h>

typedef struct ElasticSketch
{
    int bucket_num;
    int light_part_counter_num;
    HeavyPart heavy_part;
    LightPart light_part;

    void (*Constructor)(struct ElasticSketch *, int, int);
    void (*Destructor)(struct ElasticSketch *);
    void (*Clear)(struct ElasticSketch *);

    void (*Insert)(struct ElasticSketch *, KEY_TYPE * , PACKET_INFO *);
    VAL_TYPE (*Query)(struct ElasticSketch *, KEY_TYPE *);
}ElasticSketch;

//构造函数
void _constructor_ElasticSketch(struct ElasticSketch * elastic_sketch_p, int bucket_num, int light_part_counter_num);

//析构函数
void _destructor_ElasticSketch(struct ElasticSketch * elastic_sketch_p);

//清除内存
void _clear_ElasticSketch(struct ElasticSketch * elastic_sketch_p);

//插入
void _insert_ElasticSketch(struct ElasticSketch * elastic_sketch_p, KEY_TYPE * key_p, PACKET_INFO * packet_p);

//查询
VAL_TYPE _query_ElasticSketch(struct ElasticSketch * elastic_sketch_p, KEY_TYPE * key_p);

//初始化
//注意步骤：
//1.先创建ElasticSketch实例
//2.调用Init_ElasticSketch()方法初始化
//3.调用完成后徐亚显式调用析构函数释放内存
void Init_ElasticSketch(struct ElasticSketch * elastic_sketch_p, int bucket_num, int tot_mem_in_bytes);
int cmp_int(const void *a, const void *b);
typedef struct TransData
{
    int minLenth;
    int maxLenth;
    float averLenth;
    float varLenth;
    int  MedianLenth;

    long minInterval;
    long maxInterval;
    double  averInterval;
    double varInterval;
    long  MedianInterval;
    uint8_t data[13];
} TransData;
 


#endif