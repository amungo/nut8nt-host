#ifndef _AXIDMA

#define _AXIDMA

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include <signal.h>
#include <memory.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netdb.h>
#include <math.h>
#include <stdint.h>
#include <iostream>

#include <config.h>

#include <gpio.h>
#include <defines.h>

#ifdef DMA_PROXY2
#include "dma-proxy2.h"
#else
#include "dma-proxy.h"
#endif

#define DMA_NUM 4

int CYCLIC_DISTANCE(int a, int b, int r);

class DMAReader {
public:
    // bs - block size
    // bc - block count for 1 super block
    DMAReader(size_t bs, int bc, std::string dmaName = "dma_proxy_rx1");
    ~DMAReader();

    void* getBeginPtr();
    void start();
    void finish();
    void tryRead();
    void write(char* data, uint32_t size);

    uint32_t getAvalableBuffsCount();
    uint32_t getClbCount();

    void* read();
    void reset();
private:
    static int blockCount[DMA_NUM];
    static size_t blockSize[DMA_NUM];
    static int SUPBUFF_COUNT[DMA_NUM];
    int dma_num;

    struct dma_proxy_channel_interface *rx_proxy_interface_p;
    int rx_proxy_fd;

    void** buff_ptr;
    static uint32_t callbackCount[DMA_NUM];
    static volatile uint32_t iterLast[DMA_NUM];
    static volatile uint32_t iterFirst[DMA_NUM];

    void init(size_t bs, std::string dmaName);
    static void callbackf0(int n, siginfo_t *info, void *unused);
    static void callbackf1(int n, siginfo_t *info, void *unused);
    static void callbackf2(int n, siginfo_t *info, void *unused);
    static void callbackf3(int n, siginfo_t *info, void *unused);

    GPIO FIFORun;
};

void printConfigs();

#endif
