//#define UBUNTU

#include <iostream>
#include <sys/types.h>
#ifdef UBUNTU
#include <sys/socket.h>
#include <sys/unistd.h>
#include <netinet/in.h>
#define closesocket close
#define Sleep usleep
#else
#include <winsock2.h>
#endif
#include <fstream>
#include <stdio.h>
#include <string.h>
#include <ctime>
#include <signal.h>
#include <vector>
#include <queue>
#include <mutex>
#include <thread>
#include <time.h>


#define BUFFER_SIZE (1024*32*80)
#define CHANNEL_NUM 4
#define SAMPLE_SIZE 4 //byte
#define SAMPLE_IQ   2 //1 - S1,S2,S3... 2 - [I1,Q1],[I2,Q2]...
#define SKIP_BYTES  (0)

//#define USE_EXTRACT

struct data_t {
    size_t size;
    char* data;
};

using namespace std;

void getData(int size);
void getInfData(int chanNum);
void writeTh();
void statisticTh();

bool run;

int sock, listener;

int16_t channelBuf16[CHANNEL_NUM][BUFFER_SIZE/CHANNEL_NUM];
float channelBufGR32[CHANNEL_NUM][BUFFER_SIZE/CHANNEL_NUM];
int8_t channelBuf8[CHANNEL_NUM][BUFFER_SIZE/CHANNEL_NUM];
int64_t rawBuf[BUFFER_SIZE];

#ifdef USE_EXTRACT
ofstream file16[CHANNEL_NUM];
#endif
ofstream fileRaw;

struct sockaddr_in addr;
char buf[BUFFER_SIZE];
unsigned long totalRead = 0;

queue<data_t> dataVec;
mutex m;

void sig_int_handler(int signo) {
    cout << "Stopping programm" << endl;

    cout << "Total read = " << totalRead/1024 << " KByte" << endl;
    if (listener)
        closesocket(listener);

    if (sock)
        closesocket(sock);

#ifdef USE_EXTRACT
    for (int i=0; i < CHANNEL_NUM; i++) {
        if (file16[i].is_open())
            file16[i].close();
    }
#endif

    run = false;
}

int main()
{
    cout << "Init TCP Server for NUT8NT" << endl;

    signal(SIGINT, sig_int_handler);
    signal(SIGTERM, sig_int_handler);

    auto rawData = time(nullptr);
    auto date = localtime(&rawData);

    cout << "Create TCP Server for NUT8NT" << endl;

#ifndef UBUNTU
    WORD wVersionRequested;
    WSADATA wsaData;
    int err;

    /* Use the MAKEWORD(lowbyte, highbyte) macro declared in Windef.h */
    wVersionRequested = MAKEWORD(2, 2);

    err = WSAStartup(wVersionRequested, &wsaData);
#endif
    listener = socket(AF_INET, SOCK_STREAM, 0);
    if(listener < 0) {
        perror("Socket listener");
        exit(-1);
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(30138);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if(bind(listener, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("Bind error");
        exit(-2);
    }

    run = true;

    listen(listener, 1);

    cout << "Start server" << endl;

    sock = accept(listener, NULL, NULL);
    if(sock < 0) {
        perror("Accept error");
        exit(-3);
    }

    cout << "Client connected" << endl;

    thread statTh(statisticTh);
    thread wrTh(writeTh);

    int size;
    recv(sock, (char*)&size, sizeof(size), 0);

    if (size) {

    }
    else {
        int chanNum;
        recv(sock, (char*)&chanNum, sizeof(chanNum), 0);

        cout << "Ctrl-C for stop" << endl;
        getInfData(chanNum);
    }
#ifdef USE_EXTRACT
    for (int i=0; i < CHANNEL_NUM; i++) {
        file16[i].close();
    }
#endif
    closesocket(sock);
    closesocket(listener);

    statTh.join();
    wrTh.join();

    return 0;
}

void getInfData(int chanNum) {
    unsigned int offsert = 0;
    int last_prog = 0;
    int skiped = 0;

    while(run) {
        int ret = recv(sock, buf, sizeof(buf), MSG_WAITALL);
        //continue;
        //cout << ret << endl;

        if (skiped < SKIP_BYTES) {
            skiped += ret;
            continue;
        }

        if(ret < 0) {
            perror("recv error");
            break;
        }
        else if (ret == 0) {
            cout << "Connection closed" << endl;
            run = false;
            break;
        } else {

            totalRead += ret;
            data_t data;
            data.size = ret;
            data.data = new char[ret];
            memcpy(data.data, buf, ret);

            m.lock();
            dataVec.push(data);
            m.unlock();
        }
    }
}

void writeTh() {
    std::cout << "Start write thread\n";

    auto rawData = time(nullptr);
    auto date = localtime(&rawData);

    char fileName[128] = {0};

    sprintf(fileName, "Dump_%.2d.%.2d.%.2d_%.2d.%.2d.int64", date->tm_mday, date->tm_mon + 1, 1900 + date->tm_year, date->tm_hour, date->tm_min);
    fileRaw.open(fileName, ios_base::binary);

#ifdef USE_EXTRACT
    for (int i=0; i < CHANNEL_NUM; i++) {
        sprintf(fileName, "Dump_%.2d.%.2d.%.2d_%.2d.%.2d_channel%d.int16", date->tm_mday, date->tm_mon + 1, 1900 + date->tm_year, date->tm_hour, date->tm_min, i+1);
        file16[i].open(fileName);
    }
#endif

    while (run) {
        m.lock();

        if (!dataVec.empty()) {
            data_t d = dataVec.front();
            dataVec.pop();
            m.unlock();

            fileRaw.write(d.data, d.size);

#ifdef USE_EXTRACT
            int chanNum = 4;
            int16_t* buf16 = (int16_t*)d.data;

            for (int i=0; i < d.size / sizeof(int32_t); i += chanNum) {
                for (int j=0; j < chanNum; j++) {
                    channelBuf16[j][i/(2)] = buf16[(i+j)*SAMPLE_IQ];
                    channelBuf16[j][i/(2) + 1] = buf16[(i+j)*SAMPLE_IQ + 1];
                }
            }

            for (int i=0; i < chanNum; i++) {
                file16[i].write((char*)(channelBuf16[i]), d.size/4);
            }
#endif

            delete[] d.data;
        } else {
            m.unlock();
        }

        Sleep(100);
    }

    cout << "Saving data...\n";
    m.lock();

    while (!dataVec.empty()) {
        data_t d = dataVec.front();
        dataVec.pop();

        fileRaw.write(d.data, d.size);

#ifdef USE_EXTRACT
        int16_t* buf16 = (int16_t*)d.data;
        int chanNum = 4;

        for (int i=0; i < d.size / sizeof(int32_t); i += chanNum) {
            for (int j=0; j < chanNum; j++) {
                channelBuf16[j][i/(chanNum)] = buf16[(i+j)*SAMPLE_IQ];
                channelBuf16[j][i/(chanNum) + 1] = buf16[(i+j)*SAMPLE_IQ + 1];
            }
        }

        for (int i=0; i < chanNum; i++) {
            file16[i].write((char*)(channelBuf16[i]), d.size/4);
        }
#endif

        delete[] d.data;
    }
    m.unlock();

#ifdef USE_EXTRACT
    for (int i=0; i<4; i++) {
        file16[i].close();
    }
#endif

    fileRaw.close();
}

void statisticTh() {

    return;
    while (run) {
        long dataCount;

        m.lock();
        data_t d = dataVec.front();
        dataCount = dataVec.size();
        m.unlock();

        cout << "Using memory: " << (dataCount * d.size / 1024) << "KB\n";

        Sleep(1000);
    }
}
