# coding=utf-8

from socket import *
import sys
import random
import time

BASE_DATA = "abcdefghijklmnopqrstuvwxyz" \
            "ABCDEFGHIJKLMNOPQRSTUVWZYX" \
            "1234567890"


def main():
    # 获取第一个参数 IP
    server_ip = str(sys.argv[1])
    # 获取第二个参数 端口
    server_port = int(sys.argv[2])
    # 获取第三个参数 间隔发包时间
    delay_time = float(sys.argv[3])
    # 获取第四个参数 总的时间
    running_time = float(sys.argv[4])

    # 1. 创建udp套接字
    udp_socket = socket(AF_INET, SOCK_DGRAM)
    # udp_socket.bind(("", 50000))

    # 2. 准备接收方的地址
    # '192.168.1.103'表示目的ip地址
    # 8080表示目的端口
    dst_address = (server_ip, server_port)  # 注意 是元组，ip是字符串，端口是数字

    # 3. 从键盘获取数据

    while True:
        data_size = random.randint(1, 1400)
        send_data = ""
        for i in range(data_size):
            send_data = send_data + str(random.choice(BASE_DATA))
        udp_socket.sendto(send_data.encode('utf-8'), dst_address)

        time.sleep(delay_time)
        running_time = running_time - delay_time
        # 判断是否结束执行
        if running_time <= 0:
            break

    # 5. 关闭套接字
    udp_socket.close()


if __name__ == '__main__':
    main()
