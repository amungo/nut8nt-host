#include "axi_dma.h"

AXI_DMA::AXI_DMA() : fifo_run(GPIO_PIN_FIFO_RUN, GPIO_DIR_OUT) {
    rx_proxy_fd = open("/dev/dma_proxy_rx", O_RDWR);
    if (rx_proxy_fd < 1) {
        printf("Unable to open DMA proxy device file");
        return;
    }

    //ioctl(rx_proxy_fd, PROXY_DMA_LOOP_STOP, 0);

    rx_proxy_interface_p = (struct dma_proxy_channel_interface *)mmap(NULL, sizeof(struct dma_proxy_channel_interface),
                                    PROT_READ | PROT_WRITE, MAP_SHARED, rx_proxy_fd, 0);
    if (rx_proxy_interface_p == MAP_FAILED) {
        perror("mmap tx_proxy_interface_p");
        return;
    }
}

AXI_DMA::~AXI_DMA() {
    munmap(rx_proxy_interface_p, sizeof(struct dma_proxy_channel_interface));
    close(rx_proxy_fd);
}

void AXI_DMA::test() {
    fifo_run.out(1);
    printf("DMA read:\n");
    rx_proxy_interface_p->length = 8192;
    rx_proxy_interface_p->buf_num = 4;
    ioctl(rx_proxy_fd, PROXY_DMA_SYNC_READ_ALL_BUFFERS, 0);

    switch (rx_proxy_interface_p->buffer[8192]) {
    case 0xF0:
        printf(" Detect sync pattern! (0xF0)\n");
        break;
    case 0x00:
        printf(" No data (0x00)\n");
        break;
    case 0x55:
        printf(" Detect 0101 pattern! (0x55)\n");
        break;
    default:
        printf(" Detect data! (0x%.2X)\n", rx_proxy_interface_p->buffer[8192]);
        break;
    }
    fifo_run.out(0);
}
