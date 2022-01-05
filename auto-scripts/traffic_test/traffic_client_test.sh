#!/usr/bin/env bash

# 服务器的IP地址
server_ip=172.16.0.4
# UDP端口
server_udp_port=40000
# TCP端口
server_tcp_port=50000

# 间隔时间
delay_time=1
# 发送的总时间
running_time=5
# 测试的流的个数
flow_nums=5

echo "TCP/UDP client starting"
# shellcheck disable=SC2051
for ((i=0; i < flow_nums; i++))
do
  echo "send tcp flow no "${i}
  # 启动tcp client
  python3 tcp_client.py ${server_ip} ${server_tcp_port} ${delay_time} ${running_time} &

  echo "send udp flow no "${i}
  # 启动udp client
  python3 udp_client.py ${server_ip} ${server_udp_port} ${delay_time} ${running_time} &
done


wait

# 杀死上面产生的进程
echo "Ending test"
