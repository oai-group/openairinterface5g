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


// #define RESULT_LOG_FILE "/home/enb/openairinterface5g/measure_log/results_log.txt"
// #define TIMER_LOG_FILE "/home/enb/openairinterface5g/measure_log/timer_log.txt"
// #define STATISTICS_LOG_FILE_SEND "/home/enb/openairinterface5g/measure_log/statistics_log_SEND.txt"
// #define STATISTICS_LOG_FILE_RECV "/home/enb/openairinterface5g/measure_log/statistics_log_RECV.txt"

#define RESULT_LOG_FILE "../../../measure_log/results_log.txt"
#define TIMER_LOG_FILE "../../../measure_log/timer_log.txt"
#define STATISTICS_LOG_FILE_SEND "../../../measure_log/statistics_log_SEND.txt"
#define STATISTICS_LOG_FILE_RECV "../../../measure_log/statistics_log_RECV.txt"

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

typedef struct insertNode
{
    /* key */
    // 0 上行   1 下行
    uint8_t type;
    uint8_t hasDelayInfo;



    uint8_t  key[KEY_LENGTH];
    //volume data
    double total_Bytes;
    double total_Pkts;

    double delay;
    double loss;

    //status data


    struct insertNode *next;
}insertNode;

typedef struct insertData{
    uint32_t size;
    insertNode* head;
}insertData;


typedef struct recvPacketHeadNode{
    uint8_t  key[KEY_LENGTH];

    uint32_t packetLength;//报文长度
    struct timespec *nowtime;

    uint8_t flag;//丢包率信息

    uint8_t packetType;//正常报文：0 或者  时延报文：1。 正常报文有长度丢包率等信息，时延测量报文没有

    
    

    DelayData *dData; //正常报文没有

    struct recvPacketHeadNode *next;


}recvPacketHeadNode;

typedef struct tmpRecvData{
    uint32_t size;

    recvPacketHeadNode *head;
    recvPacketHeadNode *tail;

}tmpRecvData;

// 0 = send ,1 = recv
void save_flow_statistics(int count, ElasticSketch *sketch,MyHashSet *Set, MYSQL *mysql, int type, insertData *inData);
// void save_flow_statistics(int count, ElasticSketch *sketch,MyHashSet *Set, MYSQL *mysql, int type, insertNode *head);

void mysqldb_insert(MYSQL *mysql, unsigned char *flow_key, double total_Bytes,double total_Pkts);
void mysqldb_insert2(MYSQL *mysql, unsigned long time, double total_Bytes,double total_Pkts, int type);
void mysqldb_delete(MYSQL *mysql, unsigned char *flow_key);
void measure_packet(char* packet, MyHashSet * Set, int sock, pthread_mutex_t *mutex,ElasticSketch *elastic_sketch);

void insertDataToDB(insertData* inData,MYSQL *mysql);

void processTmpPacket(tmpRecvData tmp, MyHashSet * Set, int sock,ElasticSketch *elastic_sketch,int num);

#endif

