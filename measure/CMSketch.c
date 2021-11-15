#include "CMSketch.h"

//初始化CMSketch，主要是设置参数，开辟空间，初始化
void init_CMSketch(CMSketch* sketch, int d, int memory_in_byte){
    sketch->d = d;
    sketch->memory_in_byte = memory_in_byte;
    sketch->w = memory_in_byte / 4 / d;

    //申请空间
    sketch->counters = (u_int32_t **) malloc(sizeof(uint32_t *) * d);
    sketch->hash_prime_nums = (uint32_t*) malloc(sizeof(uint32_t) * d);

    //对于每个counter
    for (int i = 0; i < d; i++){
        //申请w大小的数组空间，将数组指针赋值,然后初始化数组内存
        sketch->counters[i] = (uint32_t *)malloc(sizeof(uint32_t) * sketch->w);
        memset(sketch->counters[i], 0 ,sizeof(uint32_t) * sketch->w);

        //初始化不同hash函数使用的hash_prime_num
        sketch->hash_prime_nums[i] = i + 750;
    }

    //TODO：打印log信息
}

//由于调用了malloc申请空间，结束后需要释放空间
void free_CMSketch(CMSketch* sketch){
    //从内到外依次释放
    for (int i = 0; i < sketch->d; i++){
        free(sketch->counters[i]);
    }

    free(sketch->counters);
    free(sketch->hash_prime_nums);
}

//插入sketch
void insert_to_CMSketch(CMSketch* sketch, uint8_t* key, int cnt){
    //将key进行d次hash运算，存储到counter的对应位置
    for (int i = 0; i < sketch->d; i++){
        int index = (Bob_hash32(sketch->hash_prime_nums[i],(const char *)key, CMSKETCH_KEY_LEN)) % sketch->w;
        sketch->counters[i][index] += cnt;
    }
}
void reset_CMSketch(CMSketch* sketch){
        for (int i = 0; i < sketch->d; i++){
        //申请w大小的数组空间，将数组指针赋值,然后初始化数组内存
        memset(sketch->counters[i], 0 ,sizeof(uint32_t) * sketch->w);
    }

}

//从sketch中查询
int query_from_CMSketch(CMSketch* sketch, uint8_t* key){
    //初始化一个非常大的数
    uint32_t ret = 1 << 30;

    //遍历d个counter，选出最小值
    for (int i = 0; i < sketch->d; i++){
        int index = (Bob_hash32(sketch->hash_prime_nums[i],(const char *)key, CMSKETCH_KEY_LEN)) % sketch->w;
        int temp = sketch->counters[i][index];
        ret = temp < ret ? temp : ret;
    }

    return ret;
}



int test_hash(CMSketch* sketch, uint8_t* key, int i){
    return (Bob_hash32(sketch->hash_prime_nums[i],(const char *)key, CMSKETCH_KEY_LEN)) % sketch->w;
}