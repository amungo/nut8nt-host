#include <iostream>
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

#include "gpio.h"
#include "GNU_UDP_client.h"
#include "dma-proxy.h"

#define PERIOD 4

using namespace std;

struct dma_proxy_channel_interface *rx_proxy_interface_p;
uint32_t* status_reg;
int rx_proxy_fd;
GPIO FIFO(88, GPIO_DIR_OUT);
udp_client* client;
uint32_t fifo_full = 0,
         fifo_empty = 0,
         evt_count = 0;
bool stop = false;
int ch = 0;

void init(int bn, int len);
void startCyclic(int c, int timeout);

void callbackf(int n, siginfo_t *info, void *unused) {

    if (evt_count % 128 == 0) {
        uint16_t b[1024];
        for (int i=0; i<1024; i++) {
            b[i] = ((uint16_t*) rx_proxy_interface_p->buffer)[i*8 + ch];
        }
        client->send((char*)b, 1024);
    }

    //fast_check(buff_ptr[(evt_count*PERIOD) % rx_proxy_interface_p->buf_num], rx_proxy_interface_p->length*PERIOD);

    if (status_reg[0] & (1 << 29)) {
        fifo_full++;
    }
    if (status_reg[0] & (1 << 28)) {
        fifo_empty++;
    }

    evt_count++;
}

int main(int argc, char *argv[])
{
    if (argc < 3) {
        printf("usage: NUT8_DMA2UDP address channel\n  channel range [0;7]\n");
        return 0;
    }

    init(512, 8192);
    ch = argv[2][0] - '0';
    client = new udp_client(argv[1], 30137);

    startCyclic(-100, 50);

    cout << "last data [" << ((uint16_t*) rx_proxy_interface_p->buffer)[0] << ", " << ((uint16_t*) rx_proxy_interface_p->buffer)[1] << "]" << endl;
    cout << "evt_count = " << evt_count * PERIOD << endl;

    munmap(rx_proxy_interface_p, sizeof(struct dma_proxy_channel_interface));
    close(rx_proxy_fd);

    return 0;
}

void init(int bn, int len) {
    printf("DMA init\n");
    int fd_mem = open("/dev/mem", O_RDWR | O_SYNC);
    status_reg = (uint32_t*)mmap(NULL, 4, PROT_READ | PROT_WRITE, MAP_SHARED, fd_mem, 0xA0010000L);

    FIFO.out(1);

    rx_proxy_fd = open("/dev/dma_proxy_rx", O_RDWR);
    if (rx_proxy_fd < 1) {
        printf("Unable to open DMA proxy device file");
        exit(EXIT_FAILURE);
    }

    rx_proxy_interface_p = (struct dma_proxy_channel_interface *)mmap(NULL, sizeof(struct dma_proxy_channel_interface),
                                    PROT_READ | PROT_WRITE, MAP_SHARED, rx_proxy_fd, 0);
    if (rx_proxy_interface_p == MAP_FAILED) {
        perror("mmap tx_proxy_interface_p");
        exit(EXIT_FAILURE);
    }

    rx_proxy_interface_p->length = len;
    rx_proxy_interface_p->buf_num = bn;

    memset(rx_proxy_interface_p->buffer, 0, BUFFER_SIZE);
}

void startCyclic(int c, int timeout) {
    int dummy = 0;
    printf("startCyclic start\n");
    struct sigaction sig;
    sig.sa_sigaction = callbackf;
    sig.sa_flags = SA_SIGINFO;
    if( sigaction(SIGIO, &sig, NULL) < 0 ){
        perror("sigaction");
        exit(1);
    }

    signal_parameters sig_params;
    sig_params.on = 1;
    sig_params.data = 48;
    sig_params.period = PERIOD;

    ioctl(rx_proxy_fd, PROXY_DMA_SET_SIGNAL, &sig_params);

    ioctl(rx_proxy_fd, PROXY_DMA_CYCLIC_START, &dummy);

    clock_t begin = clock();
    while(!stop){
        clock_t end = clock();
        double elapsed_secs = (double)(end - begin) / CLOCKS_PER_SEC;
        if(elapsed_secs > timeout) break;
        //readStatus();
    }

    ioctl(rx_proxy_fd, PROXY_DMA_CYCLIC_STOP, &dummy);


    //memdump(rx_proxy_interface_p->buffer, rx_proxy_interface_p->length * rx_proxy_interface_p->buf_num);
    //memdump_file((void*)rx_proxy_interface_p->buffer, rx_proxy_interface_p->length * rx_proxy_interface_p->buf_num, "cyclic.data");
    //check_buf((void*)rx_proxy_interface_p->buffer, rx_proxy_interface_p->length * rx_proxy_interface_p->buf_num);
}
