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
 
//插入在尾部
void myListInsertDataAtLast(MyList *  list, uint8_t*  data)
{
    
    // printf("begin malloc1 \n\n\n");    
    MyNode * node = (MyNode *) malloc(sizeof(MyNode));
    // printf("malloc done");
    // node->data = data;
    memcpy(node->data,data,KEY_LENGTH);
    node->next = NULL;
    node->isClassified = 0;
    node->isReceived = 1;
    node->notReceived = 0;
    node->totalTime = 0;
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

void myListInsertDelayDataAtLast(MyList *  list, uint8_t*  data, DelayData *delayInfo)
{
    
    // printf("begin malloc1 \n\n\n");    
    MyNode * node = (MyNode *) malloc(sizeof(MyNode));
    // printf("malloc done");
    // node->data = data;
    memcpy(node->data,data,KEY_LENGTH);
    node->next = NULL;
    node->delayInfo = delayInfo;
    // node->isClassified = 0;
    // node->isReceived = 1;
    // node->notReceived = 0;
    // node->totalTime = 0;
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

    // printf("begin malloc1 \n\n\n");
    MyNode * node = (MyNode *) malloc(sizeof(MyNode));
    // printf("malloc done");
    // node->data = data;
    memcpy(node->data,data,KEY_LENGTH);
    node->next = NULL;
    struct timeval t;
    gettimeofday(&t, 0);
    node->samplingData.lastSamplingTime = (uint64_t)((uint64_t)t.tv_sec * 1000 * 1000 + t.tv_usec);
    // node->isClassified = 0;
    // node->isReceived = 1;
    // node->notReceived = 0;
    // node->totalTime = 0;
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
    printf("1");
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
 
//插入
// void myListInsertDataAt(MyList *  list, void*  data, int index)
// {
//     if (index == 0)
//     {
//         myListInsertDataAtFirst(list, data);
//         return;
//     }
//     if (index == list->count)
//     {
//         myListInsertDataAtLast(list, data);
//         return;
//     }
//     MyNode * node = (MyNode *) malloc(sizeof(MyNode));
//     node->data = data;
//     node->next = NULL;
 
//     MyNode * p = list->first;
//     for (int i = 0; i < index - 1; i++)
//     {
//         p = p->next;
//     }
//     node->next = p->next;
//     p->next = node;
 
//     (list->count)++;
// }
 
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
        // printf("\n pdata%d%d%d%d  ",p->data[9],p->data[10],p->data[11],p->data[12]);
        // printf("\n data%d%d%d%d   ",data[9],data[10],data[11],data[12]);
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
        // printf("\n pdata%d%d%d%d  ",p->data[9],p->data[10],p->data[11],p->data[12]);
        // printf("\n data%d%d%d%d   ",data[9],data[10],data[11],data[12]);
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