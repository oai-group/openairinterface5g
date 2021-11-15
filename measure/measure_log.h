#ifndef _MEASURE_LOG_H
#define _MEASURE_LOG_H

#include <stdio.h>
#include "parser.h"
// #include "myList.h"
// #include "myHashSet.h"
// #include "CMSketch.h"
#include "ElasticSketch.h"
#include "myHashSet.h"
#include "netinet/in.h"
#include <mysql/mysql.h>
#include <pthread.h>
#include <time.h>
#include <math.h>
#include <sys/socket.h>

#define RESULT_LOG_FILE "/home/enb1/openairinterface5g/measure_log/results_log.txt"
#define TIMER_LOG_FILE "/home/enb1/openairinterface5g/measure_log/timer_log.txt"
#define STATISTICS_LOG_FILE_SEND "/home/enb1/openairinterface5g/measure_log/statistics_log_SEND.txt"
#define STATISTICS_LOG_FILE_RECV "/home/enb1/openairinterface5g/measure_log/statistics_log_RECV.txt"
// #define RESULT_LOG_FILE "../measure_log/results_log.txt"
// #define TIMER_LOG_FILE "../measure_log/timer_log.txt"
// #define STATISTICS_LOG_FILE_SEND "../measure_log/statistics_log_SEND.txt"
// #define STATISTICS_LOG_FILE_RECV "../measure_log/statistics_log_RECV.txt"

typedef struct delNode
{
    uint8_t  data[KEY_LENGTH];
    struct delNode *next;
} delNode;
typedef struct PreserveNode
{
    uint8_t  key[KEY_LENGTH];
    VAL_TYPE val;
    struct PreserveNode *next;
} PreNode;
// 0 = send ,1 = recv
void save_flow_statistics(int count, ElasticSketch *sketch,MyHashSet *Set, MYSQL *mysql, int type);

void mysqldb_insert(MYSQL *mysql, unsigned char *flow_key, double total_Bytes,double total_Pkts);
void mysqldb_insert2(MYSQL *mysql, unsigned long time, double total_Bytes,double total_Pkts, int type);
void mysqldb_delete(MYSQL *mysql, unsigned char *flow_key);
void measure_packet(char* packet, MyHashSet * Set, int sock, pthread_mutex_t *mutex,ElasticSketch *elastic_sketch);
// void test_show();
// void show_packet_key(packet_key_t* key);

// void show_flow_key(uint8_t* flow_key);
// void show_flow_cnt(uint8_t* flow_key, int cnt);
// void show_num(int i);
// void show_timer_num(int i);
// void show_timer_time(char* str);

#endif

