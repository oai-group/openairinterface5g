#include <stdio.h>
typedef unsigned short  uint16_t;  
        
printf("index\t\t\t\tSrc IP\t\t\t\tDst IP\t\t\t\tProtocolt\t\tSrc port\t\tDst portt\t\tTotal Byte\t\tTotal Packet");

unsigned short int flow_key[13] ={1}
printf("%3d :   ", x);
printf("    %3d.%3d.%3d.%3d   ", flow_key[0],
                                    flow_key[1],
                                    flow_key[2],
                                    flow_key[3]);
printf("    %3d.%3d.%3d.%3d   ", flow_key[4],
                                    flow_key[5],
                                    flow_key[6],
                                    flow_key[7]);
if (flow_key[12] == 6){
    printf("  TCP  ");
}else if (flow_key[12] == 17){
    printf("  UDP  ");
}else{
    printf("  %3d ",flow_key[12]);
}
printf("    %5d",htons(*((uint16_t*)&(flow_key[8]))));   
printf("    %5d",htons(*((uint16_t*)&(flow_key[10]))));
printf(" Total Byte: %10u   Total Packet: %5u\n", 100000,100);