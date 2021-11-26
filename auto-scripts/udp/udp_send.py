#coding=utf-8
 
from socket import *
import sys
import random
import time

# 获取第一个参数 
delay_time = float(sys.argv[1])

# 获取第二个参数 总的时间
running_time = int(sys.argv[2])

# 1. 创建udp套接字
udp_socket = socket(AF_INET, SOCK_DGRAM)
udp_socket.bind(("",8877))

# 2. 准备接收方的地址
# '192.168.1.103'表示目的ip地址
# 8080表示目的端口
dest_addr = ('172.16.0.4', 9998)  # 注意 是元组，ip是字符串，端口是数字
 
# 3. 从键盘获取数据

while True:
    send_data = str(random.randint(100, 10000000000)) 
    udp_socket.sendto(send_data.encode('utf-8'), dest_addr)
    time.sleep(delay_time)
    running_time = running_time - delay_time
    # 判断是否结束执行
    if running_time <= 0:
        break

# 5. 关闭套接字
udp_socket.close()
