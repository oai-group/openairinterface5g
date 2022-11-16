from socket import *
from sys import *
from threading import Thread
from time import sleep

HOST = str(argv[1]) if (len(argv) >= 2) else '127.0.0.2'
PORT = int(argv[2]) if (len(argv) >= 3) else 10000
# HOST = '127.0.0.1'
# PORT = 10000
BUFSIZE = 2048

ADDR = (HOST, PORT)

udpSerSock = socket(AF_INET, SOCK_DGRAM)
udpSerSock.bind(ADDR)
# udpSerSock.listen(20)
th_pool = []
sock_pool = []
x = 0
def handle(udpCliSock):
    print(ADDR)
    print(HOST)
    print(PORT)
    while True:
        data,clintAddr = udpCliSock.recvfrom(BUFSIZE)
        if data:
            # print(data)
            global x
            x += 1
            # print(x)
            # print(len(data))
        if len(data) == 0:
            break
    # print(x)

def count():
    while True:
        print(x)
        sleep(2);
thread1 = Thread(target=handle, args=(udpSerSock,))
thread2 = Thread(target=count)
# thread.setDaemon(True)
thread1.start()
thread2.start()
while True:
    sleep(10)
# handle(udpSerSock)

