#include "axidma.h"

uint32_t DMAReader::callbackCount[DMA_NUM];
volatile uint32_t DMAReader::iterLast[DMA_NUM];
volatile uint32_t DMAReader::iterFirst[DMA_NUM];
int DMAReader::blockCount[DMA_NUM] = {0};
size_t DMAReader::blockSize[DMA_NUM] = {0};
int DMAReader::SUPBUFF_COUNT[DMA_NUM] = {1};

void printConfigs() {
    std::cout << " == " << NUT_NAME << std::endl;
    std::cout << " == Release " << __DATE__ << std::endl;
    std::cout << " == DMA_BUF_SIZE \t" << DMA_BUF_SIZE << std::endl;
    std::cout << " == SUPER_BUFFER_SIZE \t" << SUPER_BUFFER_SIZE << std::endl;
    std::cout << " == CHANNELS_NUM \t" << CHANNELS_NUM << std::endl;
    std::cout << " == SAMPLE_SIZE \t" << SAMPLE_SIZE << std::endl;
    std::cout << std::endl;
}

int CYCLIC_DISTANCE(int a, int b, int r){
    int ret = b - a;

    if (ret < 0)
        ret += r;

    return ret;
}

DMAReader::DMAReader(size_t bs, int bc, std::string dmaName) : FIFORun(GPIO_PIN_FIFO_RUN, GPIO_DIR_OUT){
    if (dmaName == "dma_proxy_rx1") {
        dma_num = 0;
    }

    if (dmaName == "dma_proxy_tx1") {
        dma_num = 1;
    }

    if (dmaName == "dma_proxy_rx2") {
        dma_num = 2;
    }

    if (dmaName == "dma_proxy_tx2") {
        dma_num = 3;
    }

    blockCount[dma_num] = bc;
    blockSize[dma_num] = bs;

    if (dmaName == "dma_proxy_tx1")
        SUPBUFF_COUNT[dma_num] = 1;
    else
        SUPBUFF_COUNT[dma_num] = BUFF_COUNT / blockCount[dma_num];

    init(bs, dmaName);

    buff_ptr = new void*[SUPBUFF_COUNT[dma_num]];

    for (int i=0; i<SUPBUFF_COUNT[dma_num]; i++) {
        buff_ptr[i] = (char*)&(rx_proxy_interface_p->buffer[i * bs * blockCount[dma_num]]);
    }
}

DMAReader::~DMAReader() {
    finish();

    munmap(rx_proxy_interface_p, sizeof(struct dma_proxy_channel_interface));
    close(rx_proxy_fd);

    delete [] buff_ptr;
}

void DMAReader::reset() {
    ioctl(rx_proxy_fd, PROXY_DMA_RESET, 0);
}

void DMAReader::init(size_t bs, std::string dmaName) {
    std::cout << "DMA init" << std::endl;
    std::cout << "blockCount = " << blockCount[dma_num] << std::endl;
    std::cout << "blockSize = " << blockSize[dma_num] << std::endl;
    std::cout << "SUPBUFF_COUNT = " << SUPBUFF_COUNT[dma_num] << std::endl;

    std::string path = "/dev/" + dmaName;

    std::cout << "Opening dma " << path << std::endl;

    rx_proxy_fd = open(path.data(), O_RDWR);
    if (rx_proxy_fd < 1) {
        perror("Unable to open DMA proxy device file ");
        exit(EXIT_FAILURE);
    }

    std::cout << "mmap dma " << path << std::endl;
    rx_proxy_interface_p = (struct dma_proxy_channel_interface *)mmap(NULL, sizeof(struct dma_proxy_channel_interface),
                                    PROT_READ | PROT_WRITE, MAP_SHARED, rx_proxy_fd, 0);
    if (rx_proxy_interface_p == MAP_FAILED) {
        perror("mmap tx_proxy_interface_p");
        exit(EXIT_FAILURE);
    }

    printf(" == %X == \n", sizeof(struct dma_proxy_channel_interface));

    if (dmaName == "dma_proxy_tx1")
        return;

    rx_proxy_interface_p->length = bs;
    rx_proxy_interface_p->buf_num = BUFF_COUNT;

    //memset(rx_proxy_interface_p->buffer, 0, DMA_BUFFER_SIZE);


    struct sigaction sig;
    if (dmaName == "dma_proxy_rx1") {
        sig.sa_sigaction = &DMAReader::callbackf0;
    }

    if (dmaName == "dma_proxy_tx1") {
        return;
        sig.sa_sigaction = &DMAReader::callbackf1;
    }

    if (dmaName == "dma_proxy_rx2") {
        //return;
        sig.sa_sigaction = &DMAReader::callbackf2;
    }

    if (dmaName == "dma_proxy_tx2") {
        sig.sa_sigaction = &DMAReader::callbackf3;
    }


    sig.sa_flags = SA_SIGINFO;
    int sigNum = SIGRTMIN + dma_num;

    if( sigaction(sigNum, &sig, NULL) < 0 ){
        perror("sigaction");
        exit(1);
    }

    signal_parameters sig_params;
    sig_params.on = sigNum;
    sig_params.data = 3;
    sig_params.period = blockCount[dma_num];

    std::cout << "Setting signal" << std::endl;

    ioctl(rx_proxy_fd, PROXY_DMA_SET_SIGNAL, &sig_params);
}

void DMAReader::callbackf0(int n, siginfo_t *info, void *unused) {
    const int _n = 0;
    //std::cout << "callbackf0, l: " << iterLast[_n] << ", f: " << iterFirst[_n] << std::endl;
    callbackCount[_n]++;

    iterLast[_n] = (iterLast[_n] + 1) % (SUPBUFF_COUNT[_n]);

    if (CYCLIC_DISTANCE(iterLast[_n], iterFirst[_n], SUPBUFF_COUNT[_n]) < 5) {
        std::cout << "To slow read speed for DMA" << _n <<  ". Error!" << std::endl;
        iterFirst[_n] = (iterLast[_n] - 20) % (SUPBUFF_COUNT[_n]);
    }
}

void DMAReader::callbackf1(int n, siginfo_t *info, void *unused) {
    const int _n = 1;
    //std::cout << "callbackf1, l: " << iterLast[_n] << ", f: " << iterFirst[_n] << std::endl;
    callbackCount[_n]++;

    iterLast[_n] = (iterLast[_n] + 1) % (SUPBUFF_COUNT[_n]);

    if (CYCLIC_DISTANCE(iterLast[_n], iterFirst[_n], SUPBUFF_COUNT[_n]) < 5) {
        std::cout << "To slow read speed for DMA" << _n <<  ". Error!" << std::endl;
        iterFirst[_n] = (iterLast[_n] - 20) % (SUPBUFF_COUNT[_n]);
    }
}

void DMAReader::callbackf2(int n, siginfo_t *info, void *unused) {
    const int _n = 2;
    //std::cout << "callbackf2, l: " << iterLast[_n] << ", f: " << iterFirst[_n] << std::endl;
    callbackCount[_n]++;

    iterLast[_n] = (iterLast[_n] + 1) % (SUPBUFF_COUNT[_n]);

    if (CYCLIC_DISTANCE(iterLast[_n], iterFirst[_n], SUPBUFF_COUNT[_n]) < 5) {
        std::cout << "To slow read speed for DMA" << _n <<  ". Error!" << std::endl;
        iterFirst[_n] = (iterLast[_n] - 20) % (SUPBUFF_COUNT[_n]);
    }
}

void DMAReader::callbackf3(int n, siginfo_t *info, void *unused) {
    const int _n = 3;
    //std::cout << "callbackf3, l: " << iterLast[_n] << ", f: " << iterFirst[_n] << std::endl;
    callbackCount[_n]++;

    iterLast[_n] = (iterLast[_n] + 1) % (SUPBUFF_COUNT[_n]);

    if (CYCLIC_DISTANCE(iterLast[_n], iterFirst[_n], SUPBUFF_COUNT[_n]) < 5) {
        std::cout << "To slow read speed for DMA" << _n <<  ". Error!" << std::endl;
        iterFirst[_n] = (iterLast[_n] - 20) % (SUPBUFF_COUNT[_n]);
    }
}

void DMAReader::write(char *data, uint32_t size) {

//    int div = 16;

//    for (int i = 0; i < size / div; i++) {
//        printf("%.8X | ", i * div);
//        for (int j = 0; j < div; j++) {
//            if (data[i * div + j])
//                printf("%.2X ", data[i * div + j]);
//            else
//                printf("-- ");
//        }
//        printf("\n");
//    }

    memcpy(rx_proxy_interface_p->buffer, data, size);
    ioctl(rx_proxy_fd, PROXY_DMA_WRITE, size);
}

void DMAReader::start() {
    callbackCount[dma_num] = 0;
    iterLast[dma_num] = 0;
    iterFirst[dma_num] = 0;

    //printf("!!!!%d %d!!!!!!!\n", rx_proxy_interface_p->length, rx_proxy_interface_p->buf_num);

    FIFORun.out(0);

    int dummy;
    ioctl(rx_proxy_fd, PROXY_DMA_CYCLIC_STOP, &dummy);
    ioctl(rx_proxy_fd, PROXY_DMA_CYCLIC_START, 0);

    usleep(1);
    FIFORun.out(1);
}

void DMAReader::finish() {
    FIFORun.out(0);
    int dummy;
    ioctl(rx_proxy_fd, PROXY_DMA_CYCLIC_STOP, &dummy);
}

void DMAReader::tryRead() {
    int dummy;
    ioctl(rx_proxy_fd, PROXY_DMA_SYNC_READ_ALL_BUFFERS, &dummy);
}

uint32_t DMAReader::getClbCount() {
    return callbackCount[dma_num];
}

uint32_t DMAReader::getAvalableBuffsCount() {
    return CYCLIC_DISTANCE(iterFirst[dma_num], iterLast[dma_num], SUPBUFF_COUNT[dma_num]);
}

void* DMAReader::getBeginPtr() {
    return buff_ptr[0];
}

void* DMAReader::read() {
    if (CYCLIC_DISTANCE(iterFirst[dma_num], iterLast[dma_num], SUPBUFF_COUNT[dma_num]) < 1) {
        return nullptr;
    }

    //std::cout << iterFirst << std::endl;

    void* ret = buff_ptr[iterFirst[dma_num]];

    iterFirst[dma_num] = (iterFirst[dma_num] + 1) % (SUPBUFF_COUNT[dma_num]);

    return ret;
}
