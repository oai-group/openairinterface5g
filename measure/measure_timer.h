#ifndef _MEASURE_TIMER_H
#define _MEASURE_TIMER_H

//通过开辟一个新线程，使用sleep来定时
//不使用定时器，以避免占用信号量

#include <unistd.h>
#include "openair2/COMMON/commonDef.h"
#include <pthread.h>
#include <time.h>
#include "measure_log.h"
// #include "myList.h"
// #include "myHashSet.h"
// #include "ElasticSketch.h"
#include <unistd.h>
#include <arpa/inet.h>
#include <mysql/mysql.h>



typedef struct timer_param_t
{
    int time_val;
    MyHashSet *recv_Set;
    ElasticSketch *recv_sketch;
    pthread_mutex_t* recv_mutex;
    MyHashSet *send_Set;
    ElasticSketch *send_sketch;
    pthread_mutex_t* send_mutex;
    int sock;
    // int type;

    int signal;
    tmpRecvData tmp;
}timer_param_t;

timer_param_t timer_param;


//定时器需要执行的动作
//测试定时器，每隔一段时间打印一次当前时间
void* print_current_time(void* argv);
//创建并启动定时器线程
// void measure_timer_create(int time_val,MyHashSet *Set, ElasticSketch *sketch,pthread_mutex_t* mutex,int sock, int type);
void measure_timer_create(  int time_val,
                            MyHashSet *recv_Set, ElasticSketch *recv_sketch,pthread_mutex_t* recv_mutex,
                            MyHashSet *send_Set, ElasticSketch *send_sketch,pthread_mutex_t* send_mutex,
                            int sock,
                            int signal,tmpRecvData tmp);

#endif