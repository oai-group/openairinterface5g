
#include "HeavyPart.h"



void _constructor_HeavyPart(struct HeavyPart * heavy_part_p, int bucket_num){
    heavy_part_p->bucket_num = bucket_num;
    heavy_part_p->buckets = (Bucket_t*) malloc(bucket_num * sizeof(Bucket_t));
    memset(heavy_part_p->buckets, 0, sizeof(Bucket_t) * bucket_num);
}

void _destructor_HeavyPart(struct HeavyPart * heavy_part_p){
    free(heavy_part_p->buckets);
    heavy_part_p->buckets = NULL;
    heavy_part_p->bucket_num = 0;
}

void _clear_HeavyPart(struct HeavyPart * heavy_part_p){
    memset(heavy_part_p->buckets, 0, sizeof(Bucket_t) * heavy_part_p->bucket_num);
}

int _calculateFP_HeavyPart(struct HeavyPart * heavy_part_p, uint32_t * key_p, uint32_t* fp){
    *fp = *((uint32_t*)key_p);
    return CalculateBucketPos(*fp) % heavy_part_p->bucket_num;
}

int _insert_HeavyPart_noSIMD(struct HeavyPart * heavy_part_p, KEY_TYPE * key_p, KEY_TYPE * swap_key_p, VAL_TYPE * swap_val_p, PACKET_INFO * packet_p){
    uint32_t fp;
    //fp是取key的前4Byte，pos是计算出的索引
    int pos = heavy_part_p->CalculateFP(heavy_part_p, (uint32_t *)(&key_p->flow_id[SHIFT]), &fp);

    int matched = -1, empty = -1, min_counter = 0;

    //空白流ID
    char empty_id[KEY_LENGTH];
    memset(&empty_id,0,KEY_LENGTH);

    //初始化bucket中最小流的大小为第一个counter的值
    uint32_t min_counter_val = heavy_part_p->buckets[pos].val[0].tot_size;

    // for all counters in a bucket
    for(int i = 0; i < COUNTER_PER_BUCKET - 1; i++) {
      //比较五元组,如果相同
      if(!memcmp(heavy_part_p->buckets[pos].key[i].flow_id, key_p->flow_id, KEY_LENGTH)){
        matched = i;
        // printf("matched: %d\n", matched);
        break;
      }

      //找第一个空的counter
      // if(heavy_part_p->buckets[pos].key[i].flow_id == 0 && empty == -1) {
      if(!memcmp(heavy_part_p->buckets[pos].key[i].flow_id, &empty_id, KEY_LENGTH) && empty == -1) {
        empty = i;
      }

      //检查是否需要更改最小流的大小
      if(min_counter_val > heavy_part_p->buckets[pos].val[i].tot_size) {
        min_counter = i;
        min_counter_val = heavy_part_p->buckets[pos].val[i].tot_size;
      }
    }

    //如果找到匹配的counter, 则进行更新
    if(matched != -1) {    
      //前10个packet记录大小和到达时间
      if (heavy_part_p->buckets[pos].val[matched].packet_num < TOP_10_PACKET){
        // heavy_part_p->buckets[pos].val[matched].top_10_packets[heavy_part_p->buckets[pos].val[matched].packet_num] = *packet_p;
        memcpy(&heavy_part_p->buckets[pos].val[matched].top_10_packets[heavy_part_p->buckets[pos].val[matched].packet_num],packet_p,sizeof(PACKET_INFO));
      }
      // printf("hp_insert_matched: packet_num: %d, tot_size: %d\n",heavy_part_p->buckets[pos].val[matched].packet_num, heavy_part_p->buckets[pos].val[matched].tot_size);
      heavy_part_p->buckets[pos].val[matched].tot_size += packet_p->size;
      heavy_part_p->buckets[pos].val[matched].packet_num++;
      // if (heavy_part_p->buckets[pos].val[matched].packet_num)

      return 0;
    }

    //如果没有匹配到但是存在一个空的counter
    if(empty != -1) {
      // heavy_part_p->buckets[pos].key[empty] = *key_p;
      // heavy_part_p->buckets[pos].val[empty].top_10_packets[0] = *packet_p;
      memcpy(&heavy_part_p->buckets[pos].key[empty],key_p,sizeof(KEY_TYPE));
      memcpy(&heavy_part_p->buckets[pos].val[empty].top_10_packets[0],packet_p,sizeof(PACKET_INFO));
      heavy_part_p->buckets[pos].val[empty].packet_num = 1;
      heavy_part_p->buckets[pos].val[empty].tot_size = packet_p->size;

      return 0;
    }

    // in MAX_VALID_COUNTER
    uint32_t guard_val = heavy_part_p->buckets[pos].val[MAX_VALID_COUNTER].tot_size;
    guard_val += packet_p->size; 

    // #define JUDGE_IF_SWAP(min_val, guard_val) ((guard_val) > ((min_val) << 3))
    //确定阈值,如果bucket中计数器最小值大于阈值guard的1/8,则不进行交换
    if(!JUDGE_IF_SWAP(min_counter_val, guard_val)) {
      // do not swap
      // printf("beafore: %d\n", heavy_part_p->buckets[pos].val[MAX_VALID_COUNTER].tot_size);
      heavy_part_p->buckets[pos].val[MAX_VALID_COUNTER].tot_size = guard_val;
      // printf("after: %d\n", heavy_part_p->buckets[pos].val[MAX_VALID_COUNTER].tot_size);
      return 2;
    }

    // 交换这里可能有问题
    *swap_key_p = heavy_part_p->buckets[pos].key[min_counter];
    *swap_val_p = heavy_part_p->buckets[pos].val[min_counter];


    VAL_TYPE empty_val;
    memset(&empty_val,0,sizeof(VAL_TYPE));

    heavy_part_p->buckets[pos].val[MAX_VALID_COUNTER] = empty_val;

    // heavy_part_p->buckets[pos].key[min_counter] = *key_p;
    memcpy(&heavy_part_p->buckets[pos].key[min_counter],key_p,sizeof(KEY_TYPE));

    heavy_part_p->buckets[pos].val[min_counter] = empty_val;
    // heavy_part_p->buckets[pos].val[min_counter].top_10_packets[0] = *packet_p;
    memcpy(&heavy_part_p->buckets[pos].val[empty].top_10_packets[0],packet_p,sizeof(PACKET_INFO));
    heavy_part_p->buckets[pos].val[min_counter].packet_num = 1;
    heavy_part_p->buckets[pos].val[min_counter].tot_size = packet_p->size;
    heavy_part_p->buckets[pos].val[min_counter].is_evicted = 1;

    return 1;

}

VAL_TYPE _query_HeavyPart(struct HeavyPart * heavy_part_p, KEY_TYPE * key_p){
    uint32_t fp;
    int pos = heavy_part_p->CalculateFP(heavy_part_p, (uint32_t *)(&key_p->flow_id[SHIFT]), &fp);

    for(int i = 0; i < MAX_VALID_COUNTER; ++i){
      if(!memcmp(heavy_part_p->buckets[pos].key[i].flow_id, key_p->flow_id, KEY_LENGTH)){
        return heavy_part_p->buckets[pos].val[i];
      }
    }

    // 如果没有找到，需要返回空结构体
    VAL_TYPE empty_val;
    memset(&empty_val, 0, sizeof(VAL_TYPE));
    return empty_val;
}


void Init_HeavyPart(struct HeavyPart * heavy_part_p, int bucket_num){
    heavy_part_p->Constructor = _constructor_HeavyPart;
    heavy_part_p->Destructor = _destructor_HeavyPart;
    heavy_part_p->Clear = _clear_HeavyPart;
    heavy_part_p->CalculateFP = _calculateFP_HeavyPart;
    // heavy_part_p->Insert = _insert_HeavyPart;
    heavy_part_p->Insert = _insert_HeavyPart_noSIMD;
    heavy_part_p->Query = _query_HeavyPart;

    heavy_part_p->Constructor(heavy_part_p, bucket_num);
}
