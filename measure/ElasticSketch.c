
#include "ElasticSketch.h"


//构造函数
void _constructor_ElasticSketch(struct ElasticSketch * elastic_sketch_p, int bucket_num, int light_part_counter_num){
    elastic_sketch_p->bucket_num = bucket_num;
    elastic_sketch_p->light_part_counter_num = light_part_counter_num;

    Init_HeavyPart(&(elastic_sketch_p->heavy_part), bucket_num);
    Init_LightPart(&(elastic_sketch_p->light_part), elastic_sketch_p->light_part_counter_num);
}

//析构函数
void _destructor_ElasticSketch(struct ElasticSketch * elastic_sketch_p){
    elastic_sketch_p->heavy_part.Destructor(&(elastic_sketch_p->heavy_part));
    elastic_sketch_p->light_part.Destrcutor(&(elastic_sketch_p->light_part));
}

//清除内存
void _clear_ElasticSketch(struct ElasticSketch * elastic_sketch_p){
    elastic_sketch_p->light_part.Clear(&(elastic_sketch_p->light_part));
    elastic_sketch_p->heavy_part.Clear(&(elastic_sketch_p->heavy_part));
}

//插入
void _insert_ElasticSketch(struct ElasticSketch * elastic_sketch_p, KEY_TYPE * key_p, PACKET_INFO * packet_p){
    KEY_TYPE swap_key;
    VAL_TYPE swap_val;
    memset(&swap_val,0,sizeof(VAL_TYPE));

    //先尝试插入heavy_part，根据返回值判断是否插入成功
    int result = elastic_sketch_p->heavy_part.Insert(&(elastic_sketch_p->heavy_part), key_p, &swap_key, &swap_val ,packet_p);

    switch(result) {
    // 找到匹配项
    case 0:
      return;

    case 1: {
      if(swap_val.is_evicted) {
        elastic_sketch_p->light_part.InsertFlow(&(elastic_sketch_p->light_part), &swap_key, &swap_val);
      } else {
        elastic_sketch_p->light_part.SwapInsert(&(elastic_sketch_p->light_part), &swap_key, &swap_val);
      }

      return;
    }

    case 2:
      elastic_sketch_p->light_part.Insert(&(elastic_sketch_p->light_part), key_p, packet_p);

      return;

    default:
      printf("error return value !\n");
      exit(1);
    }
}

//查询
VAL_TYPE _query_ElasticSketch(struct ElasticSketch * elastic_sketch_p, KEY_TYPE * key_p){
    VAL_TYPE heavy_result = elastic_sketch_p->heavy_part.Query(&(elastic_sketch_p->heavy_part), key_p);
   
    //如果heavy_part中没找到(tot_size == 0)或者在heavy_part中找到了，但is_evicted为true
    //该流的大小需要加上light_part的部分
    if(heavy_result.tot_size == 0 || heavy_result.is_evicted == 1) {
      //查询light_part
      VAL_TYPE light_result = elastic_sketch_p->light_part.Query(&(elastic_sketch_p->light_part), key_p);
      
      //一般而言，is_evicted为true表示该流开始是存放在light_part，后来由于大小超过阈值，再取出放到heavy_part中
      //因此，该流的前10个数据包应该在light_part中，不足10个的在heavy_part中补齐
      //但是存在误差
      
      if (light_result.packet_num < TOP_10_PACKET){
        for (int i = light_result.packet_num, j= 0; i < TOP_10_PACKET && j < heavy_result.packet_num; i++,j++){
          light_result.top_10_packets[i] = heavy_result.top_10_packets[j];
        }
      }

        light_result.tot_size += heavy_result.tot_size;
        light_result.packet_num += heavy_result.packet_num;

        return light_result;

    }

    return heavy_result;
}

//初始化
//注意步骤：
//1.先创建ElasticSketch实例
//2.调用Init_ElasticSketch()方法初始化
//3.调用完成后徐亚显式调用析构函数释放内存
void Init_ElasticSketch(struct ElasticSketch * elastic_sketch_p, int bucket_num, int tot_mem_in_bytes){
    elastic_sketch_p->Constructor = _constructor_ElasticSketch;
    elastic_sketch_p->Destructor = _destructor_ElasticSketch;
    elastic_sketch_p->Clear = _clear_ElasticSketch;
    elastic_sketch_p->Insert = _insert_ElasticSketch;
    elastic_sketch_p->Query = _query_ElasticSketch;

    elastic_sketch_p->Constructor(elastic_sketch_p, bucket_num, tot_mem_in_bytes);
}

int cmp_int(const void *a, const void *b) {
    long *_a = (long *)a;
    long *_b = (long *)b;
    if(*_a> *_b){
      return 1;
    }
    else
    {
      return -1;
    }
    
    // return *_a - *_b;
}
