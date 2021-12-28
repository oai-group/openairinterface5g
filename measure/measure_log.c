#include "measure_log.h"



//type = 0 代表上行 type = 1 代表下行；
void save_flow_statistics(int count, ElasticSketch *sketch,MyHashSet *Set, MYSQL *mysql, int type){
    FILE* fp;
    // myHashset * mySet = & Set;
    if (type == 0){
        fp = fopen(STATISTICS_LOG_FILE_SEND,"ab+");}
    else if (type == 1){
        fp = fopen(STATISTICS_LOG_FILE_RECV,"ab+");}
    else{
        fp = NULL;}
    
    if (fp){
        printf("had output statisics\n");
        fprintf(fp,"==========================the %d statisics============================\n",count);

        MyHashSetIterator itt;
        MyHashSetIterator * it = &itt;
        it->index = 0;
        it->set = Set;
        it->current = Set->dataList[0]->first;
        it->count = 0;

        int x = 0;
        //unsigned long t_bytes = 0;
       // unsigned long t_pkts = 0;
        double t_bytes = 0;
        double t_pkts = 0;

        fprintf(fp,"index\t\tSrc IP\t\t  Dst IP\tProtocolt\tSrc port\tDst port\tTotal Byte\tTotal Packet\n");
        delNode * head = NULL;
        delNode * last = NULL;
        PreNode * preHead = NULL;
        PreNode * preLast = NULL;
        // printf("change3 \n\n\n"); 


       
        while(myHashSetIteratorHasNext(it)){
            x++;

            // printf("begin get node \n\n\n");    
            MyNode* node = myHashSetIteratorNext(it);

            uint8_t * flow_key = node->data;

            node->totalTime += 1;
            // 更新未收到报文时间
            if(node->isReceived == 1){
                node->notReceived = 0;
            }
            else
                node->notReceived +=1;

            fprintf(fp,"%3d :   ", x);
            fprintf(fp,"    %3d.%3d.%3d.%3d   ", flow_key[0],flow_key[1],flow_key[2],flow_key[3]);
            fprintf(fp,"%3d.%3d.%3d.%3d   ", flow_key[4],flow_key[5],flow_key[6],flow_key[7]);
            if (flow_key[12] == 6){
                fprintf(fp,"  TCP\t");
            }else if (flow_key[12] == 17){
                fprintf(fp,"  UDP\t");
            }else{
                fprintf(fp,"  %3d\t",flow_key[12]);
            }
            fprintf(fp,"    %5d\t",htons(*((uint16_t*)&(flow_key[8]))));   
            fprintf(fp,"    %5d\t",htons(*((uint16_t*)&(flow_key[10]))));
            fprintf(fp," %1d %1d %1d ",node->notReceived,node->isReceived,node->isClassified);

            FIVE_TUPLE fkey;
            memcpy(fkey.flow_id,flow_key,13);
            VAL_TYPE result = sketch->Query(sketch,&fkey);
            //这个周期没收到报文，直接输出0
            if(node->isReceived == 0){
                
                // if(node->notReceived <= TIMEOUTSLOT){
                    fprintf(fp,"   %10u\t    %5u", 0, 0);
                    mysqldb_insert(mysql, flow_key, 0, 0);
                // }
            }
            else{
                if(node->isClassified == 1){
/*
                    fprintf(fp,"   %10.1f\t    %5.2f", result.tot_size/5.0,result.packet_num/5.0);
                    mysqldb_insert(mysql, flow_key, result.tot_size, result.packet_num);
                    t_bytes += result.tot_size;
                    t_pkts += result.packet_num;
*/
                    fprintf(fp,"   %10.2f\t    %5.2f", result.tot_size/5.0/node->totalTime,result.packet_num/5.0/node->totalTime);
                    // fprintf(fp,"   %10f.1\t    %5.1f", result.tot_size/5.0,result.packet_num/5.0);
                    mysqldb_insert(mysql, flow_key, result.tot_size/1.0/node->totalTime, result.packet_num/1.0/node->totalTime);
                    t_bytes += result.tot_size/1.0/node->totalTime;
                    t_pkts += result.packet_num/1.0/node->totalTime;
                    node->totalTime = 0;
                }
                else{
                    fprintf(fp,"   %10.2f\t    %5.2f", result.tot_size/5.0/node->totalTime,result.packet_num/5.0/node->totalTime);
                    // fprintf(fp,"   %10f.1\t    %5.1f", result.tot_size/5.0,result.packet_num/5.0);
                    mysqldb_insert(mysql, flow_key, result.tot_size/1.0/node->totalTime, result.packet_num/1.0/node->totalTime);
                    t_bytes += result.tot_size/1.0/node->totalTime;
                    t_pkts += result.packet_num/1.0/node->totalTime;
                }
            }
            fprintf(fp,"\n");
            


            //超时则记录，等会删除
            if(node->notReceived >= TIMEOUTSLOT){
                delNode *p = head;
                if(head==NULL){

                    delNode * del = (delNode *) malloc(sizeof(delNode));
                    head = del;
                    last = head;
                    head->next = NULL;
                    memcpy(head->data,flow_key,KEY_LENGTH);
                }
                else{
                    delNode * del = (delNode *) malloc(sizeof(delNode));
                    memcpy(del->data,flow_key,KEY_LENGTH);
                    del->next = NULL;
                    last->next = del;
                    last = del;
                }
            }
            // 没识别的也记录，等会用来保存
            if(node->isClassified == 0 && node->notReceived < TIMEOUTSLOT){
                PreNode *p = preHead;
                // VAL_TYPE saved;
                // memcpy(&saved,&result,sizeof(VAL_TYPE));
                if(preHead==NULL){
                    PreNode * sHead = (PreNode *) malloc(sizeof(PreNode));
                    preHead = sHead;
                    preLast = preHead;
                    preHead->next = NULL;
                    memcpy(preHead->key,flow_key,KEY_LENGTH);
                    memcpy(&preHead->val,&result,sizeof(VAL_TYPE));
                }
                else{
                    PreNode * temp = (PreNode *) malloc(sizeof(PreNode));
                    memcpy(temp->key,flow_key,KEY_LENGTH);
                    memcpy(&temp->val,&result,sizeof(VAL_TYPE));
                    temp->next = NULL;
                    preLast->next = temp;
                    preLast = temp;
                }                
            }                            



            node->isReceived = 0;

            // if(node->isClassified == 1){
            //     continue;
            // }                
        }



        fclose(fp);
        
        //将总的流量插入表
        mysqldb_insert2(mysql, count,t_bytes/5.0,t_pkts/5.0, type);
        sketch->Clear(sketch);

        //删除节点，释放链表
        delNode *p = head;
        // printf("now delete \n\n");
        int c = 1;
        while (p)
        {
            delNode * nextp = p->next;
            // printf("now delete %d\n\n", c++);
            int re = myHashSetRemoveData(Set,p->data);
            if(re){
                // printf("deleteset suc\n\n");
            }
            
            mysqldb_delete(mysql,p->data);
            free(p);
            p = nextp;
        }
        //保存未识别流，释放链表
        PreNode *q = preHead;
        while (q)
        {
            // printf("now delete sketch %d\n\n", c++);
            delNode * nextq = q->next;
            VAL_TYPE saved = q->val;
            for (int i = 0; i < q->val.packet_num; i++)
            {
                FIVE_TUPLE fkey;

                memcpy(fkey.flow_id,q->key,KEY_LENGTH);
                sketch->Insert(sketch,&fkey,&q->val.top_10_packets[i]);
            }
            
            free(q);
            q = nextq;
        }        
        


        printf("had close file\n");



    }


}

void mysqldb_insert(MYSQL *mysql, unsigned char *flow_key, double total_Bytes,double total_Pkts)
{  
    int t;  
    char *head = "INSERT INTO ";  
    char query[300];
    char field[100] = "srcIP, dstIP, srcPort, dstPort, protocol, averBytes, averPkts";
    char *left = "(";  
    char *right = ") ";  
    char *values = "VALUES";  
    char *TABLE_NAME = "measure";
    char message[200] = {0};
    unsigned long srcIP = 0;
    unsigned long dstIP = 0;
    unsigned int srcPort = 0;
    unsigned int dstPort = 0;
    unsigned int protocol = 0;

    srcIP = flow_key[0]*256*256*256UL + flow_key[1]*256*256 + flow_key[2]*256 + flow_key[3];    
    dstIP = flow_key[4]*256*256*256UL + flow_key[5]*256*256 + flow_key[6]*256 + flow_key[7];  
    protocol = flow_key[12];
    srcPort = htons(*((uint16_t*)&(flow_key[8])));
    dstPort = htons(*((uint16_t*)&(flow_key[10])));

    char strend[200] = {0};
    sprintf(strend,"ON DUPLICATE KEY UPDATE averBytes=%.2lf ,averPkts=%.2lf",total_Bytes/5.0, total_Pkts/5.0);



    sprintf(message, "%lu, %lu, %u, %u, %u, %.2lf, %.2lf", srcIP, dstIP, srcPort, dstPort, protocol, total_Bytes/5.0, total_Pkts/5.0);
    /* 拼接sql命令 */  
    sprintf(query, "%s%s%s%s%s%s%s%s%s%s", head, TABLE_NAME, left, field, right, values, left, message, right, strend);
    // printf("%s\n", query);  

    t = mysql_real_query(mysql, query, strlen(query));
    if (t) {  
        printf("Failed to query: %s\n", mysql_error(mysql));  
    } 
    // else {
    //     printf("\nInsert sucessfully!\n");
    // }    
}
void mysqldb_insert2(MYSQL *mysql, unsigned long time, double total_Bytes,double total_Pkts,int type)
{  
    int t;

    char *head1 = "INSERT INTO total_send(time, totalBytes,totalPkts) VALUES";
    char *head2 = "INSERT INTO total_recv(time, totalBytes,totalPkts) VALUES";  
    char query[300];
    char *left = "(";  
    char *right = ") ";  
    char message[200] = {0};
    char *head;

    if(type == 0) {
        head = head1;}
    else{
        head = head2;}

    sprintf(message, "%lu, %.2lf, %.2lf",time, total_Bytes, total_Pkts);
    /* 拼接sql命令 */  
    sprintf(query, "%s%s%s%s", head, left, message, right);
    // printf("%s\n", query);  

    t = mysql_real_query(mysql, query, strlen(query));
    if (t) {  
        printf("Failed to query: %s\n", mysql_error(mysql));  
    } 
    // else {
    //     printf("\nInsert sucessfully!\n");
    // }    
}
void mysqldb_delete(MYSQL *mysql, unsigned char *flow_key)
{  
    int t;  
    char *head = "DELETE FROM ";  
    char query[300];  
    char *TABLE_NAME = "measure";
    unsigned long srcIP = 0;
    unsigned long dstIP = 0;
    unsigned int srcPort = 0;
    unsigned int dstPort = 0;
    unsigned int protocol = 0;

    srcIP = flow_key[0]*256*256*256UL + flow_key[1]*256*256 + flow_key[2]*256 + flow_key[3];    
    dstIP = flow_key[4]*256*256*256UL + flow_key[5]*256*256 + flow_key[6]*256 + flow_key[7];  
    protocol = flow_key[12];
    srcPort = htons(*((uint16_t*)&(flow_key[8])));
    dstPort = htons(*((uint16_t*)&(flow_key[10])));


    /* 拼接sql命令 */  
    sprintf(query, "%s%s WHERE srcIP=%lu and dstIP=%lu and srcPort=%u and dstPort=%u and protocol=%u", head, TABLE_NAME, srcIP, dstIP, srcPort, dstPort, protocol);
    // printf("%s\n", query);  

    t = mysql_real_query(mysql, query, strlen(query));
    if (t) {  
        printf("Failed to query: %s\n", mysql_error(mysql));  
    }
    
}

void measure_packet(char* packet, MyHashSet * Set, int sock, pthread_mutex_t * mutex,ElasticSketch * elastic_sketch){
    packet_key_t packet_key;
    int is_ipv4_packet = extract_packet_key(packet,&packet_key);
    if (is_ipv4_packet == 0){

    pthread_mutex_lock(mutex);
    uint8_t flow_key[14]={'0'};
    packet_key_to_char(&packet_key, &flow_key);
    (packet[1] & 0x01) == 0x01


    if(packet_key.packet_len > 0){
      //printf("\n BYTES num %d\n",packet_key.packet_len);
      //////////////////
      myHashSetAddData(Set, &flow_key);
      if((packet[1] & 0x01) == 0x01){
          return ;
      }
      
      ///////////////////

      FIVE_TUPLE *fkey = (FIVE_TUPLE *)malloc(sizeof(FIVE_TUPLE));

      memcpy(fkey->flow_id,flow_key,13);


      struct timespec *nowtime = (struct timespec *) malloc(sizeof(struct timespec ));
      // struct timespec nowtime;          
      clock_gettime(CLOCK_REALTIME, nowtime);
      PACKET_INFO *packet_1 = (PACKET_INFO *)malloc(sizeof(PACKET_INFO ));

      // PACKET_INFO packet_1;
      memset(packet_1, 0, sizeof(PACKET_INFO));
      packet_1->size = packet_key.packet_len;
      packet_1->arrived_time = *nowtime;

      // printf("\n\n packet set done\n\n");
      (*elastic_sketch).Insert(elastic_sketch, fkey, packet_1);
      
      VAL_TYPE result = (*elastic_sketch).Query(elastic_sketch,fkey);
      

      if(result.packet_num == 10 && !myHashSetIsClassified(Set, &flow_key)){
          setNodeClassified(Set, &flow_key);
        //  printf("\n\n had 10 packet\n\n");
          TransData *TD = (TransData*)malloc(sizeof(TransData));
          // printf("sizeof(TransData) %d\n\n\n",sizeof(TransData));
          int lenthSum;
          long timeIntervalSum;
          lenthSum = 0;
          timeIntervalSum = 0;
          long timeInterval[9]={0};
          // printf("top 10 packet");
          for(int i=0; i<9 ;i++){
              
              struct timespec first = result.top_10_packets[i].arrived_time;
              struct timespec next = result.top_10_packets[i+1].arrived_time;
              // printf("top %d packet time %ld\n\n",i+1,first.tv_nsec);
              timeInterval[i] = (next.tv_sec - first.tv_sec)*1000000000L + (next.tv_nsec - first.tv_nsec);
              // printf("top %d  timeinter %ld\n\n",i+1,timeInterval[i]);
              timeIntervalSum += timeInterval[i];
          }
          int lenList[10] = {0};
          for(int i=0; i<10 ;i++){
              // printf("top %d packet len %ld\n",i+1,result.top_10_packets[i].size);
              lenList[i] = result.top_10_packets[i].size;
              // printf("top %d  sizer %d\n\n",i+1,lenList[i]);
              lenthSum += lenList[i];
          }
          qsort(lenList,10,sizeof(int), cmp_int);
          qsort(timeInterval,9,sizeof(long), cmp_int);
          TD->maxLenth = lenList[9];
          TD->minLenth = lenList[0];
          TD->averLenth = lenthSum/10.0;
          TD->MedianLenth = lenList[4];
          float var = 0;
          for(int i=0; i<10 ;i++){
              var += (lenList[i] - TD->averLenth)*(lenList[i]-TD->averLenth);
          }
          TD->varLenth = sqrt(var/10);


          TD->maxInterval = timeInterval[8];
          TD->minInterval = timeInterval[0];
          TD->averInterval = timeIntervalSum/9.0;
          TD->MedianInterval = timeInterval[4];
          
          double var2 = 0;
          for(int i=0; i<9 ;i++){
              var2 += ((timeInterval[i]-TD->averInterval)/10000.0)*((timeInterval[i]-TD->averInterval)/10000.0)/9.0;
          }
          TD->varInterval =sqrt(var2)*10000.0;
          memcpy(TD->data,fkey->flow_id,13);
          // printf("cul done\n\n\n");

          char snd_buf[80] = {0};
          memcpy(snd_buf,TD,sizeof(*TD));
          write(sock, snd_buf, sizeof(snd_buf));
          free(TD);
      }
      free(fkey);
      free(nowtime);
      free(packet_1);
    }
    pthread_mutex_unlock(mutex);
  }
}
