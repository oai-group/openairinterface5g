
#ifndef MYHASHCODE_H_INCLUDED
#define MYHASHCODE_H_INCLUDED

#include <string.h>
#include "myList.h"
#include "hash.h"
#include <time.h>
typedef unsigned char uint8_t;
typedef unsigned short  uint16_t;

#define HASHCODE_MULT 31

//string类型的hashCode
int myHashCodeString(void * a);

int myEqualString(uint8_t * a, uint8_t *b);


typedef struct myHashSet
{
    int size; //大小
    int initialCapacity; //初始容量
    float loadFactor; //加载因子
    int (*hashCode)(void *data);
    int (*equal)(void *data1, void *data2);
    MyList ** dataList;
} MyHashSet;

typedef struct myHashSetIterator
{
    int index; //第几个链表
    MyHashSet *set;
    MyNode *current;
    int count; //第几个数据
} MyHashSetIterator;

//创建HashSet
MyHashSet *createMyHashSet(int (*hashCode)(void *data),int (*equal)(void *data1,void *data2), MyHashSet set);
void initHashSet(int(*hashCode)(void *data), int(*equal)(void *data1, void *data2), MyHashSet *re);
//使用全部参数创建HashSet
MyHashSet *createMyHashSetForAll(int initialCapacity,float loadFactor,int (*hashCode)(void *data),int (*equal)(void *data1,void *data2));

//释放HashSet
void freeMyHashSet(MyHashSet * set);

//是否包含某个data
int myHashSetContains(MyHashSet * set, void * data);

//增加一条数据,返回是否添加成功
// int myHashSetAddData(MyHashSet *  set, void * data);
// int myHashSetAddData(MyHashSet *  set, uint8_t*  data);
MyNode *myHashSetAddData(MyHashSet *  set, uint8_t*  data);
int myHashSetAddDelayData(MyHashSet *  set, uint8_t*  flow_key,  DelayData *delayInfo);
int myHashSetAddSamplingData(MyHashSet *  set, uint8_t*  flow_key);

int myHashSetAddRecvPLRData(MyHashSet *  set, uint8_t*  flow_key,int flag);
int myHashSetGetSendPLRFlag(MyHashSet *  set, uint8_t*  flow_key);

int setNodeClassified(MyHashSet *  set, uint8_t*  flow_key);
int myHashSetIsClassified(MyHashSet *  set, uint8_t*  flow_key);
// int myHashSetAddData(MyHashSet *  set, uint8_t*  data, uint16_t len, int sock);

//数据的容量
int myHashSetGetSize( MyHashSet *  set);

//创建迭代器
MyHashSetIterator* createMyHashSetIterator(MyHashSet * set, MyHashSetIterator *ree);

//释放迭代器
void freeMyHashSetIterator(MyHashSetIterator* iterator);

//迭代器是否有下一个
int myHashSetIteratorHasNext(MyHashSetIterator* iterator);

//遍历下一个元素
// uint8_t * myHashSetIteratorNext(MyHashSetIterator* iterator);
MyNode* myHashSetIteratorNext(MyHashSetIterator* iterator);

//删除一条数据，返回是否删除成功
int myHashSetRemoveData(MyHashSet *  set, void *  data);

//将第二个Set的全部对象加入到第一个，返回增加的个数
// int myHashSetAddAllSet(MyHashSet * set,MyHashSet *other);

//复制HashSet
// MyHashSet* myHashSetCopy(MyHashSet * set);

//遍历
// void myHashSetOutput(MyHashSet *set, void(*pt)(void*));

#endif // MYHASHCODE_H_INCLUDED
