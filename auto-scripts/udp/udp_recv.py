import socket

def severSocket():
    # 1.found
    udp_socket = socket.socket(socket.AF_INET,socket.SOCK_DGRAM)

    # 2.bind
    local_addr = ('172.16.0.4',9998)
    udp_socket.bind(local_addr)
    i = 1
    while True:
        # accept
        revc_data = udp_socket.recvfrom(1024)

        # show =>(data,(ip,port))
        print(i)
        i += 1
        print(revc_data[0].decode("utf-8"))
    # resource
    udp_socket.close()

if __name__ == '__main__':
    severSocket()


