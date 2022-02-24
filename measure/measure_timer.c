#include "measure_timer.h"
#include "measure_log.h"
#include "myHashSet.h"





// pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

//定时器需要执行的动作
//测试定时器，每隔一段时间打印一次当前时间
void* print_current_time(void* argv){
    //获取参数，即定时时间
    int count = 0;
    int time_val = ((timer_param_t*)argv)->time_val;

    MyHashSet *recv_Set = ((timer_param_t*)argv)->recv_Set;
    ElasticSketch *recv_sketch = ((timer_param_t*)argv)->recv_sketch;
    pthread_mutex_t* recv_mutex = ((timer_param_t*)argv)->recv_mutex;
    
    MyHashSet *send_Set = ((timer_param_t*)argv)->send_Set;
    ElasticSketch *send_sketch = ((timer_param_t*)argv)->send_sketch;
    pthread_mutex_t* send_mutex = ((timer_param_t*)argv)->send_mutex;

    int sock = ((timer_param_t*)argv)->sock;

    int signal = ((timer_param_t*)argv)->signal;
    tmpRecvData tmp = ((timer_param_t*)argv)->tmp;
    // int type = ((timer_param_t*)argv)->type;

    // sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));  //每个字节都用0填充
    serv_addr.sin_family = AF_INET;  //使用IPv4地址
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");  //具体的IP地址
    serv_addr.sin_port = htons(12345);  //端口
    connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    printf("connect done");

    MYSQL *conn_ptr;
    unsigned int timeout = 600;	//超时时间7秒
    int ret = 0;
    conn_ptr = mysql_init(NULL);//初始化
	if(!conn_ptr)
	{
		printf("mysql_init failed!\n");
		//return -1;
	}

	ret = mysql_options(conn_ptr,MYSQL_OPT_CONNECT_TIMEOUT,(const char*)&timeout);//设置超时选项
	if(ret)
	{
		printf("Options Set ERRO!\n");
	}
	conn_ptr = mysql_real_connect(conn_ptr,"127.0.0.1","root","123456","mytestdb",0,NULL,0);//连接MySQL testdb数据库
	if(conn_ptr)
	{
		printf("Connection Succeed!\n");
	}
	else	//错误处理
	{
		printf("Connection Failed!\n");
		if(mysql_errno(conn_ptr))
		{
			printf("Connect Erro:%d %s\n",mysql_errno(conn_ptr),mysql_error(conn_ptr));//返回错误代码、错误消息
		}
		//return -2;
	}
    // mysql_autocommit(&mysql, 0);
    char query[100] = "truncate table measure";
    mysql_real_query(conn_ptr,"truncate table measure",strlen(query));
    char query2[100] = "truncate table total";
    mysql_real_query(conn_ptr,"truncate table total",strlen(query2));
    char query3[100] = "truncate table total_send";
    mysql_real_query(conn_ptr,"truncate table total_send",strlen(query3));
    char query4[100] = "truncate table total_recv";
    mysql_real_query(conn_ptr,"truncate table total_recv",strlen(query4));
    printf("have clear table measure, total in mytestdb\n");
    // mysql_commit(&mysql);
    char buf[ 1024 ];
    getcwd(buf, 1024);
    printf("\n");
    printf("%s\n", buf);
    printf("\n");
    


    time_t now;
    struct tm* timenow;
    //清空文件
    FILE *fp, *fp2;
    fp = fopen(STATISTICS_LOG_FILE_RECV,"wb+");
    fp2 = fopen(STATISTICS_LOG_FILE_SEND,"wb+");
    fprintf(fp,"=====================================================================\n");
    fprintf(fp2,"===================================================================\n");

    fclose(fp);
    fclose(fp2);


    insertData *inData = (insertData *)malloc(sizeof(insertData));
    memset(inData,0,sizeof(insertData));

    // tmpRecvData *tmp = (tmpRecvData *)malloc(sizeof(tmpRecvData));
    // memset(tmp,0,sizeof(tmpRecvData));
    clock_t prev_count, now_count;
    prev_count = clock();


    while(1){ 
        
        count += 1;
        // sleep(time_val);
        // time(&now);
        
        
        // timenow = gmtime(&now);
        now_count = clock();

        while ((double)(now_count-prev_count)/CLOCKS_PER_SEC < 5)
        {
            usleep(10000);
            now_count = clock();
        }
        prev_count = now_count;
        
        
        //睡眠

        printf("\nmeasurement module is running, statistics will be saved in /measure_log/statistics_log.txt\n");

        clock_t start_time, end_time; 
        start_time = clock(); // 开始时间
        pthread_mutex_lock(send_mutex);
        mysql_query(conn_ptr,"START TRANSACTION;");
        printf("send log\n\n");
        save_flow_statistics(count, send_sketch, send_Set, conn_ptr, 0,inData);
        pthread_mutex_unlock(send_mutex);

        // pthread_mutex_lock(recv_mutex);
        signal = 1;
        // pthread_mutex_unlock(recv_mutex);
        // pthread_mutex_lock(recv_mutex);
        printf("recv log\n\n");  
        save_flow_statistics(count, recv_sketch, recv_Set, conn_ptr, 1,inData);  
        
        // save_flow_statistics(count, recv_sketch, recv_Set, conn_ptr, 1);
        // pthread_mutex_unlock(recv_mutex);
        // signal = 0;

        end_time = clock();  // 结束时间
        /* 计算得出程序运行时间, 并将其输出到屏幕 */
        printf("\nread Struct time : %lf ", (double)(end_time - start_time) / CLOCKS_PER_SEC);
        printf("measure_log close\n");


        start_time = clock(); // 开始时间

        clock_t last_time, now_time;
        last_time = clock();
        printf("\n tmp has %u data",tmp.size);
        while (1)
        {
            pthread_mutex_lock(recv_mutex);
            // pthread_mutex_unlock(recv_mutex);
            while ((double)(now_time - last_time)/ CLOCKS_PER_SEC < 0.005)
            {
                /* code */
                printf("\ninsert store times : %lf ", (double)(now_time - last_time) / CLOCKS_PER_SEC);
                processTmpPacket(tmp, recv_Set, sock, recv_sketch, 50);
                now_time = clock();
            }
            last_time = now_time;
            // pthread_mutex_lock(recv_mutex);
            
            if(tmp.size == 0){
                signal = 0;
                pthread_mutex_unlock(recv_mutex);
                break;
            }
            pthread_mutex_unlock(recv_mutex);
            usleep(5000);
        }
         


        // insertDataToDB(inData,conn_ptr);
        mysql_query(conn_ptr, "COMMIT;");


        // mysql_commit(&mysql);
        end_time = clock();  // 结束时间
        printf("\ninsert MySQL timestamps : %lf ", (double)(end_time - start_time) / CLOCKS_PER_SEC);

        
    }


}




void measure_timer_create(  int time_val,
                            MyHashSet *recv_Set, ElasticSketch *recv_sketch,pthread_mutex_t* recv_mutex,
                            MyHashSet *send_Set, ElasticSketch *send_sketch,pthread_mutex_t* send_mutex,
                            int sock,
                            int signal,tmpRecvData tmp
                            ){
    pthread_t measure_timer_thread;

    timer_param.time_val = time_val;
    timer_param.sock = sock;

    timer_param.recv_Set = recv_Set;
    timer_param.recv_sketch = recv_sketch;
    timer_param.recv_mutex = recv_mutex;

    timer_param.send_Set = send_Set;
    timer_param.send_sketch = send_sketch;
    timer_param.send_mutex = send_mutex;

    // timer_param.recv_type = type;
    timer_param.signal = signal;
    timer_param.tmp = tmp;
    
    pthread_create(&measure_timer_thread,NULL,print_current_time,&timer_param);
}
