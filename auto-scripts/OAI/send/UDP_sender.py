import os
from socket import *
import time
from threading import Thread
from sys import *
from concurrent.futures import *

multi = int(argv[1]) if (len(argv) >= 2) else 0
ip = argv[2] if (len(argv) >= 3) else '127.0.0.2'
port = int(argv[3]) if (len(argv) >= 4) else 10000
type = int(argv[4]) if (len(argv) >= 5) else 4

# def start_udp_client(filename, i):
#     print()
#     HOST = '127.0.0.2'  # or 'localhost'
#     PORT = 12344
#     global port
#     global ip
#     dest = (ip, port)
#     # dest = ('127.0.0.1', 10000)
#     start = time.time_ns()
#     # print(start)
#     send_msg = []
#     flowDict = dict()
#     cliSockList = dict()
#     now = time.perf_counter()
#     # port = 0
#     with open(filename, "r") as f:
#         for line in f.readlines():
#             con = line.split()
#             con[0] = int(float(con[0]))  # 间隔
#             con[1] = int(con[1])  # 长度
#             con[3] = int(con[3])  # 编号
#             send_msg.append(con)
#             if con[3] not in flowDict:
#                 flowDict[con[3]] = con[3] + 40000 + i * 1000
#     for k, v in flowDict.items():
#         udpCliSock = socket(AF_INET, SOCK_DGRAM)
#         udpCliSock.bind(('172.16.0.2', v))
#
#         cliSockList[k] = udpCliSock
#     delay = 0
#     last = time.time_ns()
#     last1 = time.time_ns()
#     index = 0
#     tt = 0
#     while True:
#         now = time.time_ns()
#         if (now - last) >= delay:
#             msg = send_msg[index]
#             data = '2' * msg[1]
#             tt += int(msg[0])
#
#             delay = int(msg[0]) // multi
#             # print(delay)
#             udpCliSock = cliSockList[msg[3]]
#             udpCliSock.sendto(data.encode(), dest)
#             last = now
#             index += 1
#         if index >= len(send_msg): break
#         # if (now - start)>1E10:break;
#     print(len(send_msg))
#     print(tt)
#     print(index)
#     now1 = time.time_ns()
#     # print(time.get_clock_info('perf_counter'))
#     print(filename + "用时:" + str((now1 - last1) / 1E9))

xxx = 0
yyy = 0

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
            global yyy
            yyy += msg[1]
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
    global xxx
    xxx += len(send_msg)
    # print(tt)
    # print(index)
    now1 = time.time_ns()
    # print(time.get_clock_info('perf_counter'))
    # print(filename + "用时:" + str((now1 - last1) / 1E9))


if __name__ == '__main__':
    dir = "./pkt/"
    index = 0
    thread_list = []
    last = time.time_ns()
    filepath_list = []
    tasks = []
    for root, folder_names, file_names in os.walk(dir):
        for file_name in file_names:
            file_path = root + os.sep + file_name
            filepath_list.append(file_path)

    video_pool = ThreadPoolExecutor(max_workers=2)
    voip_pool = ThreadPoolExecutor(max_workers=10)
    iot_pool = ThreadPoolExecutor(max_workers=20)

    for file in filepath_list:
        print(file)
        if file.find("video") != -1 and (type==1 or type==4):
            idd = int(file.split("_")[-1].split(".")[0])

            # idd = file.split("_")[-1].split(".")[0]
            # print("idd"+idd)
            # print("find video" + file)
            # tasks.append(video_pool.submit(start_udp_client_single_flow, file, index))
            tasks.append(video_pool.submit(start_udp_client_single_flow, file, idd))
        if file.find("voip") != -1 and (type==2 or type==4):
            idd = int(file.split("_")[-1].split(".")[0])
            tasks.append(voip_pool.submit(start_udp_client_single_flow, file, 1000+idd))
        if file.find("iot") != -1 and (type==3 or type==4):
            idd = int(file.split("_")[-1].split(".")[0])
            tasks.append(iot_pool.submit(start_udp_client_single_flow, file, 2000+idd))
        index += 1
        #
        # thread = Thread(target=start_udp_client_single_flow, args=(file, index,))
        # thread.setDaemon(True)
        # thread.start()
        # thread_list.append(thread)
    # for t in thread_list:
    #     t.join()
    wait(tasks, return_when=ALL_COMPLETED)
    # video_pool.shutdown(wait=True)
    # voip_pool.shutdown(wait=True)
    # iot_pool.shutdown(wait=True)
    now = time.time_ns()
    # time.sleep(2)
    print(xxx)
    print(yyy)
    print("发送完成")
    print("用时:" + str((now - last) / 1E9))
# start_udp_client()
