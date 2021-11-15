#ifndef _MEASURE_PARSER_H
#define _MEASURE_PARSER_H

#include "openair2/COMMON/commonDef.h"
#include "netinet/in.h"

#define IP_VERSION_4 4
#define IP_VERSION_6 6

//IP头各信息的偏移
#define VERSION_AND_HEADER_LEN_OFFSET 0
#define DS_FIELD_OFFSET 1
#define TOTAL_LEN_OFFSET 2
#define IDENTIFICATION_OFFSET 4
#define FLAGS_AND_FRAGMENT_OFFSET_OFFSET 6
#define TTL_OFFSET 8
#define PROTOCOL_OFFSET 9
#define HEADER_CHECKSUM_OFFSET 10
#define SRC_IP_OFFSET 12
#define DST_IP_OFFSET 16

//传输层协议头偏移
#define SRC_PORT_OFFSET 20
#define DST_PORT_OFFSET 22

//显示IP
#define FIRST_IP_PART(ip) ((unsigned char) ((ip >> 24) & 0xff))
#define SECOND_IP_PART(ip) ((unsigned char) ((ip >> 16) & 0xff))
#define THIRD_IP_PART(ip) ((unsigned char) ((ip >> 8) & 0xff))
#define LAST_IP_PART(ip) ((unsigned char) (ip & 0xff))

//
#define TCP_PROTOCOL_NUM 6
#define UDP_PROTOCOL_NUM 17
#define UDP_HEADER_LEN 8
#define TCP_HEADER_LEN_OFFSET 32


typedef struct IP_header_t
{
    //version实际上占4位
    uint8_t version;
    //head_len实际上占4位
    uint8_t header_len;
    uint8_t DS_field;
    uint16_t total_len;
    uint16_t identification;
    //flags实际上占3位
    uint8_t flags;
    //fragment_offset实际上占13位
    uint16_t fragment_offset; 
    uint8_t ttl;
    uint8_t protocol;
    uint16_t header_checksum;
    uint32_t src_ip;
    uint32_t dst_ip;
}IP_header_t;

//传输层结构体
typedef struct trans_header_t
{
    uint16_t src_port;
    uint16_t dst_port;
    //其余忽略
}trans_header_t;

//五元组
typedef struct packet_key_t
{
    uint32_t src_ip;
    uint32_t dst_ip;
    uint8_t protocol;
    uint16_t src_port;
    uint16_t dst_port;

    uint16_t packet_len;
}packet_key_t;




//从PDCP_IND报文中提取IP头部信息，存储到IP_header结构体中
//考虑字节序，高于8位的，需要使用htons或者htonl进行转换
int IP_header_parser(char* packet, IP_header_t* header);
int trans_header_parser(char* packet, trans_header_t* header);
int extract_packet_key(char* packet, packet_key_t* key);
void packet_key_to_char(packet_key_t* packey_key, uint8_t* five_tuple);


#endif