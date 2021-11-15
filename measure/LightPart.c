
#include "LightPart.h"

// #define MEM_PER_COUNTER_IN_LIGHTPART (1) 


void _constructor_LightPart(struct LightPart * light_part_p, int counter_num){
    light_part_p->counter_num = counter_num;

    Init_BobHash32(&(light_part_p->bobhash), BOBHASH_SEED);

    light_part_p->counters = (VAL_TYPE*) malloc(sizeof(VAL_TYPE) * light_part_p->counter_num);
    memset(light_part_p->counters, 0, sizeof(VAL_TYPE) * light_part_p->counter_num);
}

void _destructor_LightPart(struct LightPart * light_part_p){
    free(light_part_p->counters);
    light_part_p->counters = NULL;
    light_part_p->counter_num = 0;
    light_part_p->bobhash.Destructor(&light_part_p->bobhash);
}

void _clear_LightPart(struct LightPart * light_part_p){
    memset(light_part_p->counters, 0, sizeof(VAL_TYPE) * light_part_p->counter_num);
}

//插入
void _insert_LightPart(struct LightPart * light_part_p, KEY_TYPE * key_p, PACKET_INFO * packet_p){
    uint32_t hash_val = (uint32_t)light_part_p->bobhash.Run(&(light_part_p->bobhash), (const char*)(key_p->flow_id), KEY_LENGTH);
    //计算存储到的索引位置
    uint32_t pos = hash_val % (uint32_t)light_part_p->counter_num;

    if (light_part_p->counters[pos].packet_num < TOP_10_PACKET){
        // light_part_p->counters[pos].top_10_packets[light_part_p->counters[pos].packet_num] = *packet_p;
        memcpy(&light_part_p->counters[pos].top_10_packets[light_part_p->counters[pos].packet_num],packet_p,sizeof(PACKET_INFO));
    }
    light_part_p->counters[pos].packet_num++;
    light_part_p->counters[pos].tot_size += packet_p->size;
}

void _insert_flow_to_LightPart(struct LightPart * light_part_p, KEY_TYPE * key_p, VAL_TYPE * val_p){
    uint32_t hash_val = (uint32_t)light_part_p->bobhash.Run(&(light_part_p->bobhash), (const char*)(key_p->flow_id), KEY_LENGTH);
    //计算存储到的索引位置
    uint32_t pos = hash_val % (uint32_t)light_part_p->counter_num;

    if (light_part_p->counters[pos].packet_num < TOP_10_PACKET){
        for (int i = light_part_p->counters[pos].packet_num, j= 0; i < TOP_10_PACKET && j < val_p->packet_num; i++,j++){
          light_part_p->counters[pos].top_10_packets[i] = val_p->top_10_packets[j];
        }
    }
    light_part_p->counters[pos].packet_num += val_p->packet_num;
    light_part_p->counters[pos].tot_size += val_p->tot_size;
}

//
void _swap_insert_LightPart(struct LightPart * light_part_p, KEY_TYPE * key_p, VAL_TYPE * val_p){
    uint32_t hash_val = (uint32_t)light_part_p->bobhash.Run(&(light_part_p->bobhash), (const char*)(key_p->flow_id), KEY_LENGTH);
    uint32_t pos = hash_val % (uint32_t)light_part_p->counter_num;

    if (light_part_p->counters[pos].tot_size < val_p->tot_size) {
    //   light_part_p->counters[pos] = *val_p;
      memcpy(&light_part_p->counters[pos],val_p,sizeof(PACKET_INFO));
    }
}

//查询
VAL_TYPE _query_LightPart(struct LightPart * light_part_p, KEY_TYPE * key_p){
    uint32_t hash_val = (uint32_t)light_part_p->bobhash.Run(&(light_part_p->bobhash), (const char*)(key_p->flow_id),4);
    uint32_t pos = hash_val % (uint32_t)light_part_p->counter_num;


    return light_part_p->counters[pos];
}

void Init_LightPart(struct LightPart * light_part_p, int init_mem_in_bytes){
    light_part_p->Constructor = _constructor_LightPart;
    light_part_p->Destrcutor = _destructor_LightPart;
    light_part_p->Clear = _clear_LightPart;
    light_part_p->Insert = _insert_LightPart;
    light_part_p->InsertFlow = _insert_flow_to_LightPart;
    light_part_p->SwapInsert = _swap_insert_LightPart;
    light_part_p->Query = _query_LightPart;

    light_part_p->Constructor(light_part_p,init_mem_in_bytes);
}

