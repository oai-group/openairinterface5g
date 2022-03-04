
#ifndef MYLIST_H_INCLUDED
#define MYLIST_H_INCLUDED
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "ElasticSketch.h"
#include "params.h"
typedef unsigned char uint8_t;
typedef unsigned short  uint16_t;



typedef struct linkDelay{
    uint8_t startNode;
    uint8_t endNode;
    unsigned long delay;
} LinkDelay;

typedef struct delayData{
    int linkNum;
    int count;
    unsigned long NodeToNodeDelay;
    LinkDelay links[10];
} DelayData;

typedef struct samplingData{
    uint64_t lastSamplingTime;
} SamplingData;

typedef struct packetLossRateData
{
    // send
    int sendFlag;
    int sendCount;

    //recv;
    int recvFlag;
    int recvCount;
    int realRecv;
    int shouldRecv;

} PLRData;



 
typedef struct myNode
{
    uint8_t data[KEY_LENGTH];
    // int lenth;//数量
    // long count;//总字节数
    // int flag;//是否已发送标志
    // struct timespec *nowtime;
    // struct timespec** timer;
    // int longlog[10] ;

    // send
    int isClassified;
    int isReceived;
    int notReceived;
    int totalTime;
    int now_pkt_cnt;
    int now_byte_cnt;

    SamplingData samplingData;

    // receive
    DelayData *delayInfo;




    //PLR measure
    PLRData plrData;

    struct myNode *next;


} MyNode;




typedef struct myList
{
    MyNode * first;
    MyNode * last;
    int count;
    int (*equal)(uint8_t * a, uint8_t * b);
} MyList;
 
typedef struct myListIterator
{
    MyNode * p;
    int count;
    int allSize;
} MyListIterator;
 
//创建链表
MyList * createMyList();
 
//创建链表，带有相等参数，用于查找
MyList * createMySearchList(int(*equal)(uint8_t * a, uint8_t * b));
 
//释放链表
void freeMyList(MyList * list);
 
//插入在尾部
// void myListInsertDataAtLast(MyList*  list, uint8_t*  data,uint16_t len);
// void myListInsertDataAtLast(MyList*  list, uint8_t*  data);
MyNode *myListInsertDataAtLast(MyList*  list, uint8_t*  data);
void myListInsertDelayDataAtLast(MyList*  list, uint8_t*  data, DelayData *delayInfo);
void myListInsertSamplingDataAtLast(MyList *  list, uint8_t*  data);

void myListInsertRecvPLRDataAtLast(MyList *  list, uint8_t*  data,int flag);
void myListInsertSendPLRDataAtLast(MyList *  list, uint8_t*  data);


void myListAddtDataToIndex(MyList *  list, uint8_t*  data,uint16_t len, int index,int sock);
 
//插入在首部
void myListInsertDataAtFirst(MyList *  list, void*  data);
 
//插入
void myListInsertDataAt(MyList *  list, void*  data, int index);
 
//删除在尾部
void* myListRemoveDataAtLast(MyList*  list);
 
//删除在首部
void* myListRemoveDataAtFirst(MyList *  list);
 
//删除
void* myListRemoveDataAt(MyList*  list, int index);
 
//删除对象,返回是否删除成功
// int myListRemoveDataObject(MyList*  list, void * data);
int myListRemoveDataObject(MyList*  list, uint8_t * data);
 
//长度
int myListGetSize( MyList *  list);
 
//打印
void myListOutput( MyList *  list, void(*pt)( void * ));
 
//取得数据
void* myListGetDataAt( MyList *  list, int index);
 
//取得第一个数据
void* myListGetDataAtFirst( MyList *  list);
 
//取得最后一个数据
void* myListGetDataAtLast( MyList *  list);
 
//查找某个数据的位置,如果equal方法为空，比较地址，否则调用equal方法
//如果不存在返回-1，如果存在，返回出现的第一个位置
//int myListFindDataIndex( MyList *  list, uint8_t  * data);
MyNode *myListFindDataIndex( MyList *  list, uint8_t  * data);
 
//创建遍历器
MyListIterator* createMyListIterator( MyList *  list);
 
//释放遍历器
void freeMyListIterator(MyListIterator* iterator);
 
//遍历器是否有下一个元素
int myListIteratorHasNext( MyListIterator*  iterator);
 
//返回遍历器的下一个元素
void * myListIteratorNext(MyListIterator*  iterator);
// int cmp_int(const void *a, const void *b);
 
#endif // MYLIST_H_INCLUDED