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

#include "dma-proxy.h"
#include <stdint.h>

#define GPIO_FIFO   88
#define GPIO_BASE   338
#define GPIO_DIR_IN 0
#define GPIO_DIR_OUT 1

#define PERIOD 2
#define DUMP_SIZE 128*1024*1024
#define TIMEOUT 6

static int test_size;

void GPIO(int pin, int dir);
void GPIOout(int pin, int val);
int GPIOin(int pin);
void deinitGPIO(int pin);

void readStatus();

char sbuff[8192*2];
volatile int evt_count = 0;
int max_evt_count = 0;
struct dma_proxy_channel_interface *rx_proxy_interface_p;
int rx_proxy_fd;
int dummy;
int counter;
int max_counter;
pthread_t tid;
void* buff_ptr[1024];
char* dumpbuf;
uint32_t dumpbuf_it = 0;
uint8_t stop = 0;
uint32_t* status_reg;
uint64_t fifo_full = 0, fifo_empty = 0;

void check_buf(void* ptr, int size);
void init(int bn, int len);
void readSync();
void readLoop(int c, int timeout);
void readSyncAllBuffers();
void startCyclic(int c, int timeout);
void memdump(void* ptr, int size);
void memdump_file(void* ptr, uint64_t size, char* name);
void memdump_8_files(void* ptr, int size, char* name);
void fast_check(void* ptr, int size);
void test();
void find_min_max(void* ptr, int size);
void memdump_TCP(void* ptr, int size, char* addr);

void thread_f() {
    printf("thread_f start %d\n", evt_count);

    while (evt_count < 1000);

    while (stop != 1) {
        if (status_reg[0] & (1 << 29)) {
            fifo_full++;
        }
        if (status_reg[0] & (1 << 28)) {
            fifo_empty++;
        }
    }
}

void callbackf(int n, siginfo_t *info, void *unused) {

    if (max_evt_count > 0) {
        if (evt_count >= max_evt_count) {
            //ioctl(rx_proxy_fd, PROXY_DMA_CYCLIC_STOP, &dummy);
            stop = 1;
        }
    }
    //printf("evt\n");
    evt_count++;
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        printf("usage: NUT8_DMA2TCP address\n");
        return 0;
    }

    init(8000, 65536);


    pthread_t th;
    pthread_create(&th, NULL, thread_f, NULL);

    printf("generate ptr table\n");
    for (int i=0; i<1024; i++) {
        buff_ptr[i] = (void*)rx_proxy_interface_p->buffer + i * rx_proxy_interface_p->length;
    }

    //test();

    startCyclic(rx_proxy_interface_p->buf_num*4, TIMEOUT);

    memdump_TCP((void*)rx_proxy_interface_p->buffer, rx_proxy_interface_p->length * rx_proxy_interface_p->buf_num, argv[1]);
    //memdump_file((void*)rx_proxy_interface_p->buffer, rx_proxy_interface_p->length * rx_proxy_interface_p->buf_num, "fullb.data");
    //memdump_file((void*)rx_proxy_interface_p->buffer, rx_proxy_interface_p->length * rx_proxy_interface_p->buf_num / 2, "fullb.data1");
    //memdump_8_files((void*)rx_proxy_interface_p->buffer, rx_proxy_interface_p->length * rx_proxy_interface_p->buf_num, "fullb.data");
    //find_min_max((void*)rx_proxy_interface_p->buffer, rx_proxy_interface_p->length * rx_proxy_interface_p->buf_num);
    //check_buf((void*)rx_proxy_interface_p->buffer, rx_proxy_interface_p->length * rx_proxy_interface_p->buf_num);
    //memdump((void*)rx_proxy_interface_p->buffer, rx_proxy_interface_p->length * rx_proxy_interface_p->buf_num);
    //readSyncAllBuffers();
    //memdump_file((void*)rx_proxy_interface_p->buffer, rx_proxy_interface_p->length * rx_proxy_interface_p->buf_num, "fullb.data");

    printf("exp = %d\n", rx_proxy_interface_p->buf_num*2*PERIOD);
    munmap(rx_proxy_interface_p, sizeof(struct dma_proxy_channel_interface));
    close(rx_proxy_fd);

    GPIOout(GPIO_FIFO, 0);
    deinitGPIO(GPIO_FIFO);
    //readStatus();
    printf("DMA proxy test complete\nevt_count = %d\n", evt_count*PERIOD);

    printf("FIFO full: %lu\n", fifo_full);
    printf("FIFO empty: %lu\n", fifo_empty);

    //memdump_file(dumpbuf, DUMP_SIZE, "data.data");
    //check_buf(dumpbuf, DUMP_SIZE);

    //check_buf(sbuff, sizeof (sbuff));
    return 0;
}

void memdump_TCP(void* ptr, int size, char* addr) {
    int sockfd, connfd;
    struct sockaddr_in servaddr, cli;

    // socket create and varification
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("socket creation failed...\n");
        return;
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
        //exit(0);
        return;
    }
    else
        printf("connected to the server..\n");

    printf("Sending..\n");
    write(sockfd, ptr, size);
    printf("Done!\n");
    close(sockfd);
}

void GPIO(int pin, int dir) {

    char buffer[100];
    int fd = 0;

    fd = open("/sys/class/gpio/export", O_WRONLY);

    if (fd < 0){
        sprintf(buffer, " Can't open /sys/class/gpio/export for GPIO %d", pin);
        perror(buffer);
    }

    char gpio_num[4];
    sprintf(gpio_num, "%03d", pin + GPIO_BASE);

    int res = write(fd, gpio_num, 4);

    close(fd);
    if(res < 0){
        sprintf(buffer, " Can't write /sys/class/gpio/export for GPIO %d", pin);
        perror(buffer);
        //perror(" Can't write /sys/class/gpio/export");
        return;
    }



    sprintf(buffer, "/sys/class/gpio/gpio%d/direction", pin + GPIO_BASE);

    fd = 0;
    fd = open(buffer, O_WRONLY);

    if(fd <= 0){
        perror(" Failed to open GPIO");
    }

    if (dir == GPIO_DIR_OUT)
        res = write(fd, "out", 4);
    else
        res = write(fd, "in", 3);

    close(fd);
}


void GPIOout(int pin, int val) {
    char buffer[100];
    sprintf(buffer, "/sys/class/gpio/gpio%d/value", pin + GPIO_BASE);

    int fd = 0;
    fd = open(buffer, O_WRONLY);

    if(fd <= 0){
        perror(" Failed to write GPIO");
        return;
    }

    char dir_str[2] = {0};
    dir_str[0] = '0' + val;
    int res = write(fd, &dir_str, 2);

    close(fd);
}

int GPIOin(int pin) {
    char buffer[100];
    sprintf(buffer, "/sys/class/gpio/gpio%d/value", pin + GPIO_BASE);

    int fd = 0;
    fd = open(buffer, O_RDONLY);

    if(fd <= 0){
        perror(" Failed to read GPIO");
    }

    char ch;
    read(fd, &ch, 1);
    ch -= '0';

    close(fd);

    return ch;
}

void deinitGPIO(int pin) {
    char buffer[100];
    int fd = open("/sys/class/gpio/unexport", O_WRONLY);

    if (fd < 0){
        sprintf(buffer, " Can't open /sys/class/gpio/unexport for GPIO %d", pin);
        perror(buffer);
    }

    char gpio_num[4];
    sprintf(gpio_num, "%03d", pin + GPIO_BASE);
    int res = write(fd, gpio_num, 4);

    close(fd);
    if(res < 0){
        sprintf(buffer, " Can't write /sys/class/gpio/unexport for GPIO %d", pin);
        perror(buffer);
        //perror(" Can't write /sys/class/gpio/unexport");
        return;
    }
}

void init(int bn, int len) {
    printf("DMA init\n");
    int fd_mem = open("/dev/mem", O_RDWR | O_SYNC);
    status_reg = (uint32_t*)mmap(NULL, 4, PROT_READ | PROT_WRITE, MAP_SHARED, fd_mem, 0xA0010000L);
    printf("Statusreg[%d] = 0x%X\n",0 , status_reg[0]);

    //for (int i=0; i<10; i++)
    //    printf("Statusreg[%d] = 0x%X\n",i , status_reg[i]);
    //return 0;


    GPIO(GPIO_FIFO, GPIO_DIR_OUT);
    GPIOout(GPIO_FIFO, 1);
    printf("DMA proxy test\n");
    printf("Buffer size = %d\n", bn * len);
    //readStatus();

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

    printf(" == %d %d ==\n", rx_proxy_interface_p->buf_num, rx_proxy_interface_p->length);
}

void find_min_max(void* ptr, int size) {
    int16_t* ptr16 = ptr;
    int size16 = size/2;
    int16_t min, max;
    min = max = ptr16[0];

    for (int i=0; i<size16; i++) {
        if (ptr16[i] < min)
            min = ptr16[i];

        if (ptr16[i] > max)
            max = ptr16[i];
    }

    printf("min = %d (0x%.4X), max = %d (0x%.4X)\n", min, min, max, max);
}

void readSyncAllBuffers() {
    printf("ioctl(rx_proxy_fd, 0, &dummy)\n");
    ioctl(rx_proxy_fd, PROXY_DMA_SYNC_READ_ALL_BUFFERS, &dummy);

    int ret = rx_proxy_interface_p->status;
    if (ret != PROXY_NO_ERROR) {
        printf("Proxy rx transfer error %d\n", ret);
    }

    for (int i=0; i<rx_proxy_interface_p->buf_num; i++) {
        //printf(" ========= BUF #%d ========\n", i);
        //memdump(rx_proxy_interface_p->buffer + rx_proxy_interface_p->length * i, rx_proxy_interface_p->length);
        //fast_check((void*)rx_proxy_interface_p->buffer + rx_proxy_interface_p->length*i, rx_proxy_interface_p->length);
        //if (dumpbuf_it < DUMP_SIZE) {
        //    memcpy(dumpbuf + dumpbuf_it, (void*)rx_proxy_interface_p->buffer + rx_proxy_interface_p->length*i, rx_proxy_interface_p->length);
        //    dumpbuf_it += rx_proxy_interface_p->length;
        //}
        //check_buf(rx_proxy_interface_p->buffer + rx_proxy_interface_p->length * i, rx_proxy_interface_p->length);
    }
}

void startCyclic(int c, int timeout) {
    printf("startCyclic start\n");
    struct sigaction sig;
    sig.sa_sigaction = callbackf;
    sig.sa_flags = SA_SIGINFO;
    if( sigaction(SIGIO, &sig, NULL) < 0 ){
        perror("sigaction");
        exit(1);
    }

    max_evt_count = c;

    signal_parameters sig_params;
    sig_params.on = 1;
    sig_params.data = 48;
    sig_params.period = PERIOD;

    ioctl(rx_proxy_fd, PROXY_DMA_SET_SIGNAL, &sig_params);

    ioctl(rx_proxy_fd, PROXY_DMA_CYCLIC_START, c);

    clock_t begin = clock();
    clock_t end;
    while(stop == 0){
        end = clock();
        double elapsed_secs = (double)(end - begin) / CLOCKS_PER_SEC;
        if(elapsed_secs > timeout) break;
        //readStatus();
    }
    printf("Time: %f\n", (double)(end - begin) / CLOCKS_PER_SEC);
    ioctl(rx_proxy_fd, PROXY_DMA_CYCLIC_STOP, &dummy);
    stop = 1;

    //memdump(rx_proxy_interface_p->buffer, rx_proxy_interface_p->length * rx_proxy_interface_p->buf_num);
    //memdump_file((void*)rx_proxy_interface_p->buffer, rx_proxy_interface_p->length * rx_proxy_interface_p->buf_num, "cyclic.data");
    //check_buf((void*)rx_proxy_interface_p->buffer, rx_proxy_interface_p->length * rx_proxy_interface_p->buf_num);
}

void memdump(void* ptr, int size) {
    uint8_t* ptr8 = ptr;
    for (int i = 0; i < size; i+=2) {
        if (i%32 == 0)
            printf("\n0x%.4X", i);

        if (i%8 == 0)
            printf(" | ");

        printf("%.2X%.2X ", ptr8[i+1], ptr8[i]);

        if (i%32 == 30) {
            uint16_t l = *((uint16_t*)&ptr8[i]);
            uint16_t f = *((uint16_t*)&ptr8[i - 30]);
            printf("    %s", (l - f) == 3 ? " ": "<< ! >>");
            if (i > 32) {
                f = *((uint16_t*)&ptr8[i - 32]);
                printf("    %s", (l - f) == 4 ? " ": " ^^ ! ^^ ");
            }
        }
    }
    printf("\n");
}

void memdump_file(void* ptr, uint64_t size, char* name) {
    FILE* f;
    f = fopen(name, "wb");

    printf("memdump_file copying...\n");
    uint64_t copy_size = 1024*1024*16;
    uint8_t* ptr8 = ptr;
    uint64_t ret;

    for (uint64_t i=0; i<size; i+= copy_size) {
        printf("  %d%\n", i*100/size);
        if (i + copy_size >= size) {
            ret = fwrite(ptr8 + i, 1, size - i, f);

            if (ret !=  size - i) {
                printf("memdump_file copying error: %lu %lu\n", ret, size - i);
            }
        }
        else {
            ret = fwrite(ptr8 + i, 1, copy_size, f);

            if (ret !=  copy_size) {
                printf("memdump_file copying error: %lu %lu\n", ret, copy_size);
            }
        }

    }
    fclose(f);
}

void memdump_8_files(void* ptr, int size, char* name) {
    FILE* f[8];
    int size16 = size/2;

    printf("memdump_8_files init files\n");
    for (int i=0; i<8; i++) {
        char nameB[128];
        sprintf(nameB, "%s%d", name, i);
        f[i] = fopen(nameB, "w");
    }

    printf("memdump_8_files init buffers\n");
    uint16_t** buffs = (uint16_t**) malloc(sizeof(uint16_t*) * 8);
    for (int i=0; i<8; i++) {
        buffs[i] = (uint16_t*) malloc(size16/8 * sizeof(uint16_t));
        if (buffs[i] == 0) {
            printf("memdump_8_files buffs[%d] malloc error\n", i);
            return;
        }
    }

    printf("memdump_8_files copy to buffers\n");
    uint16_t* ptr16 = ptr;
    for (int i=0; i<size16; i++) {
        buffs[i%8][i/8] = ptr16[i];
    }

    printf("memdump_8_files copy to file\n");
    for (int i=0; i<8; i++) {
        fwrite((char*)(buffs[i]), 1, size/8, f[i]);
        fclose(f[i]);
        free(buffs[i]);
    }
}

void check_buf(void* ptr, int size) {
    uint16_t* ptr16 = ptr;
    int size16 = size/2;

    for (int i = 0; i < size16 - 16; i+=16) {

        uint16_t l1 = ptr16[i + 15];
        uint16_t l2 = ptr16[i + 16];
        uint16_t f = ptr16[i];

        if ((uint16_t)(ptr16[i] + 1) != ptr16[i + 15]) {
            printf("inline error l=%.2X f=%.2X\n", l1, f);
            printf("0x%.5X", i*2);
            for (int j=0; j<16; j++) {
                if (j%4 == 0)
                    printf(" | ");
                printf("%.4X ", ptr16[i + j]);
            }
            printf("\n");
        }
        if ((uint16_t)(ptr16[i] + 2) != ptr16[i + 16]) {
            printf("Between line error");
            for (int j=0; j<32; j++) {
                if (j%16 == 0)
                    printf("\n0x%.5X", i*2);
                if (j%4 == 0)
                    printf(" | ");
                printf("%.4X ", ptr16[i + j]);

            }
            printf("\n");
        }
    }
    printf("\n");
}

void fast_check(void* ptr, int size) {
    uint16_t* ptr16 = ptr;
    uint32_t size16 = size/2;
    //uint16_t f = ptr16[0];
    //uint16_t l = ptr16[size16 -1];

    if ((uint16_t)(ptr16[0] + size16/4 - 1) !=  ptr16[size16 -1]) {
        printf("err first = 0x%X, expect = 0x%X, last = 0x%X, evt = %d\n", ptr16[0], ptr16[0] + size16/4 -1, ptr16[size16 -1], evt_count);
        //memcpy(sbuff, ptr, size);
        //f = ptr16[0];
        //l = ptr16[size16 -1];
        //printf("    first = 0x%X, expect = 0x%X, last = 0x%X, evt = %d\n", f, f + size16/4 -1, l, evt_count);
//        if (evt_count > 100) {
//            ioctl(rx_proxy_fd, PROXY_DMA_CYCLIC_STOP, &dummy);
//            stop = 1;
//        }
    }

}
