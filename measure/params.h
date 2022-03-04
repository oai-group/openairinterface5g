#ifndef _PARAMS_H_
#define _PARAMS_H_

#include "stdint.h"
#include "string.h"
#include "stdlib.h"
#include "stdio.h"
#include "x86intrin.h"
#include "time.h"


#define COUNTER_PER_BUCKET (8)
#define MAX_VALID_COUNTER (COUNTER_PER_BUCKET - 1)

// #define MEM_IN_BYTES_PER_COUNTER (8)


#define CONSTANT_NUMBER 2654435761u
#define CalculateBucketPos(fp) (((fp) * CONSTANT_NUMBER) >> 15)
// #define GetCounterVal(val) ((uint32_t)((val) & 0x7FFFFFFF))

//逻辑上应该是guard_val存储的是该bucket中总的包的数量, 即总数量大于最小值的8倍
#define JUDGE_IF_SWAP(min_val, guard_val) ((guard_val) > ((min_val) << 3))


//##########################################################################

// #define HEAVY_MEM (150 * 1024)
#define BUCKET_NUM (150 * 1024 / 64)
#define LIGHT_PART_COUNTER_NUM (450 * 1024)
// #define HEAVY_MEM (BUCKET_NUM * COUNTER_PER_BUCKET * MEM_IN_BYTES_PER_COUNTER)  
// #define LIGHT_MEM (LIGHT_PART_COUNTER_NUM * sizeof(VAL_TYPE))

//key的长度，4表示只区分源ip，13表示区分五元组
// #define KEY_LENGTH (4)
#define KEY_LENGTH (13)

//前10个数据包
#define TOP_10_PACKET (10)

//hash函数用到的种子
#define BOBHASH_SEED (750)
//set超时时间
#define TIMEOUTSLOT (5)
// 偏移
#define SHIFT 8
// PLR测量染色间隔
#define COLORSLOT 5

//到达时间类型，可以自己更改
//但是如果不是结果体，需要更改算法中的赋值操作(=)为内存拷贝(memcpy)
// typedef struct tm arrived_time_t;
typedef struct timespec arrived_time_t;

//五元组,存储流ID
typedef struct FIVE_TUPLE {
  uint8_t flow_id[KEY_LENGTH];
}FIVE_TUPLE;

//每个packet需要保留的信息，
//size：packet大小
//arrived_time: 到达时间
typedef struct PACKET_INFO{
  uint32_t size;
  arrived_time_t arrived_time;
}PACKET_INFO;

//FLOW_INFO
typedef struct FLOW_INFO{
    uint32_t tot_size;   //流的总的统计大小
    uint32_t packet_num;  //流的总的包数量
    // uint32_t last_cnt;
    // uint32_t now_cnt;
    PACKET_INFO top_10_packets[TOP_10_PACKET];  //前10个packet的信息,顺序同索引，即索引0是第1个包，依次类推
    //标志位，用于heavy_part中。为true的流的实际大小计算是heavy_part中的大小+light_part中的大小。
    // 前10个packet：先在light_part中取前10个，不够的由heavy_part补齐。
    int is_evicted;  
}FLOW_INFO;

typedef FIVE_TUPLE KEY_TYPE;
typedef FLOW_INFO VAL_TYPE;



#endif