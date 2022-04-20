#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <memory.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <thread>

#include <axidma.h>
#include <stdint.h>
#include <iostream>

bool stop = false;
int sockfd = 0;
uint64_t total_send = 0;

int connect_to_server(char* addr);

void speedTh() {
    int strSize;
    char str[32];
    uint64_t totalSendLast = 0;
    printf("Send speed: ");
    sleep(2);
    int time = 0;

    while (!stop) {
        sprintf(str,"%ld MByte/s (%d s), %.2f %%", (total_send - totalSendLast)/1024/1024, time);
        strSize = strlen(str);
        printf("\r%s", str);

        totalSendLast = total_send;
        time ++;
        fflush(stdout);


        sleep(1);
        if (stop)
            break;
    }
}

int ctrlC_count = 0;
void sig_int_handler(int signo) {
    printf("Stopping programm\n");

    stop = true;
    if (sockfd)
        close(sockfd);

    ctrlC_count++;

    if (ctrlC_count == 2)
        exit(-1);
}

int main(int argc, char *argv[]) {
    printConfigs();

    signal(SIGINT, sig_int_handler);
    signal(SIGTERM, sig_int_handler);

    if (argc < 2) {
        std::cout << "usage: " << argv[0] << " addr [cmod]"  << std::endl;
        return 0;
    }

    if (connect_to_server(argv[1]) != 0) {
        return 0;
    }

    std::thread th(&speedTh);

    if (argc == 2) {

        DMAReader reader(DMA_BUF_SIZE, SUPER_BUFFER_SIZE);



        reader.start();
        while(!stop) {
            char* data = (char*)reader.read();

            if (data) {
                uint32_t* iq = (uint32_t*)(data);

                if (reader.getClbCount() > 5)
                    total_send += write(sockfd, iq, DMA_BUF_SIZE);
            }
        }
        reader.finish();
    }

    th.join();

    return 0;
}

int connect_to_server(char* addr) {
    struct sockaddr_in servaddr;

    // socket create and varification
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("socket creation failed...\n");
        return -1;
    }
    else
        printf("Socket successfully created..\n");
    bzero(&servaddr, sizeof(servaddr));

    // assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(addr);
    servaddr.sin_port = htons(30138);

    // connect the client socket to server socket
    if (connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) != 0) {
        printf("connection with the server failed...\n");
        return -1;
    }
    else
        printf("connected to the server..\n");

    int size = 0;
    write(sockfd, &size, sizeof(size));

    int chan = CHANNELS_NUM;
    write(sockfd, &chan, sizeof(chan));

    return 0;
}
