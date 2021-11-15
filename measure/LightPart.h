#ifndef _LIGHT_PART_H_
#define _LIGHT_PART_H_

#include "params.h"
#include "BoBHash32.h"

// #define MEM_PER_COUNTER_IN_LIGHTPART (1) 


typedef struct LightPart
{
    int counter_num;
    BoBHash32 bobhash;
    VAL_TYPE* counters;

    void (*Constructor)(struct LightPart *, int);
    void (*Destrcutor)(struct LightPart *);
    void (*Clear)(struct LightPart *);

    void (*Insert)(struct LightPart * , KEY_TYPE * , PACKET_INFO *);
    void (*InsertFlow)(struct LightPart * , KEY_TYPE * , VAL_TYPE *);
    void (*SwapInsert)(struct LightPart *, KEY_TYPE *, VAL_TYPE *);
    VAL_TYPE (*Query)(struct LightPart *, KEY_TYPE *);
}LightPart;


void _constructor_LightPart(struct LightPart * light_part_p, int counter_num);

void _destructor_LightPart(struct LightPart * light_part_p);
void _clear_LightPart(struct LightPart * light_part_p);
//插入
void _insert_LightPart(struct LightPart * light_part_p, KEY_TYPE * key_p, PACKET_INFO * packet_p);

void _insert_flow_to_LightPart(struct LightPart * light_part_p, KEY_TYPE * key_p, VAL_TYPE * val_p);

//
void _swap_insert_LightPart(struct LightPart * light_part_p, KEY_TYPE * key_p, VAL_TYPE * val_p);

//查询
VAL_TYPE _query_LightPart(struct LightPart * light_part_p, KEY_TYPE * key_p);
void Init_LightPart(struct LightPart * light_part_p, int init_mem_in_bytes);


#endif