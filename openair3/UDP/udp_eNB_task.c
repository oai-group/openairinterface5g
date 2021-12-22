/*
 * Licensed to the OpenAirInterface (OAI) Software Alliance under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The OpenAirInterface Software Alliance licenses this file to You under
 * the OAI Public License, Version 1.1  (the "License"); you may not use this file
 * except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.openairinterface.org/?page_id=698
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *-------------------------------------------------------------------------------
 * For more information about the OpenAirInterface (OAI) Software Alliance:
 *      contact@openairinterface.org
 */

/*! \file udp_eNB_task.c
* \brief
* \author Sebastien ROUX, Lionel Gauthier
* \company Eurecom
* \email: lionel.gauthier@eurecom.fr
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <pthread.h>

#include "queue.h"
#include "intertask_interface.h"
#include "assertions.h"
#include "udp_eNB_task.h"

#include "common/utils/LOG/log.h"
#include "common/utils/LOG/vcd_signal_dumper.h"
#include "msc.h"

///////////////////////////////////////////
#include "measure/parser.h"
// #include "measure/measure_log.h"
// #include "measure/CMSketch.h"
// #include "measure/myList.h"
#include "measure/myHashSet.h"
#include "measure/ElasticSketch.h"
#include "measure/measure_timer.h"

typedef unsigned long int  uint64_t;


//创建下行表
MyHashSet recvSet;
//创建上行表
MyHashSet sendSet;
// MyHashSet Set;
// pthread_mutex_t mutex;
// ElasticSketch elastic_sketch;
pthread_mutex_t recv_mutex;
pthread_mutex_t send_mutex;


ElasticSketch recv_elastic_sketch;
ElasticSketch send_elastic_sketch;

int sock;
// enb id
uint8_t curr_eNB_id;
///////////////////////////////////////////

// 计算校验和的函数
uint16_t check_ip_sum(uint16_t* buffer, int size)
{
    unsigned long cksum = 0;
    while(size>1)
    {
        cksum += *buffer++;
        size -= sizeof(uint16_t);
    }
    if(size)
    {
        cksum += *(uint16_t*)buffer;
    }
    cksum = (cksum>>16) + (cksum&0xffff); 
    cksum += (cksum>>16); 
    return (uint16_t)(~cksum);
}

// 获取时间戳的函数
uint64_t getTimeUsec()
{
    struct timeval t;
    gettimeofday(&t, 0);
    return (uint64_t)((uint64_t)(t.tv_sec) * 1000 * 1000 + t.tv_usec);
}

#define IPV4_ADDR    "%u.%u.%u.%u"
#define IPV4_ADDR_FORMAT(aDDRESS)               \
    (uint8_t)((aDDRESS)  & 0x000000ff),         \
    (uint8_t)(((aDDRESS) & 0x0000ff00) >> 8 ),  \
    (uint8_t)(((aDDRESS) & 0x00ff0000) >> 16),  \
    (uint8_t)(((aDDRESS) & 0xff000000) >> 24)


struct udp_socket_desc_s {
  int       sd;              /* Socket descriptor to use */

  pthread_t listener_thread; /* Thread affected to recv */

  char     *local_address;   /* Local ipv4 address to use */
  uint16_t  local_port;      /* Local port to use */

  task_id_t task_id;         /* Task who has requested the new endpoint */

  STAILQ_ENTRY(udp_socket_desc_s) entries;
};

static STAILQ_HEAD(udp_socket_list_s, udp_socket_desc_s) udp_socket_list;
static pthread_mutex_t udp_socket_list_mutex = PTHREAD_MUTEX_INITIALIZER;


static
struct udp_socket_desc_s *
udp_eNB_get_socket_desc(task_id_t task_id);

void udp_eNB_process_file_descriptors(
  struct epoll_event *events,
  int nb_events);

static
int
udp_eNB_create_socket(
  int port,
  char *ip_addr,
  task_id_t task_id);

int
udp_eNB_send_to(
  int sd,
  uint16_t port,
  uint32_t address,
  const uint8_t *buffer,
  uint32_t length);

void udp_eNB_receiver(struct udp_socket_desc_s *udp_sock_pP);

void *udp_eNB_task(void *args_p);

int udp_enb_init(void);
/* @brief Retrieve the descriptor associated with the task_id
 */
static
struct udp_socket_desc_s *udp_eNB_get_socket_desc(task_id_t task_id)
{
  struct udp_socket_desc_s *udp_sock_p = NULL;

#if defined(LOG_UDP) && LOG_UDP > 0
  LOG_T(UDP_, "Looking for task %d\n", task_id);
#endif

  STAILQ_FOREACH(udp_sock_p, &udp_socket_list, entries) {
    if (udp_sock_p->task_id == task_id) {
#if defined(LOG_UDP) && LOG_UDP > 0
      LOG_T(UDP_, "Found matching task desc\n");
#endif
      break;
    }
  }
  return udp_sock_p;
}

void udp_eNB_process_file_descriptors(struct epoll_event *events, int nb_events)
{
  int                       i;
  struct udp_socket_desc_s *udp_sock_p = NULL;

  if (events == NULL) {
    return;
  }

  for (i = 0; i < nb_events; i++) {
    STAILQ_FOREACH(udp_sock_p, &udp_socket_list, entries) {
      if (udp_sock_p->sd == events[i].data.fd) {
#if defined(LOG_UDP) && LOG_UDP > 0
        LOG_D(UDP_, "Found matching task desc\n");
#endif
        udp_eNB_receiver(udp_sock_p);
        break;
      }
    }
  }
}

static
int udp_eNB_create_socket(int port, char *ip_addr, task_id_t task_id)
{

  struct udp_socket_desc_s  *udp_socket_desc_p = NULL;
  int                       sd, rc;
  struct sockaddr_in        sin;

  LOG_I(UDP_, "Initializing UDP for local address %s with port %d\n", ip_addr, port);

  sd = socket(AF_INET, SOCK_DGRAM, 0);
  AssertFatal(sd > 0, "UDP: Failed to create new socket: (%s:%d)\n", strerror(errno), errno);

  memset(&sin, 0, sizeof(struct sockaddr_in));
  sin.sin_family      = AF_INET;
  sin.sin_port        = htons(port);

  if (ip_addr == NULL) {
    sin.sin_addr.s_addr = inet_addr(INADDR_ANY);
  } else {
    sin.sin_addr.s_addr = inet_addr(ip_addr);
  }

  if ((rc = bind(sd, (struct sockaddr *)&sin, sizeof(struct sockaddr_in))) < 0) {
    close(sd);
    LOG_E(UDP_, "Failed to bind socket: (%s:%d) address %s port %d\n", strerror(errno), errno, ip_addr, port);
    return -1;
  }

  /* Create a new descriptor for this connection */
  udp_socket_desc_p = calloc(1, sizeof(struct udp_socket_desc_s));

  DevAssert(udp_socket_desc_p != NULL);

  udp_socket_desc_p->sd            = sd;
  udp_socket_desc_p->local_address = ip_addr;
  udp_socket_desc_p->local_port    = port;
  udp_socket_desc_p->task_id       = task_id;

  LOG_I(UDP_, "Inserting new descriptor for task %d, sd %d\n", udp_socket_desc_p->task_id, udp_socket_desc_p->sd);
  pthread_mutex_lock(&udp_socket_list_mutex);
  STAILQ_INSERT_TAIL(&udp_socket_list, udp_socket_desc_p, entries);
  pthread_mutex_unlock(&udp_socket_list_mutex);

  itti_subscribe_event_fd(TASK_UDP, sd);
  LOG_I(UDP_, "Initializing UDP for local address %s with port %d: DONE\n", ip_addr, port);
  return sd;
}

int
udp_eNB_send_to(
  int sd,
  uint16_t port,
  uint32_t address,
  const uint8_t *buffer,
  uint32_t length)
{
  struct sockaddr_in to;
  socklen_t          to_length;

  if (sd <= 0 || ((buffer == NULL) && (length > 0))) {
    LOG_E(UDP_, "udp_send_to: bad param\n");
    return -1;
  }

  memset(&to, 0, sizeof(struct sockaddr_in));
  to_length = sizeof(to);

  to.sin_family      = AF_INET;
  to.sin_port        = htons(port);
  to.sin_addr.s_addr = address;

  if (sendto(sd, (void *)buffer, (size_t)length, 0, (struct sockaddr *)&to,
             to_length) < 0) {
    LOG_E(UDP_,
          "[SD %d] Failed to send data to "IPV4_ADDR" on port %d, buffer size %u\n",
          sd, IPV4_ADDR_FORMAT(address), port, length);
    return -1;
  }

#if defined(LOG_UDP) && LOG_UDP > 0
  LOG_I(UDP_, "[SD %d] Successfully sent to "IPV4_ADDR
        " on port %d, buffer size %u, buffer address %x\n",
        sd, IPV4_ADDR_FORMAT(address), port, length, buffer);
#endif
  return 0;
}


void udp_eNB_receiver(struct udp_socket_desc_s *udp_sock_pP)
{
  uint8_t                   l_buffer[2048];
  int                n;
  socklen_t          from_len;
  struct sockaddr_in addr;
  MessageDef               *message_p        = NULL;
  udp_data_ind_t           *udp_data_ind_p   = NULL;
  uint8_t                  *forwarded_buffer = NULL;

  if (1) {
    from_len = (socklen_t)sizeof(struct sockaddr_in);

    if ((n = recvfrom(udp_sock_pP->sd, l_buffer, sizeof(l_buffer), 0,
                      (struct sockaddr *)&addr, &from_len)) < 0) {
      LOG_E(UDP_, "Recvfrom failed %s\n", strerror(errno));
      return;
    } else if (n == 0) {
      LOG_W(UDP_, "Recvfrom returned 0\n");
      return;
    } else {
      forwarded_buffer = itti_malloc(TASK_UDP, udp_sock_pP->task_id, n*sizeof(uint8_t));
      DevAssert(forwarded_buffer != NULL);
      memcpy(forwarded_buffer, l_buffer, n);
      message_p = itti_alloc_new_message(TASK_UDP, 0, UDP_DATA_IND);
      DevAssert(message_p != NULL);
      udp_data_ind_p = &message_p->ittiMsg.udp_data_ind;
      udp_data_ind_p->buffer        = forwarded_buffer;
      udp_data_ind_p->buffer_length = n;
      udp_data_ind_p->peer_port     = htons(addr.sin_port);
      udp_data_ind_p->peer_address  = addr.sin_addr.s_addr;

      /////////测量
      measure_packet((char *)&udp_data_ind_p->buffer[8], 
                      &recvSet, sock, &recv_mutex, &recv_elastic_sketch);
      /////////////
      // 调用接收程序丢弃时间戳数据包
      int flag = delay_measure_recv(udp_data_ind_p, message_p, forwarded_buffer, &recvSet);
      if(flag){
        return;
      }

#if defined(LOG_UDP) && LOG_UDP > 0
      LOG_I(UDP_, "Msg of length %d received from %s:%u\n",
            n, inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
#endif

      /* TODO: this is a hack. Let's accept failures and do nothing when
       * it happens. Since itti_send_msg_to_task crashes when the message
       * queue is full we wrote itti_try_send_msg_to_task that returns -1
       * if the queue is full.
       */
      /* look for HACK_RLC_UM_LIMIT for others places related to the hack. Please do not remove this comment. */
      if (itti_send_msg_to_task(udp_sock_pP->task_id, INSTANCE_DEFAULT, message_p) < 0) {
        LOG_E(UDP_, "Failed to send message %d to task %d\n",
              UDP_DATA_IND,
              udp_sock_pP->task_id);
        itti_free(TASK_UDP, message_p);
        itti_free(TASK_UDP, forwarded_buffer);
        return;
      }
    }
  }

  //close(udp_sock_p->sd);
  //udp_sock_p->sd = -1;

  //pthread_mutex_lock(&udp_socket_list_mutex);
  //STAILQ_REMOVE(&udp_socket_list, udp_sock_p, udp_socket_desc_s, entries);
  //pthread_mutex_unlock(&udp_socket_list_mutex);
}

/*
 * 将数据包复制一份然后添加时间戳信息
 */
uint64_t send_insert_timestamp(udp_data_req_t *udp_data_req_p) {
  uint8_t time_flag;      // ip头部服务类型的保留位 标志位 表示这个包里面是时间戳信息
  uint16_t udp_length;    // 数据改为时间戳后的udp头部和数据部分的总的长度 
  uint16_t ip_length;    // 数据改为时间戳后的ip头部和数据部分的总的长度 
  uint16_t ip_header[10];  // 提取出来的ip头部 用于计算新的校验和 原始 uint8_t ip_header[20]
  uint16_t new_checksum;  // 计算出来的新的校验和
  // 时间戳信息
  uint64_t current_millisecond;
  // 定义变量time_stamp_count：表示存放的时间戳数量 每次运行+1
  uint8_t time_stamp_count = 1;

  // 随机概率选择需要存储当前的流信息 复制当前流
  // 发送时间戳的函数
  // 将ip的头部TOS的保留位设置为1(默认为0)表示这是一个时间戳的数据包 TOS 在第8位到16位公8位一个字节，最后一位16位为保留位
  time_flag = udp_data_req_p->buffer[udp_data_req_p->buffer_offset + 9];
  LOG_D(UDP_, "before time_flag: %x\n", time_flag);
  time_flag = time_flag | 0x01;   // 把最后一位保留位变成1
  LOG_D(UDP_, "after time_flag: %x\n", time_flag);

  // 获取当前的时间戳信息
  current_millisecond = getTimeUsec();
  LOG_D(UDP_, "current_millisecond: %lx\n", current_millisecond);

  // 把时间戳的流标志位放到数据包中
  memcpy(&udp_data_req_p->buffer[udp_data_req_p->buffer_offset] + 9, &time_flag, 1);
  LOG_D(UDP_, "udp_data_req_p->buffer[udp_data_req_p->buffer_offset + 9]: %x\n", udp_data_req_p->buffer[udp_data_req_p->buffer_offset + 9]);

  // 把当前基站id和总的时间戳个数添加到数据部分
  memcpy(&udp_data_req_p->buffer[udp_data_req_p->buffer_offset] + 36, &time_stamp_count, 1);  // 添加时间戳个数
  memcpy(&udp_data_req_p->buffer[udp_data_req_p->buffer_offset] + 37, &curr_eNB_id, 1);  // 添加eNB设备id
  memcpy(&udp_data_req_p->buffer[udp_data_req_p->buffer_offset] + 38, &current_millisecond, 8); // 添加时间戳

  // 将 UDP 头部中数据长度的部分改成 8 + i * 9 + 1 = 9 + 9 * i 字节 偏移 8 + 20 + 4 = 32 字节 (8-GTP   20-IP)
  udp_length = (uint16_t)(time_stamp_count * 9 + 9);
  udp_length = (udp_length >> 8) | (udp_length << 8);   // 更改大小端的操作
  memcpy(&udp_data_req_p->buffer[udp_data_req_p->buffer_offset] + 32, &udp_length, 2);

  // 将 IP 头部中数据长度的部分改成20 + 8 + i * 9 + 1 = 29 + 9 * i字节(其中i为传输数据第一个字节：表示总共有多少个时间戳数据) 偏移8 + 2 = 10 （8-GTP长度）
  ip_length = (uint16_t)(time_stamp_count * 9 + 29);
  ip_length = (ip_length >> 8) | (ip_length << 8); 
  memcpy(&udp_data_req_p->buffer[udp_data_req_p->buffer_offset] + 10, &ip_length, 2);

  // 重新计算ip头部校验和
  memcpy(&ip_header, &udp_data_req_p->buffer[udp_data_req_p->buffer_offset] + 8, 20);
  // 将原来的校验和变为00
  ip_header[5] = 0x0000;
  // 使用check_ip_sum计算校验和 函数在最前面
  new_checksum = check_ip_sum(ip_header, 20);
  // 将新的校验和放到ip头部
  memcpy(&udp_data_req_p->buffer[udp_data_req_p->buffer_offset] + 18, &new_checksum, 2);

  return current_millisecond;
}

/*
 * 发送复制后的数据包
 */
int delay_measure_send(udp_data_req_t *udp_data_req_p,  
                  MyHashSet *sendSet,struct sockaddr * peer_addr, int udp_sd){
        // 解析ip数据
        int flag = -1;
        ssize_t send_timestamp; // 发送时间戳后返回的标志位
        int is_ipv4_packet;
        packet_key_t packet_key;
        uint8_t flow_key[13]={'0'}; // 存五元组

        is_ipv4_packet = extract_packet_key((uint8_t *)(&udp_data_req_p->buffer[udp_data_req_p->buffer_offset] + 8), &packet_key);  // is_ipv4_packet = 0; 表示ipv4
        if (is_ipv4_packet == 0) {
          packet_key_to_char(&packet_key, flow_key);
          // 判断是否插入哈希表
          flag = myHashSetAddSamplingData(sendSet, flow_key);
        }

        // 如果flag的话就插入
        uint64_t current_millisecond;
        if (flag == 1) {
          current_millisecond = send_insert_timestamp(udp_data_req_p);
        }
        // 将复制的数据包发送出去
        send_timestamp = sendto(
                  udp_sd,
                  &udp_data_req_p->buffer[udp_data_req_p->buffer_offset],   // (const char*)current_time
                  sizeof(current_millisecond) + 38,
                  0,
                  (struct sockaddr *)peer_addr,
                  sizeof(struct sockaddr_in));

        // 发送失败打印消息
        // LOG_D(UDP_, sizeof(current_time));
        // LOG_D(UDP_, "sizeof(current_time): %d, send_timestamp: %zd\n", sizeof(current_time), send_timestamp);
        LOG_D(UDP_, "send_timestamp: %zd\n", send_timestamp);
        if (send_timestamp != sizeof(current_millisecond) + 38) {
          LOG_E(UDP_, "Sending current time: %ld failed! send_timestamp: %zd\n", current_millisecond, send_timestamp);
        }


        // 已经存完了表之后
        MyHashSetIterator itt;
        MyHashSetIterator * it = &itt;
        it->index = 0;
        it->set = sendSet;
        it->current = sendSet->dataList[0]->first;
        it->count = 0;
        int x = 0;
        while(myHashSetIteratorHasNext(it)) {
            x++;

            // printf("begin get node \n\n\n");
            MyNode *node = myHashSetIteratorNext(it);

        //        uint8_t *flow_key = node->data;
            printf("\n%ld   \n", node->samplingData.lastSamplingTime);
        }
        return 0;
}

/*
 * 接收复制的数据包 
 */
int delay_measure_recv(udp_data_ind_t *udp_data_ind_p, MessageDef *message_p,
                      uint8_t *forwarded_buffer, MyHashSet *recvSet){
  uint64_t current_millisecond;  // 当前时间
  // uint64_t *p = NULL;
  // 定义ip地址的变量指针
  // uint8_t src_ip[4];
  // uint8_t dst_ip[4];
  // uint8_t protocol_type;
  // uint16_t src_port, dst_port;

  // 定义ip头部的结构体
  packet_key_t packet_key;
  int is_ipv4_packet;
  uint8_t flow_key[13]={'0'}; // 存五元组


  // 判断标志位 是否是复制的数据包
  if ((udp_data_ind_p->buffer[9] & 0x01) == 0x01) {

    // 获取当前时间
    current_millisecond = getTimeUsec();
    printf("current_millisecond : %ld\n", current_millisecond);

    // 找到有多少个节点的时间戳信息
    int time_count = (uint8_t)(udp_data_ind_p->buffer[36]);

    // 解析ip数据
    DelayData * dData = (DelayData *) malloc(sizeof(DelayData));
    memset(dData, 0, sizeof(DelayData));

    // 循环读取时间戳
    for (int i = 0; i < time_count - 1; i++) {
      LinkDelay linkDelay;
      linkDelay.startNode = (uint8_t)(udp_data_ind_p->buffer[37 + 9 * i]);
      linkDelay.endNode = (uint8_t)(udp_data_ind_p->buffer[37 + 9 * (i + 1)]);
      linkDelay.delay = *(uint64_t *)(&udp_data_ind_p->buffer[38 + 9 * (i + 1)]) - *(uint64_t *)(&udp_data_ind_p->buffer[38 + 9 * i]);

      dData->links[i] = linkDelay;
    }
    // 单独处理最后一个节点
    LinkDelay linkDelay;
    linkDelay.startNode = (uint8_t)(udp_data_ind_p->buffer[37 + 9 * (time_count - 1)]);
    linkDelay.endNode = curr_eNB_id;
    linkDelay.delay = current_millisecond - *(uint64_t *)(&udp_data_ind_p->buffer[38 + 9 * (time_count - 1)]);
    dData->links[time_count - 1] = linkDelay;

    // 计算总的端到端的时延
    dData->NodeToNodeDelay = current_millisecond - *(uint64_t *)(&udp_data_ind_p->buffer[38]);

    is_ipv4_packet = extract_packet_key((uint8_t *)(&udp_data_ind_p->buffer[8]), &packet_key);  // is_ipv4_packet = 0; 表示ipv4
    printf("\n is_ipv4_packet : %d\n", is_ipv4_packet);

    // 如果是ipv4的包
    if (is_ipv4_packet == 0) {
      packet_key_to_char(&packet_key, &flow_key);   
      myHashSetAddDelayData(recvSet, flow_key, dData);

      printf("\n");
      MyHashSetIterator itt;
      MyHashSetIterator * it = &itt;
      it->index = 0;
      it->set = recvSet;
      it->current = recvSet->dataList[0]->first;
      it->count = 0;
      int x = 0;
      while(myHashSetIteratorHasNext(it)) {
          x++;

          // printf("begin get node \n\n\n");
          MyNode *node = myHashSetIteratorNext(it);

  //        uint8_t *flow_key = node->data;
          printf("%d   %d   %d   %d\n", node->delayInfo->NodeToNodeDelay,node->delayInfo->links[0].delay,
                  node->delayInfo->links[0].startNode,node->delayInfo->links[0].endNode);
      }
      printf("\n");
    }
    
    // 释放内存
    LOG_W(UDP_, "Drop packets\n");
    itti_free(TASK_UDP, message_p);
    itti_free(TASK_UDP, forwarded_buffer);
    return 1;
  }
  else return 0;

}

void *udp_eNB_task(void *args_p)
{
  int                 nb_events;
  struct epoll_event *events;
  MessageDef         *received_message_p    = NULL;
  //const char         *msg_name = NULL;
  //instance_t          instance  = 0;
  udp_enb_init();

  itti_mark_task_ready(TASK_UDP);
  MSC_START_USE();

  // 创建表
  // MyHashSet sendSet;
  // initHashSet(myHashCodeString, myEqualString, &sendSet);
  // initHashSet(myHashCodeString, myEqualString, &recvSet); // 接收表
  // 数据初始化
    Init_ElasticSketch(&recv_elastic_sketch, BUCKET_NUM, LIGHT_PART_COUNTER_NUM);
    initHashSet(myHashCodeString, myEqualString, &recvSet);
    sock = socket(AF_INET, SOCK_STREAM, 0);
    // measure_timer_create(5, &recvSet, &recv_elastic_sketch,&recv_mutex,sock, 1);
    // MyHashSet* sendSet = &Set;

    Init_ElasticSketch(&send_elastic_sketch, BUCKET_NUM, LIGHT_PART_COUNTER_NUM);
    initHashSet(myHashCodeString, myEqualString, &sendSet);
    // measure_timer_create(5, &sendSet, &send_elastic_sketch,&send_mutex,sock, 0);
    measure_timer_create(5, &recvSet, &recv_elastic_sketch,&recv_mutex,
                        &sendSet, &send_elastic_sketch,&send_mutex,
                        sock);
    // MyHashSet* sendSet = &Set;
    // measure_timer_create(5, &Set, &elastic_sketch,&mutex,sock);
  
  
  
  // 定义ip头部的结构体
  packet_key_t packet_key;
  int is_ipv4_packet;
  uint8_t flow_key[13]={'0'}; // 存五元组
  // 当前enb id
  curr_eNB_id = 101;
  
  while(1) {
    itti_receive_msg(TASK_UDP, &received_message_p);
    VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_UDP_ENB_TASK, VCD_FUNCTION_IN);
#if defined(LOG_UDP) && LOG_UDP > 0
    LOG_D(UDP_, "Got message %p\n", &received_message_p);
#endif

    if (received_message_p != NULL) {

      //msg_name = ITTI_MSG_NAME (received_message_p);
      //instance = ITTI_MSG_DESTINATION_INSTANCE (received_message_p);

      switch (ITTI_MSG_ID(received_message_p)) {
      case UDP_INIT: {
        LOG_D(UDP_, "Received UDP_INIT\n");
        udp_init_t *udp_init_p;
        udp_init_p = &received_message_p->ittiMsg.udp_init;
        udp_eNB_create_socket(
          udp_init_p->port,
          udp_init_p->address,
          ITTI_MSG_ORIGIN_ID(received_message_p));
      }
      break;

      case UDP_DATA_REQ: {
#if defined(LOG_UDP) && LOG_UDP > 0
        LOG_D(UDP_, "Received UDP_DATA_REQ\n");
#endif
        int     udp_sd = -1;
        ssize_t bytes_written;

        struct udp_socket_desc_s *udp_sock_p = NULL;
        udp_data_req_t           *udp_data_req_p;
        struct sockaddr_in        peer_addr;

        udp_data_req_p = &received_message_p->ittiMsg.udp_data_req;

        memset(&peer_addr, 0, sizeof(struct sockaddr_in));

        peer_addr.sin_family       = AF_INET;
        peer_addr.sin_port         = htons(udp_data_req_p->peer_port);
        peer_addr.sin_addr.s_addr  = udp_data_req_p->peer_address;

        pthread_mutex_lock(&udp_socket_list_mutex);
        udp_sock_p = udp_eNB_get_socket_desc(ITTI_MSG_ORIGIN_ID(received_message_p));

        if (udp_sock_p == NULL) {
          LOG_E(UDP_,
                "Failed to retrieve the udp socket descriptor "
                "associated with task %d\n",
                ITTI_MSG_ORIGIN_ID(received_message_p));
          pthread_mutex_unlock(&udp_socket_list_mutex);

          if (udp_data_req_p->buffer) {
            itti_free(ITTI_MSG_ORIGIN_ID(received_message_p), udp_data_req_p->buffer);
          }

          goto on_error;
        }

        udp_sd = udp_sock_p->sd;
        pthread_mutex_unlock(&udp_socket_list_mutex);

//#if defined(LOG_UDP) && LOG_UDP > 0
        LOG_D(UDP_, "[%d] Sending message of size %u to "IPV4_ADDR" and port %u\n",
              udp_sd,
              udp_data_req_p->buffer_length,
              IPV4_ADDR_FORMAT(udp_data_req_p->peer_address),
              udp_data_req_p->peer_port);
//#endif
        bytes_written = sendto(
                          udp_sd,
                          &udp_data_req_p->buffer[udp_data_req_p->buffer_offset],
                          udp_data_req_p->buffer_length,
                          0,
                          (struct sockaddr *)&peer_addr,
                          sizeof(struct sockaddr_in));

        if (bytes_written != udp_data_req_p->buffer_length) {
          LOG_E(UDP_, "There was an error while writing to socket %d rc %zd"
                "(%d:%s) May be normal if GTPU kernel module loaded on same host (may NF_DROP IP packet)\n",
                udp_sd, bytes_written, errno, strerror(errno));
        }

        // 插入数据
        measure_packet((uint8_t *)(&udp_data_req_p->buffer[udp_data_req_p->buffer_offset] + 8), 
                        &sendSet, sock, &send_mutex, &send_elastic_sketch);
        // 调用复制数据包的程序发送复制后的数据包
        delay_measure_send(udp_data_req_p, &sendSet,(struct sockaddr *)&peer_addr, udp_sd);
		
        itti_free(ITTI_MSG_ORIGIN_ID(received_message_p), udp_data_req_p->buffer);
      }
      break;

      case TERMINATE_MESSAGE: {
        LOG_W(UDP_, " *** Exiting UDP thread\n");
        itti_exit_task();
      }
      break;

      case MESSAGE_TEST: {
      } break;

      default: {
        LOG_W(UDP_, "Unkwnon message ID %d:%s\n",
              ITTI_MSG_ID(received_message_p),
              ITTI_MSG_NAME(received_message_p));
      }
      break;
      }

on_error:
      itti_free (ITTI_MSG_ORIGIN_ID(received_message_p), received_message_p);
      received_message_p = NULL;
    }

    nb_events = itti_get_events(TASK_UDP, &events);

    /* Now handle notifications for other sockets */
    if (nb_events > 0) {
#if defined(LOG_UDP) && LOG_UDP > 0
      LOG_D(UDP_, "UDP task Process %d events\n",nb_events);
#endif
      udp_eNB_process_file_descriptors(events, nb_events);
    }

    VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_UDP_ENB_TASK, VCD_FUNCTION_OUT);
  }

  LOG_I(UDP_, "Task UDP eNB exiting\n");
  return NULL;
}

int udp_enb_init(void)
{
  LOG_I(UDP_, "Initializing UDP task interface\n");
  STAILQ_INIT(&udp_socket_list);
  LOG_I(UDP_, "Initializing UDP task interface: DONE\n");
  return 0;
}
