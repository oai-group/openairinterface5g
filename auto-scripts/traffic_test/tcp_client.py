# tcp客户端
import random
import sys
import socket
import time

BASE_DATA = "abcdefghijklmnopqrstuvwxyz" \
            "ABCDEFGHIJKLMNOPQRSTUVWZYX" \
            "1234567890"


def main():
    # 获取第一个参数 IP
    server_ip = str(sys.argv[1])
    # 获取第二个参数 端口
    server_port = int(sys.argv[2])
    # 获取第三个参数
    delay_time = float(sys.argv[3])
    # 获取第四个参数 总的时间
    running_time = float(sys.argv[4])

    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    # 创建链接---发送链接请求
    s.connect((server_ip, server_port))

    while True:
        data_size = random.randint(1, 1400)
        send_data = ""
        for i in range(data_size):
            send_data = send_data + str(random.choice(BASE_DATA))

        s.send(send_data.encode('utf-8'))

        time.sleep(delay_time)
        running_time = running_time - delay_time
        # 判断是否结束执行
        if running_time <= 0:
            # 发送退出指令
            send_data = "tcp client will exit"
            s.send(send_data.encode('utf-8'))
            time.sleep(1)
            break

    # 5关闭套接字
    s.close()


if __name__ == '__main__':
    main()
