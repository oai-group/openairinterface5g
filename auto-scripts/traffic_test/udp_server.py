import socket
import sys
import threading
import time


def main():
    # 获取第一个参数 IP
    server_ip = str(sys.argv[1])
    # 获取第二个参数 端口
    server_port = int(sys.argv[2])
    # 获取第三个参数 停止时间
    # running_time = float(sys.argv[3]) + 3.0

    # 1.found
    udp_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    # 设置端口复用
    udp_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, True)

    # 2.bind
    udp_socket.bind((server_ip, server_port))

    # 监听个数
    # udp_socket.listen(128)
    # begin_time = time.time()

    while True:
        # accept
        rev_data, ip_port = udp_socket.recvfrom(1024)

        print(ip_port, ":", rev_data.decode("utf-8"))

    # resource
    # udp_socket.close()


if __name__ == '__main__':
    main()
