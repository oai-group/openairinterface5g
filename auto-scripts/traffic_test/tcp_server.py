# 1创建套接字
import socket
import sys
import threading


# 接收消息
def rev(client_socket, ip_port):
    while True:
        client_text = client_socket.recv(1024)
        # 如果接收的消息长度不为0，则将其解码输出
        if client_text:
            print(ip_port, ":", client_text.decode("utf-8"))
            # 给客户端响应
            # client_socket.send("收到\n".encode())
        # 当客户端断开连接时，会一直发送''空字符串，所以长度为0已下线
        else:
            print(ip_port, "Exit")
            client_socket.close()
            break


def main():
    # 获取第一个参数 IP
    server_ip = str(sys.argv[1])
    # 获取第二个参数 端口
    server_port = int(sys.argv[2])

    # 1建立socket对象
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    # 设置端口复用
    s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, True)

    # 2，须要本身绑定一个ip地址和端口号
    s.bind((server_ip, server_port))

    # 3监听操做时刻注意是否有客户端请求发来
    s.listen(128)

    while True:
        s1, address = s.accept()
        # print(address)
        # rev   接收数据
        # 有客户端连接后，创建一个线程将客户端套接字，IP端口传入rev函数，
        t1 = threading.Thread(target=rev, args=(s1, address))
        # 设置线程守护
        t1.setDaemon(True)
        # 启动线程
        t1.start()

        # data = s1.rev(1024)  # 一次接收1024字节
        # print(data.decode('utf-8'))  # decode()解码收到的字节
        # # s1.send(data)
        # if data == 'exit':
        #     break

    # s.close()


if __name__ == '__main__':
    main()
