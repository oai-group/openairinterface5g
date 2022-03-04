#include "myHashSet.h"

#define DEFAULT_LOAD_FACTOR 2
#define DEFAULT_INITIAL_CAPACITY 10000

int myHashCodeString(void * a)
{
    int index = (Bob_hash32(750,(const char *)a, KEY_LENGTH));
    // int re = 0;
    // char *aa = (char *) a;
    // while (*aa)
    // {
    //     re += HASHCODE_MULT * *aa;
    //     aa++;
    // }
    // return re;
    return index;
    
}




int myEqualString(uint8_t * a, uint8_t *b)
{
    for (int i = 0; i < 13; i++){
        if(a[i]!=b[i])
            return 0;
        }
    // uint8_t  *aa = (uint8_t *) a;
    // uint8_t  *bb = (uint8_t *) b;
    return 1;
}
//创建HashSet
MyHashSet *createMyHashSet(int(*hashCode)(void *data), int(*equal)(void *data1, void *data2), MyHashSet set)
{
   // MyHashSet *re = malloc(sizeof(MyHashSet));
    MyHashSet * re = &set;
    re->size = 0;
    re->loadFactor = DEFAULT_LOAD_FACTOR;
    re->initialCapacity = DEFAULT_INITIAL_CAPACITY;
    re->dataList = (MyList **) malloc(sizeof(MyList*) * re->initialCapacity);
    re->hashCode = hashCode;
    re->equal = equal;
    for (int i = 0; i < re->initialCapacity; i++)
    {
        re->dataList[i] = createMySearchList(equal);
    }
    return re;
}
void initHashSet(int(*hashCode)(void *data), int(*equal)(void *data1, void *data2), MyHashSet *re){
    re->size = 0;
    re->loadFactor = DEFAULT_LOAD_FACTOR;
    re->initialCapacity = DEFAULT_INITIAL_CAPACITY;
    re->dataList = (MyList **) malloc(sizeof(MyList*) * re->initialCapacity);
    re->hashCode = hashCode;
    re->equal = equal;
    for (int i = 0; i < re->initialCapacity; i++)
    {
        re->dataList[i] = createMySearchList(equal);
    }
}
//使用全部参数创建HashSet
MyHashSet *createMyHashSetForAll(int initialCapacity, float loadFactor, int(*hashCode)(void *data), int(*equal)(void *data1, void *data2))
{
    MyHashSet set1;
    MyHashSet *re = createMyHashSet(hashCode, equal,set1);
    re->initialCapacity = initialCapacity;
    re->loadFactor = loadFactor;
    return re;
}
//释放HashSet
void freeMyHashSet(MyHashSet * set)
{
    for (int i = 0; i < set->initialCapacity; i++)
    {
        freeMyList(set->dataList[i]);
    }
    free(set->dataList);
    // set->dataList = (MyList **) malloc(sizeof(MyList*) * set->initialCapacity);
//    free(set);
}
//是否包含某个data
int myHashSetContains(MyHashSet *  set, void *  data)
{
    int hasCode = (*(set->hashCode))(data);
    hasCode %= set->initialCapacity;
    if (hasCode<0)
        hasCode+=set->initialCapacity;
    int re = myListFindDataIndex(set->dataList[hasCode], data);
    return re > -1;
}
int myHashSetIsClassified(MyHashSet *  set, uint8_t*  flow_key)
{
    int hasCode = (*(set->hashCode))(flow_key);
    hasCode %= set->initialCapacity;
    if (hasCode<0)
        hasCode+=set->initialCapacity;
    MyNode * re = myListFindDataIndex(set->dataList[hasCode], flow_key);
    if(re)
        return re->isClassified;
    else
        return 0;
    return 0;
}


//增加一条数据,返回是否添加成功
// int myHashSetAddData(MyHashSet *  set, void *  data)
// int myHashSetAddData(MyHashSet *  set, uint8_t*  flow_key)
MyNode * myHashSetAddData(MyHashSet *  set, uint8_t*  flow_key)
{
    
    // struct timespec *nowtime = (struct timespec *) malloc(sizeof(struct timespec ));
    // clock_gettime(CLOCK_REALTIME, nowtime);
    
    uint8_t *data = (uint8_t *) malloc(sizeof(uint8_t)*13);
    
    // for (int i = 0; i < 13; i++){
    //         data[i] = flow_key[i];
    //     }
    memcpy(data, flow_key, 13);
    int hasCode = (*(set->hashCode))(data);
    
    hasCode %= set->initialCapacity;
    if (hasCode<0)
        hasCode+=set->initialCapacity;
   // printf("\n\n\nhashcode: %d \n\n",hasCode);
    MyNode * re = myListFindDataIndex(set->dataList[hasCode], data);
    // printf("\n\n\nreeeee: %d \n\n",re);
    if (re == NULL)
    {
        MyNode * ans = myListInsertDataAtLast(set->dataList[hasCode], data);
        // MyNode *node = set->dataList[hasCode]->first;
        (set->size)++;
        free(data);
        return ans;
    }
    else{
        re->isReceived = 1;
        
    }
    free(data);
    return re;
}

//增加一条延迟数据,返回是否添加成功
int myHashSetAddDelayData(MyHashSet *  set, uint8_t*  flow_key, DelayData *delayInfo)
{  
    uint8_t *data = (uint8_t *) malloc(sizeof(uint8_t)*13);
    
    memcpy(data, flow_key, 13);
    int hasCode = (*(set->hashCode))(data);
    
    hasCode %= set->initialCapacity;
    if (hasCode<0)
        hasCode+=set->initialCapacity;
   // printf("\n\n\nhashcode: %d \n\n",hasCode);
    MyNode * re = myListFindDataIndex(set->dataList[hasCode], data);
    // printf("\n\n\nreeeee: %d \n\n",re);
    if (re == NULL)
    {
        myListInsertDelayDataAtLast(set->dataList[hasCode], data, delayInfo);
        // MyNode *node = set->dataList[hasCode]->first;
        (set->size)++;
        free(data);
        return 1;
    }
    else{
        int lastCount = 0;
        unsigned long lastNodeToNodeDelay = 0;

        if(re->delayInfo){
            lastCount = re->delayInfo->count;
            lastNodeToNodeDelay = re->delayInfo->NodeToNodeDelay;
            free(re->delayInfo);
            delayInfo->count = lastCount + 1;
            delayInfo->NodeToNodeDelay = (lastNodeToNodeDelay * lastCount + delayInfo->NodeToNodeDelay)/delayInfo->count;
            
        }
        re->delayInfo = delayInfo;
        // 打印计算出来的时延信息
        printf("MyHashSet 179 -> delayInfo->NodeToNodeDelay :  %lu\n", delayInfo->NodeToNodeDelay);

        // re->isReceived = 1;
    }
    free(data);
    return 0;
}

int myHashSetAddSamplingData(MyHashSet *  set, uint8_t*  flow_key)
{
    double p = 0.1;
    // int p = 10;
    uint8_t *data = (uint8_t *) malloc(sizeof(uint8_t)*13);

    memcpy(data, flow_key, 13);
    int hasCode = (*(set->hashCode))(data);

    hasCode %= set->initialCapacity;
    if (hasCode<0)
        hasCode+=set->initialCapacity;
    // printf("\n\n\nhashcode: %d \n\n",hasCode);
    MyNode * re = myListFindDataIndex(set->dataList[hasCode], data);
    // printf("\n\n\nreeeee: %d \n\n",re);
    if (re == NULL)
    {
        myListInsertSamplingDataAtLast(set->dataList[hasCode], data);
        // MyNode *node = set->dataList[hasCode]->first;
        (set->size)++;
        free(data);
        return 1;
    }
    else{
        free(data);
        struct timeval t;
        uint64_t lastSamplingTime = 0;
        lastSamplingTime = re->samplingData.lastSamplingTime;
        gettimeofday(&t, 0);
        uint64_t nowtime = (uint64_t)((uint64_t)t.tv_sec * 1000 * 1000 + t.tv_usec);
        if(( nowtime - lastSamplingTime) > 5000000){
            re->samplingData.lastSamplingTime = nowtime;
            return 1;
        }
        else{
            // srand((unsigned)time(NULL));
            double a = (rand()%1000)/(double)1000;
            // uint8_t a = rand() % 100;
            if(a < p){
                re->samplingData.lastSamplingTime = nowtime;
                return 1;
            }
            else
                return 0;
        }

        // re->isReceived = 1;

        return 0;
    }

}
//recv端插入统计信息
int myHashSetAddRecvPLRData(MyHashSet *  set, uint8_t*  flow_key, int flag)
{  
    uint8_t *data = (uint8_t *) malloc(sizeof(uint8_t)*13);
    memcpy(data, flow_key, 13);
    int hasCode = (*(set->hashCode))(data);
    hasCode %= set->initialCapacity;
    if (hasCode<0)
        hasCode+=set->initialCapacity;
    MyNode * re = myListFindDataIndex(set->dataList[hasCode], data);


    if (re == NULL)
    {
        myListInsertRecvPLRDataAtLast(set->dataList[hasCode], data, flag);
        // MyNode *node = set->dataList[hasCode]->first;
        (set->size)++;
        free(data);
        return 1;
    }
    else{
        if(re->plrData.recvFlag!=flag){
            //换颜色
            re->plrData.realRecv += (re->plrData.recvCount > COLORSLOT)? COLORSLOT:re->plrData.recvCount; 
            re->plrData.recvCount = 1;
            re->plrData.recvFlag = flag;
            re->plrData.shouldRecv += COLORSLOT;
        }else{
            re->plrData.recvCount += 1;
        }
        // re->isReceived = 1;
    }
    free(data);
    return 0;
}

//send端返回PLR测量的标志位
int myHashSetGetSendPLRFlag(MyHashSet *  set, uint8_t*  flow_key)
{  
    uint8_t *data = (uint8_t *) malloc(sizeof(uint8_t)*13);
    memcpy(data, flow_key, 13);
    int hasCode = (*(set->hashCode))(data);
    hasCode %= set->initialCapacity;
    if (hasCode<0)
        hasCode+=set->initialCapacity;
    MyNode * re = myListFindDataIndex(set->dataList[hasCode], data);


    if (re == NULL)
    {
        myListInsertSendPLRDataAtLast(set->dataList[hasCode], data);
        (set->size)++;
        free(data);
        return 0;
    }
    else{
        re->plrData.sendCount += 1;
        if(re->plrData.sendCount > COLORSLOT){
            re->plrData.sendFlag ^= 1;
            re->plrData.sendCount = 1;
        }
        free(data);
        return re->plrData.sendFlag;
    }
}







int setNodeClassified(MyHashSet *  set, uint8_t*  flow_key)
{
    
    // struct timespec *nowtime = (struct timespec *) malloc(sizeof(struct timespec ));
    // clock_gettime(CLOCK_REALTIME, nowtime);
    
    uint8_t *data = (uint8_t *) malloc(sizeof(uint8_t)*13);
    
    for (int i = 0; i < 13; i++){
            data[i] = flow_key[i];
        }
    int hasCode = (*(set->hashCode))(data);
    hasCode %= set->initialCapacity;
    if (hasCode<0)
        hasCode+=set->initialCapacity;
   // printf("\n\n\nhashcode: %d \n\n",hasCode);
    MyNode * re = myListFindDataIndex(set->dataList[hasCode], data);
    free(data);
    re->isClassified = 1;
    return 0;
}

//数据的容量
int myHashSetGetSize( MyHashSet *  set)
{
    return set->size;
}

//创建迭代器
MyHashSetIterator* createMyHashSetIterator(MyHashSet * set, MyHashSetIterator *ree)
{
    // MyHashSetIterator* re = (MyHashSetIterator*) malloc(
    //                             sizeof(MyHashSetIterator));
    // MyHashSetIterator ree;
    // MyHashSetIterator* re = &ree;
    ree->count = 0;
    ree->index = 0;
    ree->set = set;
    ree->current = set->dataList[0]->first;
    return ree;
}

//释放迭代器
void freeMyHashSetIterator(MyHashSetIterator* iterator)
{
    free(iterator);
}

//迭代器是否有下一个
int myHashSetIteratorHasNext(MyHashSetIterator* iterator)
{
    return iterator->count < iterator->set->size;
}

//遍历下一个元素
// uint8_t* myHashSetIteratorNext(MyHashSetIterator* iterator)
// {
//     (iterator->count)++;
//     while (!(iterator->current))
//     {
//         (iterator->index)++;
//         iterator->current = iterator->set->dataList[iterator->index]->first;
//     }
//     uint8_t * re = iterator->current->data;
//     iterator->current = iterator->current->next;
//     return re;
// }
MyNode* myHashSetIteratorNext(MyHashSetIterator* iterator)
{
    (iterator->count)++;
    while (!(iterator->current))
    {
        (iterator->index)++;
        iterator->current = iterator->set->dataList[iterator->index]->first;
    }
    uint8_t * re = iterator->current->data;
    MyNode* reNode = iterator->current;
    iterator->current = iterator->current->next;
    return reNode;
}

//删除一条数据，返回是否删除成功
int myHashSetRemoveData(MyHashSet *  set, void *  data)
{
    int hasCode = (*(set->hashCode))(data);
    hasCode %= set->initialCapacity;
    if (hasCode<0)
        hasCode+=set->initialCapacity;
    int re = myListRemoveDataObject(set->dataList[hasCode], data);
    if (re)
    {
        (set->size)--;
    }
    return re;
}

//将第二个Set的全部对象加入到第一个，返回增加的个数
// int myHashSetAddAllSet(MyHashSet * set,MyHashSet *other)
// {
//     int ssize=set->size;
//     MyHashSetIterator * it=createMyHashSetIterator(other);
//     while (myHashSetIteratorHasNext(it))
//     {
//         myHashSetAddData(set,myHashSetIteratorNext(it));
//     }
//     freeMyHashSetIterator(it);
//     int re=set->size-ssize;
//     return re;
// }

//复制HashSet
// MyHashSet* myHashSetCopy(MyHashSet * set)
// {
//     MyHashSet* re=createMyHashSetForAll(set->initialCapacity,set->loadFactor,set->hashCode,set->equal);
//     myHashSetAddAllSet(re,set);
//     return re;
// }

//遍历
// void myHashSetOutput(MyHashSet *set, void(*pt)(void*))
// {
//     MyHashSetIterator * it=createMyHashSetIterator(set);
//     while (myHashSetIteratorHasNext(it))
//     {
//         pt(myHashSetIteratorNext(it));
//     }
//     freeMyHashSetIterator(it);
// }
