#ifndef SPI_FUNSTIONS_H_
#define SPI_FUNSTIONS_H_

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

int spi_write_byte(int fd, uint8_t addr, uint8_t data);
int spi_read_byte(int fd, uint8_t addr, uint8_t* data);
// int spi_write_word(int fd, uint32_t data);
int spi_transaction(int fd, uint8_t* tx_buf, uint8_t* rx_buf, int length);

int print_register(int fd, uint8_t addr);
int print_registers(int fd, uint8_t addr_start, uint8_t addr_end);

int open_bus(int major, int minor);
// void print_state();

// void read_reg(uint32_t addr);
// void read_regs(uint32_t addr_start, uint32_t addr_end);
// void write_reg(uint32_t addr, uint32_t data);

#endif
