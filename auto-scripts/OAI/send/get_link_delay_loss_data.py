# 导入模块

import pymysql


def get_and_save_delay(start, end):
    # 建立连接
    conn = pymysql.connect(
        user='root',  # 用户名
        password='123456',  # 密码：这里一定要注意123456是字符串形式
        host='192.168.1.95',  # 指定访问的服务器，本地服务器指定“localhost”，远程服务器指定服务器的ip地址
        database='mytestdb',  # 数据库的名字
        port=3306,  # 指定端口号，范围在0-65535
        charset='utf8mb4',  # 数据库的编码方式
    )
    # sql = 'SELECT sum(totalBytes)*5,sum(totalPkts)*5 from total_send'
    sql = "SELECT value from link_info where linkid='{0}-{1}.delay'".format(start,end)
    print(sql)

    curses = conn.cursor()
    curses.execute(sql)
    res = curses.fetchall()
    delay_list = []
    for i in res:
        delay_list.append(float(i[0]))
    print(delay_list)
    with open("link_delay_cache", 'w') as f:
        for i in delay_list:
            f.write(str(i) + " ")


def get_and_save_loss(start, end):
    # 建立连接
    conn = pymysql.connect(
        user='root',  # 用户名
        password='123456',  # 密码：这里一定要注意123456是字符串形式
        host='192.168.1.95',  # 指定访问的服务器，本地服务器指定“localhost”，远程服务器指定服务器的ip地址
        database='mytestdb',  # 数据库的名字
        port=3306,  # 指定端口号，范围在0-65535
        charset='utf8mb4',  # 数据库的编码方式
    )
    # sql = 'SELECT sum(totalBytes)*5,sum(totalPkts)*5 from total_send'
    # sql = 'SELECT loss from delayloss'
    sql = "SELECT value from link_info where linkid='{0}-{1}.loss'".format(start, end)

    curses = conn.cursor()
    curses.execute(sql)
    res = curses.fetchall()
    delay_list = []
    for i in res:
        delay_list.append(float(i[0]))
    print(delay_list)
    with open("link_loss_cache", 'w') as f:
        for i in delay_list:
            f.write(str(i) + " ")

def clear():
    conn = pymysql.connect(
        user='root',  # 用户名
        password='123456',  # 密码：这里一定要注意123456是字符串形式
        host='192.168.1.95',  # 指定访问的服务器，本地服务器指定“localhost”，远程服务器指定服务器的ip地址
        database='mytestdb',  # 数据库的名字
        port=3306,  # 指定端口号，范围在0-65535
        charset='utf8mb4',  # 数据库的编码方式
    )
    sql = 'truncate table link_info'

    curses = conn.cursor()
    curses.execute(sql)

if __name__ == '__main__':

    print("操作对应码：")
    print("1.计算delay准确率（10ms）")
    print("2.计算delay准确率（20ms）")
    print("3.计算loss准确率（10%）")
    print("4.计算loss准确率（20%）")
    print("5.读取数据库数据到本地")
    print("9.清除数据")
    while True:
        content = int(input("请输入分析内容："))
        if content == 1 or content == 2:
            base = 0
            if content == 1:
                base = 10
            else:
                base = 20
            n = 0
            d = 0
            with open("delay_cache", 'r') as f:
                for line in f.readlines():
                    delay_list = line.split()
                delay_list = [float(i) for i in delay_list]
                for delay in delay_list:
                    if abs(delay - base) < 3:
                        n += 1
                        d += delay
                # print(base)
                # print(n)
                try:
                    print(str(base) + "ms时延测量平均值为{:.2f}%".format(d / n))
                    print(str(base) + "ms时延测量准确率为{:.2f}%".format((1 - abs(d / n - base) / base) * 100))
                except:
                    print("没有有效数据")
        if content == 3 or content == 4:
            base = 0
            if content == 3:
                base = 10
            else:
                base = 20
            n = 0
            l = 0
            with open("loss_cache", 'r') as f:
                for line in f.readlines():
                    loss_list = line.split()
                loss_list = [float(i) * 100 for i in loss_list]
                for loss in loss_list:
                    if abs(loss - base) < 3:
                        n += 1
                        l += loss
                try:
                    print(str(base) + "%丢包测量平均值为{:.2f}%".format(l / n))
                    print(str(base) + "%丢包测量准确率为{:.2f}%".format((1 - abs(l / n - base) / base) * 100))
                except:
                    print("没有有效数据")
        if content == 5:
            start = int(input("请输入起始点"))
            end = int(input("请输入终止点"))
            get_and_save_delay(start,end)
            get_and_save_loss(start,end)
        if content == 9:
            clear()
