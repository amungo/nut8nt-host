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
#include <time.h>

#include "dma-proxy.h"
#include <stdint.h>

#define GPIO_FIFO       88
#define GPIO_BASE       338
#define GPIO_DIR_IN     0
#define GPIO_DIR_OUT    1

#define PERIOD      80
#define DIV         1
#define BUFF_COUNT  (8000)
#define BUFF_SIZE   (1024*32)
#define TIMEOUT     40

#define USE_SERVER
//#define TEST_COUNTER

static int test_size;

void GPIO(int pin, int dir);
void GPIOout(int pin, int val);
int GPIOin(int pin);
void deinitGPIO(int pin);

void readStatus();
int sockfd;
volatile int evt_count = 0;
int max_evt_count = 0;
struct dma_proxy_channel_interface *rx_proxy_interface_p;
int rx_proxy_fd;
pthread_t tid;
unsigned char* buff_ptr[BUFF_COUNT/PERIOD];
char sendbuff[BUFF_SIZE * PERIOD];
volatile uint8_t stop = 0;
uint32_t* status_reg;
uint64_t fifo_full = 0, fifo_empty = 0;
int err = 0;
uint64_t total_send = 0;

#ifdef TEST_COUNTER
uint32_t last = 0;
#endif
int iter = 0;

void init(int bn, int len);
void startCyclic(int c, int timeout);
int connect_to_server(char* addr);

void thread_f() {
    int strSize;
    char str[32];
    uint32_t totalSendLast = 0;
    printf("Send speed: ");
    sleep(2);

    while (!stop) {
        sprintf(str,"%d MByte/s", (total_send - totalSendLast)/1024/1024);
        strSize = strlen(str);
        printf("%s", str);

        totalSendLast = total_send;
        fflush(stdout);


        sleep(1);
        if (stop)
            break;

        for (int i=0; i<strSize; i++)
            printf("%c", '\b');
    }
}

int ctrlC_count = 0;
void sig_int_handler(int signo) {
    printf("Stopping programm\n");

    stop = 1;
    if (sockfd)
        close(sockfd);

    ctrlC_count++;

    if (ctrlC_count == 2)
        exit(-1);
}


void callbackf(int n, siginfo_t *info, void *unused) {
    if (max_evt_count > 0) {
        if (evt_count >= max_evt_count) {
            stop = 1;
        }
    }

    int iter_test;
    if (evt_count > 10) {
        iter_test = iter;
        iter = (iter + 1) % (BUFF_COUNT/PERIOD);
    }

#ifdef TEST_COUNTER
    uint32_t first = ((uint32_t*)(buff_ptr[iter]))[0];
    //uint32_t last = ((uint32_t*)(buff_ptr[iter_test]))[0];


    if (first - last != BUFF_SIZE/16*PERIOD && evt_count > 100) {
        printf (" dif(f-l) = %X, index = %X, f = %X, l = %X, exp = %X\n", first - last, evt_count, first, last, BUFF_SIZE/32*PERIOD);
        fifo_empty++;
    }

    last = first;
#endif

#ifdef USE_SERVER
    int it = 0;

    if (evt_count > 100) {
        int ret = send(sockfd, buff_ptr[iter], BUFF_SIZE * PERIOD, 0);
        if (ret < BUFF_SIZE * PERIOD) {
            printf("send err %d\n", ret);
        }
        total_send += ret;
    }
#endif

    evt_count++;
}

int main(int argc, char *argv[])
{
    signal(SIGINT, sig_int_handler);
    signal(SIGTERM, sig_int_handler);
#ifdef USE_SERVER
    if (argc < 2) {
        printf("usage: %s addr\n", argv[0]);
        return 0;
    }

    if (connect_to_server(argv[1]) != 0) {
        return 0;
    }
#endif
    uint8_t b = 0;
    for (int i=0; i<sizeof(sendbuff); i++) {
        sendbuff[i] = b++;
    }

    init(BUFF_COUNT, BUFF_SIZE);

    pthread_t th;
    pthread_create(&th, NULL, &thread_f, NULL);

    printf("generate ptr table\n");
    for (int i=0; i<BUFF_COUNT/PERIOD; i++) {
        buff_ptr[i] = rx_proxy_interface_p->buffer + i * BUFF_SIZE * PERIOD;
    }

    printf("start cyclic DMA\n");
    startCyclic(rx_proxy_interface_p->buf_num*0, TIMEOUT);

    munmap(rx_proxy_interface_p, sizeof(struct dma_proxy_channel_interface));
    close(rx_proxy_fd);

    GPIOout(GPIO_FIFO, 0);
    deinitGPIO(GPIO_FIFO);

    printf("DMA proxy test complete\nevt_count = %d\n", evt_count*PERIOD);
    printf("Total send %ld MByte\n", total_send/1024/1024);

    printf("FIFO full: %lu\n", fifo_full);
    printf("FIFO empty: %lu\n", fifo_empty);
    printf("FIFO err: %lu\n", err);
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

    int chan = 4;
    write(sockfd, &chan, sizeof(chan));

    return 0;
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

    GPIO(GPIO_FIFO, GPIO_DIR_OUT);
    GPIOout(GPIO_FIFO, 1);
    printf("DMA proxy TCP dumper\n");
    printf("Buffer size = %d\n", bn * len);

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
    sig_params.data = 3;
    sig_params.period = PERIOD/DIV;

    ioctl(rx_proxy_fd, PROXY_DMA_SET_SIGNAL, &sig_params);

    ioctl(rx_proxy_fd, PROXY_DMA_CYCLIC_START, c);

    struct timespec begin;
    struct timespec end;
    clock_gettime(CLOCK_REALTIME, &begin);
    while(stop == 0){
        clock_gettime(CLOCK_REALTIME, &end);
        double elapsed_secs = (end.tv_sec - begin.tv_sec) + (end.tv_nsec - begin.tv_nsec) / 1e9;

        usleep(10);

        if (timeout)
            if(elapsed_secs > timeout) break;
    }
    double time = (end.tv_sec - begin.tv_sec) + (end.tv_nsec - begin.tv_nsec) / 1e9;
    printf("Time: %f %f\n", time, time*80);
    int dummy;
    long long evt_c = ioctl(rx_proxy_fd, PROXY_DMA_CYCLIC_STOP, &dummy);
    printf("Data speed: %f\n", evt_c*BUFF_SIZE/1024.0/1024.0/time*DIV);
    stop = 1;
}





