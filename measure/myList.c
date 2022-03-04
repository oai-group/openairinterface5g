#include "myList.h"
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>

//创建链表
MyList * createMyList()
{
    MyList * re = (MyList *) malloc(sizeof(MyList));
    re->count = 0;
    re->first = NULL;
    re->last = NULL;
    re->equal = NULL;
    return re;
}
 int myEqualString2(uint8_t * a, uint8_t *b)
{
    for (int i = 0; i < KEY_LENGTH; i++){
        if(a[i]!=b[i])
            return 0;
        }
    // uint8_t  *aa = (uint8_t *) a;
    // uint8_t  *bb = (uint8_t *) b;
    return 1;
}
//释放链表
void freeMyList(MyList * list)
{
    MyNode * p;
    while (list->first)
    {
        p = list->first->next;
        free(list->first);
        list->first = p;
    }
    free(list);
}
 
//插入在尾部,通常是新建的一个节点
MyNode * myListInsertDataAtLast(MyList *  list, uint8_t*  data)
{
    
    // printf("begin malloc1 \n\n\n");    
    MyNode * node = (MyNode *) malloc(sizeof(MyNode));
    // printf("malloc done");
    // node->data = data;
    memset(node,0,sizeof(MyNode));
    memcpy(node->data,data,KEY_LENGTH);   
    node->isReceived = 1;
    // node->next = NULL;
    // node->isClassified = 0;
    
    // node->notReceived = 0;
    // node->totalTime = 0;
    // node->samplingData.lastSamplingTime = 0;


    //plr measure
    // memset(&node->plrData,0,sizeof(PLRData));

    // node->delayInfo = NULL;


    if (list->count)
    {
        list->last->next = node;
        list->last = node;
    }
    else
    {
        list->first = node;
        list->last = node;
    }
    (list->count)++;
    return node;
}
// 添加时延信息
void myListInsertDelayDataAtLast(MyList *  list, uint8_t*  data, DelayData *delayInfo)
{
    
    // printf("begin malloc1 \n\n\n");    
    MyNode * node = (MyNode *) malloc(sizeof(MyNode));
    // printf("malloc done");
    // node->data = data;
    // memcpy(node->data,data,KEY_LENGTH);
    // node->next = NULL;
    node->delayInfo = delayInfo;
    memset(node,0,sizeof(MyNode));
    memcpy(node->data,data,KEY_LENGTH);

    node->isReceived = 1;
    node->delayInfo = delayInfo;

    if (list->count)
    {
        list->last->next = node;
        list->last = node;
    }
    else
    {
        list->first = node;
        list->last = node;
    }
    (list->count)++;
}


void myListInsertSamplingDataAtLast(MyList *  list, uint8_t*  data)
{

    MyNode * node = (MyNode *) malloc(sizeof(MyNode));
    // memcpy(node->data,data,KEY_LENGTH);
    // node->next = NULL;

    // node->isClassified = 0;
    // node->isReceived = 1;
    // node->notReceived = 0;
    // node->totalTime = 0;
    // memset(&node->plrData,0,sizeof(PLRData));
    // node->delayInfo = NULL;

    memset(node,0,sizeof(MyNode));
    memcpy(node->data,data,KEY_LENGTH);
    node->isReceived = 1;
    struct timeval t;
    gettimeofday(&t, 0);
    node->samplingData.lastSamplingTime = (uint64_t)((uint64_t)t.tv_sec * 1000 * 1000 + t.tv_usec);





    if (list->count)
    {
        list->last->next = node;
        list->last = node;
    }
    else
    {
        list->first = node;
        list->last = node;
    }
    (list->count)++;
}

//recv端增加统计信息，在末尾，新建
void myListInsertRecvPLRDataAtLast(MyList *  list, uint8_t*  data,int flag)
{

    MyNode * node = (MyNode *) malloc(sizeof(MyNode));
    memset(node,0,sizeof(MyNode));
    memcpy(node->data,data,KEY_LENGTH);
    node->isReceived = 1;
    // memcpy(node->data,data,KEY_LENGTH);
    // node->next = NULL;

    //plr measure
    // memset(&node->plrData,0,sizeof(PLRData));
    node->plrData.recvCount = 1;
    // node->plrData.recvFlag = flag;

    // node->isClassified = 0;
    // node->isReceived = 1;
    // node->notReceived = 0;
    // node->totalTime = 0;


    // node->delayInfo = NULL;

    



    if (list->count)
    {
        list->last->next = node;
        list->last = node;
    }
    else
    {
        list->first = node;
        list->last = node;
    }
    (list->count)++;
}
//send端获取Flag信息，在末尾，新建
void myListInsertSendPLRDataAtLast(MyList *  list, uint8_t*  data)
{

    MyNode * node = (MyNode *) malloc(sizeof(MyNode));
    memset(node,0,sizeof(MyNode));
    memcpy(node->data,data,KEY_LENGTH);
    node->isReceived = 1;
    // memcpy(node->data,data,KEY_LENGTH);
    // node->next = NULL;

    // //plr measure
    // memset(&node->plrData,0,sizeof(PLRData));
    node->plrData.sendCount = 1;
    node->plrData.sendFlag = 0;

    // node->isClassified = 0;
    // node->isReceived = 1;
    // node->notReceived = 0;
    // node->totalTime = 0;
    // node->delayInfo = NULL;

    if (list->count)
    {
        list->last->next = node;
        list->last = node;
    }
    else
    {
        list->first = node;
        list->last = node;
    }
    (list->count)++;
}







 
//长度
int myListGetSize( MyList *  list)
{
    return list->count;
}
 
//打印
void myListOutput( MyList *  list, void(*pt)( void * ))
{
    MyNode * p = list->first;
    while (p)
    {
        (*pt)(p->data);
        p = p->next;
    }
}
 
//删除在尾部
void* myListRemoveDataAtLast(MyList*  list)
{
    if (list->count == 1)
    {
        return myListRemoveDataAtFirst(list);
    }
    MyNode * p = list->first;
    while (p->next != list->last)
    {
        p = p->next;
    }
    void *re = list->last->data;
    if(list->last->delayInfo)
        free(list->last->delayInfo);
    free(list->last);
    p->next = NULL;
    list->last = p;
    (list->count)--;
    return re;
}
 
//删除在首部
void* myListRemoveDataAtFirst(MyList *  list)
{
    MyNode *p = list->first;
    list->first = p->next;
    void * re = p->data;
    if(p->delayInfo)
        free(p->delayInfo);
    free(p);
    (list->count)--;
    if (list->count == 0)
    {
        list->last = NULL;
    }
    return re;
}
 
 
//删除
void* myListRemoveDataAt(MyList*  list, int index)
{
    if (index == 0)
    {
        return myListRemoveDataAtFirst(list);
    }
    if (index == list->count - 1)
    {
        return myListRemoveDataAtLast(list);
    }
 
    MyNode * p = list->first;
    for (int i = 0; i < index - 1; i++)
    {
        p = p->next;
    }
    MyNode *tp = p->next;
    p->next = p->next->next;
    void * re = tp->data;
    if(tp->delayInfo)
        free(tp->delayInfo);
    free(tp);
    (list->count)--;
    return re;
}
 
//取得数据
void* myListGetDataAt( MyList *  list, int index)
{
    if (index == list->count - 1)
    {
        return myListGetDataAtLast(list);
    }
    MyNode * p = list->first;
    for (int i = 0; i < index; i++)
    {
        p = p->next;
    }
    return p->data;
}
 
//取得第一个数据
void* myListGetDataAtFirst( MyList *  list)
{
    return list->first->data;
}
 
//取得最后一个数据
void* myListGetDataAtLast( MyList *  list)
{
    return list->last->data;
}
 
//查找某个数据的位置,如果equal方法为空，比较地址，否则调用equal方法
//如果不存在返回-1，如果存在，返回出现的第一个位置
MyNode * myListFindDataIndex( MyList *  list, uint8_t * data)
{
    MyNode * p = list->first;
    int re = 0;
        while (p)
    {
        // if (p->data == data || (*(list->equal))(p->data, data))
        if (myEqualString2(p->data, data))
        {
            //return re;
            return p;
        }
        re++;
        p = p->next;
    }

    return NULL;
}
 
//创建链表，带有相等参数，用于查找
MyList * createMySearchList(int(*equal)(uint8_t * a, uint8_t * b))
{
    MyList * re = createMyList();
    re->equal = equal;
    return re;
}
 
//创建遍历器
MyListIterator* createMyListIterator( MyList *  list)
{
    MyListIterator * re = (MyListIterator *) malloc(sizeof(MyListIterator));
    re->p = list->first;
    re->allSize = list->count;
    re->count = 0;
    return re;
}
 
//释放遍历器
void freeMyListIterator(MyListIterator* iterator)
{
    free(iterator);
}
 
//遍历器是否有下一个元素
int myListIteratorHasNext( MyListIterator*  iterator)
{
    return iterator->count < iterator->allSize;
}
 
//返回遍历器的下一个元素
void * myListIteratorNext(MyListIterator*  iterator)
{
    void * re = iterator->p->data;
    iterator->p = iterator->p->next;
    (iterator->count)++;
    return re;
}
 
//删除对象,返回是否删除成功
// int myListRemoveDataObject(MyList*  list, void * data)
// {
//     MyListIterator * it = createMyListIterator(list);
//     int a = 0;
//     while (myListIteratorHasNext(it))
//     {
//         void * ld = myListIteratorNext(it);
//         // if (data == ld || (list->equal != NULL && (*(list->equal))(ld, data)))
//         if (myEqualString2(ld, data))
//         {
//             a = 1;
//             break;
//         }
//     }
//     if (a)
//     {
//         myListRemoveDataAt(list, it->count - 1);
//     }
//     return a;
// }
int myListRemoveDataObject(MyList*  list, uint8_t * data)
{
    MyNode * p = list->first;
    int re = 0;
    int index = -1;
        while (p)
    {
        // if (p->data == data || (*(list->equal))(p->data, data))
        if (myEqualString2(p->data, data))
        {
            index = re;
        }
        re++;
        p = p->next;
    }
    if (index != -1)
    {
        myListRemoveDataAt(list,index);
        return 1;
    }
    else
        return 0;
    

}