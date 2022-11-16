import os
from socket import *
import time
from threading import Thread
from sys import *
from concurrent.futures import *

multi = int(argv[1]) if (len(argv) >= 2) else 1
ip = argv[2] if (len(argv) >= 3) else '127.0.0.2'
port = int(argv[3]) if (len(argv) >= 4) else 10000
type = int(argv[4]) if (len(argv) >= 5) else 3

def start_udp_client(filename, i):
    print()
    HOST = '127.0.0.2'  # or 'localhost'
    PORT = 12344
    global port
    global ip
    dest = (ip, port)
    # dest = ('127.0.0.1', 10000)
    start = time.time_ns()
    # print(start)
    send_msg = []
    flowDict = dict()
    cliSockList = dict()
    now = time.perf_counter()
    # port = 0
    with open(filename, "r") as f:
        for line in f.readlines():
            con = line.split()
            con[0] = int(float(con[0]))  # 间隔
            con[1] = int(con[1])  # 长度
            con[3] = int(con[3])  # 编号
            send_msg.append(con)
            if con[3] not in flowDict:
                flowDict[con[3]] = con[3] + 40000 + i * 1000
    for k, v in flowDict.items():
        udpCliSock = socket(AF_INET, SOCK_DGRAM)
        udpCliSock.bind(('127.0.0.3', v))

        cliSockList[k] = udpCliSock
    delay = 0
    last = time.time_ns()
    last1 = time.time_ns()
    index = 0
    tt = 0
    while True:
        now = time.time_ns()
        if (now - last) >= delay:
            msg = send_msg[index]
            data = '2' * msg[1]
            tt += int(msg[0])

            delay = int(msg[0]) // multi
            # print(delay)
            udpCliSock = cliSockList[msg[3]]
            udpCliSock.sendto(data.encode(), dest)
            last = now
            index += 1
        if index >= len(send_msg): break
        # if (now - start)>1E10:break;
    print(len(send_msg))
    print(tt)
    print(index)
    now1 = time.time_ns()
    # print(time.get_clock_info('perf_counter'))
    print(filename + "用时:" + str((now1 - last1) / 1E9))


def start_udp_client_single_flow(filename, i):
    # print("123")
    global port
    global ip
    dest = (ip, port)
    send_msg = []
    flowDict = dict()
    cliSockList = dict()

    with open(filename, "r") as f:
        for line in f.readlines():
            con = line.split()
            con[0] = int(float(con[0]))  # 间隔
            con[1] = int(con[1])  # 长度
            con[3] = int(con[3])  # 编号
            send_msg.append(con)
            if con[3] not in flowDict:
                flowDict[con[3]] = 40000 + i
    udpCliSock = socket(AF_INET, SOCK_DGRAM)
    udpCliSock.bind(('127.0.0.3', 40000 + i))

    delay = 0
    last = time.time_ns()
    last1 = time.time_ns()
    index = 0
    tt = 0
    # print(len(send_msg))
    while True:
        now = time.time_ns()
        if (now - last) >= delay:
            msg = send_msg[index]
            data = '2' * msg[1]
            tt += int(msg[0])

            if multi != 0:
                delay = int(msg[0]) // multi
            # print(delay)
            udpCliSock.sendto(data.encode(), dest)
            last = now
            index += 1
        if index >= len(send_msg): break
        # if (now - start)>1E10:break;
    print(len(send_msg))
    # print(tt)
    # print(index)
    now1 = time.time_ns()
    # print(time.get_clock_info('perf_counter'))
    print(filename + "用时:" + str((now1 - last1) / 1E9))


if __name__ == '__main__':
    dir = "./pkt/"
    index = 0
    last = time.time_ns()

    start_udp_client_single_flow("./pkt/delay/delay.pkts",1)
    now = time.time_ns()
    # time.sleep(2)
    print("发送完成")
    print("用时:" + str((now - last) / 1E9))
# start_udp_client()
