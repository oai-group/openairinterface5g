# 导入模块

import pymysql


def acc3():
    # 建立连接
    conn = pymysql.connect(
        user='root',  # 用户名
        password='123456',  # 密码：这里一定要注意123456是字符串形式
        host='192.168.1.95',  # 指定访问的服务器，本地服务器指定“localhost”，远程服务器指定服务器的ip地址
        database='mytestdb',  # 数据库的名字
        port=3306,  # 指定端口号，范围在0-65535
        charset='utf8mb4',  # 数据库的编码方式
    )
    sql = 'SELECT sum(totalBytes)*5,sum(totalPkts)*5 from total_send'

    curses = conn.cursor()
    curses.execute(sql)
    res = curses.fetchall()[0]
    bytes = int(res[0])
    pkts = int(res[1])
    print("总字节数为：" + str(bytes))
    print("总报文数为：" + str(pkts))
def acc():
    # 建立连接
    conn = pymysql.connect(
        user='root',  # 用户名
        password='123456',  # 密码：这里一定要注意123456是字符串形式
        host='192.168.1.95',  # 指定访问的服务器，本地服务器指定“localhost”，远程服务器指定服务器的ip地址
        database='mytestdb',  # 数据库的名字
        port=3306,  # 指定端口号，范围在0-65535
        charset='utf8mb4',  # 数据库的编码方式
    )
    sql = 'SELECT sum(totalBytes)*5,sum(totalPkts)*5 from total_send'

    curses = conn.cursor()
    curses.execute(sql)
    res = curses.fetchall()[0]
    bytes = int(res[0])
    pkts = int(res[1])
    print("总字节数为：" + str(bytes))
    print("总报文数为：" + str(pkts))

    print("操作对应码：")
    print("1.清空统计表")
    print("2.计算统计的总字节数")
    print("3.计算统计的总报文数")
    print("4.计算统计的字节测量准确率")
    print("5.计算统计的数据包测量准确率")
    while True:
        content = input("请输入分析内容：")
        if int(content) == 1:
            op = input("\033[31m你正在清空表数据，确认请输入1:\033[0m")
            if int(op) == 1:
                sql = 'truncate table total_send'
                curses.execute(sql)
            else:
                print("\t\033[32m取消清除数据表1\033[0m")
        elif int(content) == 2:
            print("\t总字节数为：" + str(bytes))
        elif int(content) == 3:
            print("\t总报文数为：" + str(pkts))
        elif int(content) == 4:
            r_bytes = int(input("\t请输入真实值："))
            print("\t\t总字节数为：" + str(bytes))
            print("\t\t准确率为：{:.1f}%".format(100 - abs(r_bytes - bytes) / bytes * 100))
        elif int(content) == 5:
            r_pkts = int(input("\t请输入真实值："))
            print("\t\t总报文数为：" + str(pkts))
            print("\t\t准确率为：{:.1f}%".format(100 - abs(r_pkts - pkts) / pkts * 100))


def test():
    # 建立连接
    conn = pymysql.connect(
        user='root',  # 用户名
        password='123456',  # 密码：这里一定要注意123456是字符串形式
        host='192.168.1.81',  # 指定访问的服务器，本地服务器指定“localhost”，远程服务器指定服务器的ip地址
        database='mytestdb',  # 数据库的名字
        port=3306,  # 指定端口号，范围在0-65535
        charset='utf8mb4',  # 数据库的编码方式
    )

    sql = "INSERT INTO measure2 (srcIP, dstIP, srcPort, dstPort, protocol, averBytes, averPkts) values (1, '7', '6', '4', '5', '6.5', '6.8');"
    # sql = 'SELECT * from measure2'

    curses = conn.cursor()
    curses.execute(sql)
    conn.commit()
    res = curses.fetchall()
    for str in res:
        print(str)


def acc2():
    conn = pymysql.connect(
        user='root',  # 用户名
        password='123456',  # 密码：这里一定要注意123456是字符串形式
        host='192.168.1.95',  # 指定访问的服务器，本地服务器指定“localhost”，远程服务器指定服务器的ip地址
        database='mytestdb',  # 数据库的名字
        port=3306,  # 指定端口号，范围在0-65535
        charset='utf8mb4',  # 数据库的编码方式
    )

    sql = 'SELECT srcIP, dstIP, srcPort, dstPort, protocol, sum(averBytes)*5 as total_bytes, sum(averPkts)*5 as total_pkts from measure2 group by srcIP, dstIP, srcPort, dstPort, protocol'

    curses = conn.cursor()
    curses.execute(sql)
    res = curses.fetchall()

    # 计算数目和准确率
    flow_n = 0
    accuracy_bytes = 0
    accuracy_pkts = 0
    r_all_bytes = 0
    r_all_pkts = 0
    diff_bytes = 0
    diff_pkts = 0
    for r in res:
        # print(r)
        srcPort = int(r[2])
        m_bytes = int(r[5])
        m_pkts = int(r[6])
        # print(m_bytes)
        t = (srcPort%10000) // 1000
        prefix = ''
        if t == 0:
            prefix = "video"
        if t == 1:
            prefix = "voip"
        if t == 2:
            prefix = "iot"
        index = srcPort % 1000
        # print(srcPort)
        # print(index)
        file_name = "./pkt/" + prefix + "/" + prefix + "_" + str(index) + ".pkts"
        # file_name = "./pkt/video/video_1.pkts"
        r_pkts = 0
        r_bytes = 0
        try:
            with open(file_name, "r") as f:
                flow_n += 1
                for line in f.readlines():
                    r_pkts += 1
                    r_bytes += int(line.split()[1])
            one_acc_bytes = (1 - abs(r_bytes - m_bytes) / r_bytes)

            one_acc_pkts = (1 - abs(r_pkts - m_pkts) / r_pkts)
            print(str(srcPort) + "字节测量准确率为{:.2f}%".format(one_acc_bytes*100))
            print(str(srcPort) + "包数测量准确率为{:.2f}%".format(one_acc_pkts*100))
            r_all_bytes += r_bytes
            r_all_pkts += r_pkts
            diff_bytes += m_bytes - r_bytes
            diff_pkts += m_pkts - r_pkts
            accuracy_bytes += one_acc_bytes
            accuracy_pkts += one_acc_pkts
        except:
            pass
        # print("r_bytes=" + str(r_bytes))
        # print("r_pkts=" + str(r_pkts))


        # if one_acc_pkts<0:
        #     print("eeee")


        # print("r_bytes=" + str(r_bytes))
        # print("r_pkts=" + str(r_pkts))
    # print(diff_bytes)
    # print(diff_pkts)
    # print(r_all_bytes)
    # print(r_all_pkts)
    print("字节测量准确率为{:.2f}%".format(accuracy_bytes/flow_n*100))
    print("包数测量准确率为{:.2f}%".format(accuracy_pkts/flow_n*100))


if __name__ == '__main__':
    # test()
    # acc()
    acc3()
    acc2()
