#ifndef ADS_5292_H_
#define ADS_5292_H_

#include <stdint.h>

int ads_config(int fd_spi);
void ads_hardreset();

int ReadOutOnADS(int fd);
int ReadOutoffADS(int fd);

void write_to_ads(int fd, uint8_t addr, uint16_t data);
uint16_t read_from_ads(int fd, uint8_t addr);

void fifo_enable();
void pl_axistream_counter_enable();

#endif
