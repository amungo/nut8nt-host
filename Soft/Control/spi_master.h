#ifndef SPI_MASTER_H
#define SPI_MASTER_H

#include <stdint.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/spi/spidev.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <math.h>
#include <tgmath.h>
#include <iostream>

#define READ_SPI_OP		0x80

class SPI_master
{
protected:
    int fd;
    char* dev;

    uint8_t mode;
    uint8_t bits;
    uint8_t lsb;
    uint32_t speed;
    uint16_t delay;
public:
    SPI_master(char* dev);
    ~SPI_master();
    int32_t openDevice(char* dev);
    int32_t readReg(uint8_t _address, uint8_t* _data);
    int32_t transmitData(uint8_t* _tx_buf, uint8_t* _rx_buf, int length);
    int32_t writeReg(uint8_t _address, uint8_t _data);

    virtual void checkDevice() = 0;
};

#endif // SPI_MASTER_H
