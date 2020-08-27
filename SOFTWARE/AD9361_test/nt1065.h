#include <unistd.h>
#include <stdint.h>
#include "gpio_functions.h"


#ifndef SRC_NT1065_H_
#define SRC_NT1065_H_

#define READ_NT_SPI_OP		0x80
#define WRITE_NT_SPI_OP		0x00

typedef struct {
	uint8_t addr;
	uint8_t data;
} spi_register_t;

void nt1065_config(int fd);
void nt1065_hardreset();

#endif
