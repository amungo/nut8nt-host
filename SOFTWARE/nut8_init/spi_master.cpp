#include "spi_master.h"


SPI_master::SPI_master(char* _dev) {
    std::cout << "Open SPI bus " << _dev << std::endl;

    fd = open(_dev, O_RDWR);
    if(fd < 0) {
        perror(" Failed to open SPI bus");
        exit(1);
    }

    dev = _dev;
}

SPI_master::~SPI_master() {
    if (fd)
        close(fd);
}

int32_t SPI_master::openDevice(char *_dev) {
    std::cout << "Open SPI bus " << _dev << std::endl;

    fd = open(_dev, O_RDWR);
    if(fd < 0) {
        perror(" Failed to open SPI bus");
        exit(1);
    }
    dev = _dev;

    return fd;
}

int32_t SPI_master::writeReg(uint8_t _address, uint8_t _data) {
    uint8_t tx_buf[2];
    tx_buf[0] = _address;
    tx_buf[1] = _data;

    uint8_t rx_buf[2];
    rx_buf[0] = 0;
    rx_buf[1] = 0;

    struct spi_ioc_transfer tr;

    tr.tx_buf = (unsigned long)tx_buf;
    tr.rx_buf = (unsigned long)rx_buf;
    tr.len = 2;
    tr.cs_change = 0;
    tr.delay_usecs = delay;
    tr.speed_hz = speed;
    tr.bits_per_word = bits;


    int res = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
    if (res < 1){
        perror("Can't send spi message");
    }

    return res;
}

int32_t SPI_master::readReg(uint8_t _address, uint8_t *_data) {
    uint8_t tx_buf[2];
    tx_buf[0] = _address | READ_SPI_OP;
    tx_buf[1] = 0xFF;
    // tx_buf[2] = 0xFF;

    uint8_t rx_buf[2];
    rx_buf[0] = 0;
    rx_buf[1] = 0;
    // rx_buf[2] = 0;

    struct spi_ioc_transfer tr;

    tr.tx_buf = (unsigned long)tx_buf;
    tr.rx_buf = (unsigned long)rx_buf;
    tr.len = 2;
    tr.cs_change = 0;
    tr.delay_usecs = delay;
    tr.speed_hz = speed;
    tr.bits_per_word = bits;


    int res = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
    if (res < 1){
        perror("Can't send spi message");
    }

    *_data = rx_buf[1];

    return res;
}

int32_t SPI_master::transmitData(uint8_t* _tx_buf, uint8_t* _rx_buf, int length){

    struct spi_ioc_transfer tr;

    tr.tx_buf = (unsigned long)_tx_buf;
    tr.rx_buf = (unsigned long)_rx_buf;
    tr.len = length;
    tr.cs_change = 0;
    tr.delay_usecs = delay;
    tr.speed_hz = speed;
    tr.bits_per_word = bits;

    int res = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
    if (res < 1){
        std::cout << " Error on transmit data " << std::endl;
        perror("Can't send spi message");
    }

    return res;
}
