#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/unistd.h>
#include <netinet/in.h>
#include <fstream>
#include <stdio.h>
#include <string.h>
#include <ctime>

#define BUFFER_SIZE (1024)
#define CHANNEL_NUM 8

using namespace std;

int main()
{
    cout << "Init TCP Server for NUT8NT" << endl;

    int sock, listener;
    struct sockaddr_in addr;
    char buf[BUFFER_SIZE];
    int16_t* buf16 = (int16_t*)buf;

    int16_t channelBuf16[CHANNEL_NUM][BUFFER_SIZE/CHANNEL_NUM];
    int8_t channelBuf8[CHANNEL_NUM][BUFFER_SIZE/CHANNEL_NUM];
    ofstream file8[CHANNEL_NUM];
    ofstream file16[CHANNEL_NUM];

    auto rawData = time(nullptr);
    auto date = localtime(&rawData);

    for (int i=0; i < CHANNEL_NUM; i++) {
        char fileName[128] = {0};
        sprintf(fileName, "Dump_%.2d.%.2d.%.2d_%.2d.%.2d_channel%d.int8", date->tm_mday, date->tm_mon + 1, 1900 + date->tm_year, date->tm_hour, date->tm_min, i+1);

        file8[i].open(fileName);

        fileName[strlen(fileName) - 1] = '1';
        fileName[strlen(fileName)] = '6';

        file16[i].open(fileName);
    }

    cout << "Create TCP Server for NUT8NT" << endl;

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

    listen(listener, 1);

    cout << "Start server" << endl;

    sock = accept(listener, NULL, NULL);
    if(sock < 0) {
        perror("Accept error");
        exit(-3);
    }

    cout << "Client connected" << endl;

    unsigned long totalRead = 0;
    unsigned int offsert = 0;

    while(1) {
        int ret = recv(sock, buf + offsert, sizeof(buf), 0);

        if(ret < 0) {
            perror("recv error");
            break;
        }
        else if (ret == 0) {
            cout << "Connection closed" << endl;
            break;
        } else {
            ret += offsert;
            offsert = ret % 16;
            ret -= offsert;

            if (ret % 16) {
                cout << "ret = " << ret << endl;
            }
            totalRead += ret;

            for (int i=0; i < ret/2; i += CHANNEL_NUM) {
                for (int j=0; j < CHANNEL_NUM; j++) {
                    channelBuf16[j][i/8] = buf16[i+j];
                    channelBuf8[j][i/8] = (buf16[i+j] >> 8) & 0xFF;
                }
            }

            for (int i=0; i < CHANNEL_NUM; i++) {
                file8[i].write((char*)(channelBuf8[i]), ret/2/8);
                file16[i].write((char*)(channelBuf16[i]), ret/8);
            }

            memcpy(buf, buf+ret, offsert);

        }
    }

    for (int i=0; i < CHANNEL_NUM; i++) {
        file8[i].close();
        file16[i].close();
    }

    cout << "Recieved " << totalRead/1024/1024 << "MB" << endl;

    close(sock);

    return 0;
}
