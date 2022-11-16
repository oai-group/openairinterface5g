#include <stdio.h>
#include <stdlib.h>
#include <unordered_map>
#include <vector>
#include <fstream>

#include "elastic/BucketSketch.h"
#include <sys/io.h>
#include <dirent.h>
using namespace std;

#define START_FILE_NO 1
#define END_FILE_NO 10
#define HEAVY_MEM (800 * 1024)
#define BUCKET_NUM (HEAVY_MEM / 64)
#define TOT_MEM_IN_BYTES (1000 * 1024)
int count_evict = 0;


struct FIVE_TUPLE {
    unsigned short length;
    char key[13];
    ///
//    unsigned short length;
    ////
};
typedef vector<FIVE_TUPLE> TRACE;
TRACE traces[END_FILE_NO - START_FILE_NO + 1];

void ReadInTraces(const char *trace_prefix) {
    for (int datafileCnt = START_FILE_NO; datafileCnt <= END_FILE_NO; ++datafileCnt) {
        char datafileName[100];
        sprintf(datafileName, "%s%d.dat", trace_prefix, datafileCnt - 1);
        FILE *fin = fopen(datafileName, "rb");

        FIVE_TUPLE tmp_five_tuple;
        traces[datafileCnt - 1].clear();
        while (fread(&tmp_five_tuple, 1, 15, fin) == 15) {
            traces[datafileCnt - 1].push_back(tmp_five_tuple);
        }
        fclose(fin);

    }
    printf("\n");
}

void Measure(int type, const char *datafiledir){
    ReadInTraces(datafiledir);
    BucketSketch<BUCKET_NUM, TOT_MEM_IN_BYTES> *elastic = NULL;
    double AREList[END_FILE_NO] = {0.0};
    for (int datafileCnt = START_FILE_NO; datafileCnt <= END_FILE_NO; ++datafileCnt) {
        unordered_map<string, int> Real_Freq;
        elastic = new BucketSketch<BUCKET_NUM, TOT_MEM_IN_BYTES>();
        int packet_cnt = (int) traces[datafileCnt - 1].size();
        unordered_map<string, int> Aggregation_insert;
        for (int i = 0; i < packet_cnt; ++i) {
            string str((const char *) (traces[datafileCnt - 1][i].key), 4);
            if (type == 0) {
                elastic->insert((uint8_t *) (traces[datafileCnt - 1][i].key));
                Real_Freq[str]++;
            } else {
                elastic->insert((uint8_t *) (traces[datafileCnt - 1][i].key), traces[datafileCnt - 1][i].length);
                Real_Freq[str] += traces[datafileCnt - 1][i].length;
            }
        }
        double total = 0.0;
        double measure = 0.0;

        ////
        double ARE = 0;
        double absE = 0;
        for (unordered_map<string, int>::iterator it = Real_Freq.begin(); it != Real_Freq.end(); ++it) {
            uint8_t key[4];
            memcpy(key, (it->first).c_str(), 4);
            int est_val = elastic->query(key);
            int dist = std::abs(it->second - est_val);
            ARE += dist * 1.0 / (it->second);
            absE += dist * 1.0;
            measure += est_val;
            total += it->second;
        }
        printf("流量Trace%d.dat: \t", datafileCnt - 1);
        if (type == 0) {
            printf("测量发送包数为：%.0lf  ", measure);
            printf("真实发送包数为：%.0lf  ", total);
            printf("准确率为：%.1lf%%  ", (1 - std::abs(total - measure) / total)*100);
        } else {
            printf("测量发送字节数：%.0lf  ", measure);
            printf("真实发送字节数：%.0lf  ", total);
            printf("准确率为：%.1lf%% ", (1 - std::abs(total - measure) / total)*100);
        }
        printf("\n");
        delete elastic;
        Real_Freq.clear();
    }
}
void StatisticOffline(const char *datafiledir){
    ReadInTraces(datafiledir);
        BucketSketch<BUCKET_NUM, TOT_MEM_IN_BYTES> *elastic = NULL;
        for (int datafileCnt = START_FILE_NO; datafileCnt <= END_FILE_NO; ++datafileCnt) {
            unordered_map<string, int> Real_Freq_bytes;
            elastic = new BucketSketch<BUCKET_NUM, TOT_MEM_IN_BYTES>();
            int packet_cnt = (int) traces[datafileCnt - 1].size();
            unordered_map<string, int> Aggregation_insert;
            for (int i = 0; i < packet_cnt; ++i) {
                string str((const char *) (traces[datafileCnt - 1][i].key), 4);
                Real_Freq_bytes[str] += traces[datafileCnt - 1][i].length;
            }

            double total = 0.0;

            for (unordered_map<string, int>::iterator it = Real_Freq_bytes.begin(); it != Real_Freq_bytes.end(); ++it) {
                uint8_t key[4];
                memcpy(key, (it->first).c_str(), 4);
                total += it->second;

            }
            printf("流量Trace%d.dat:  ", datafileCnt - 1);
            printf("发送业务流数为:%llu  ", Real_Freq_bytes.size());
            printf("发送包数为:%d  ", packet_cnt);
            printf("发送字节数为:%.0lf(%.3lf GB)  ", total,total/ pow(1024,3));
            printf("流平均包数为:%.2lf  ",(double )packet_cnt/(double )Real_Freq_bytes.size());
            printf("流平均字节数为:%.0lf  \n",(double )total/(double )Real_Freq_bytes.size());
            delete elastic;
            Real_Freq_bytes.clear();
        }
        printf("\n\n\n");
}

//获取特定格式的文件名
void GetAllFormatFiles( string path, vector<string>& files,string format)
{
    DIR *pDir;
    struct dirent* ptr;
    if(!(pDir = opendir(path.c_str()))){
        cout<<"Folder doesn't Exist!"<<endl;
        return;
    }
    while((ptr = readdir(pDir))!=0) {
        if (strcmp(ptr->d_name, ".") != 0 && strcmp(ptr->d_name, "..") != 0){
            files.push_back(path + "/" + ptr->d_name);
        }
    }
    closedir(pDir);
}
void Stringsplit(const string& str, const char split, vector<string>& res)
{
    if (str == "")		return;
    string strs = str + split;
    size_t pos = strs.find(split);
    while (pos != strs.npos)
    {
        string temp = strs.substr(0, pos);
        res.push_back(temp);
        strs = strs.substr(pos + 1, strs.size());
        pos = strs.find(split);
    }
}
void onlineStatistic(){
    //string onlineFileDir = "../data/onlineTraceData/";
    string onlineFileDir1 = "../data/onlineTraceData/iot/";
    string onlineFileDir2 = "../data/onlineTraceData/voip/";
    string onlineFileDir3 = "../data/onlineTraceData/video/";
    vector<string> files;
    string format = ".pkts";
    //GetAllFormatFiles(onlineFileDir, files,format);
    GetAllFormatFiles(onlineFileDir1, files,format);
    GetAllFormatFiles(onlineFileDir2, files,format);
    GetAllFormatFiles(onlineFileDir3, files,format);
    int size = files.size();
    int packet_cnt = 0;
    unordered_map<string, int> Real_Freq_bytes;
    for (int i = 0;i<size;i++)
    {
        ifstream in(files[i]);
        string filename;
        string line;
        if(in) // 有该文件
        {
            while (getline (in, line)) // line中不包括每行的换行符
            {
                vector<string> strList;
                Stringsplit(line, ' ', strList);
                string key = to_string(i)+"-"+strList[3];
                int bytes = atoi(strList[1].c_str());
                Real_Freq_bytes[key] += bytes;
                packet_cnt++;
            }
        }
        else // 没有该文件
        {
            cout <<"no such file" << endl;
        }
    }

    double total = 0.0;

    for (unordered_map<string, int>::iterator it = Real_Freq_bytes.begin(); it != Real_Freq_bytes.end(); ++it) {
        total += it->second;
    }
    printf("发送业务流数为:%llu  ", Real_Freq_bytes.size());
    printf("发送包数为:%d  ", packet_cnt);
    printf("发送字节数为:%.0lf  ", total);
    printf("流平均包数为:%.2lf  ",(double )packet_cnt/(double )Real_Freq_bytes.size());
    printf("流平均字节数为:%.0lf  \n",(double )total/(double )Real_Freq_bytes.size());
    Real_Freq_bytes.clear();

}
int main(int argc, char *argv[]) {

    char datafiledir[100];
//    sprintf(datafiledir, "../data/%slineTraceData/", argv[1]);

    int op;
    cout << "请输入需要进行的操作:" << endl;
    cout << "1:离线数据回放" << endl;
    cout << "2:在线数据回放" << endl;
    cout << "3:离线数据测量效果分析（包数目）" << endl;
    cout << "4:离线数据测量效果分析（字节）" << endl;

    cin >> op;
    cout << "" << endl;
    if (op == 1) {
        sprintf(datafiledir, "../data/offlineTraceData/");
        cout << "正在进行离线数据回放" << endl;
        StatisticOffline(datafiledir);
    } else if (op == 2) {


        cout << "正在进行在线数据回放" << endl;
        onlineStatistic();
    }
    else if (op == 3) {
        sprintf(datafiledir, "../data/offlineTraceData/");
        cout << "正在进行离线数据测量效果分析（包数目）" << endl;
        Measure(0,datafiledir);
    }
    else if (op == 4) {
        sprintf(datafiledir, "../data/offlineTraceData/");
        cout << "正在进行离线数据测量效果分析（字节）" << endl;
        Measure(1,datafiledir);
    }



}
