#!/usr/bin/env bash

# 服务器的IP地址
server_ip=172.16.0.4
# UDP端口
server_udp_port=40000
# TCP端口
server_tcp_port=50000
# 测试时间/s 这里尽量长一些 相比客户端来说
running_time=3600

sudo lsof -i:${server_udp_port},${server_tcp_port}|awk 'NR!=1{print $2}'|xargs sudo kill -9 >/dev/null 2>&1

echo "UDP server starting"
# gnome-terminal --window -x 
timeout -k 2s ${running_time}s python3 -u udp_server.py ${server_ip} ${server_udp_port} &

echo "TCP Server starting"
# gnome-terminal --window -x
timeout -k 2s ${running_time}s python3 -u tcp_server.py ${server_ip} ${server_tcp_port} &

# wait

# 杀死上面产生的进程
echo "Ending test"
