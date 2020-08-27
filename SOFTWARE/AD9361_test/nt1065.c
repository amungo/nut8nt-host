#include "nt1065.h"
#include "gpio_pins.h"
#include "spi_functions.h"


#define SIZE_NT_CONFIG 49


static spi_register_t nt_config_s [SIZE_NT_CONFIG] = {
    {.addr = 0	, .data = 0x00},
        {.addr = 1	, .data = 0x00},
        {.addr = 2	, .data = 0x03},
        {.addr = 3	, .data = 0x00},
        {.addr = 4	, .data = 0x01},
        {.addr = 5	, .data = 0x00},
        {.addr = 6	, .data = 0x1D},
        {.addr = 7	, .data = 0x00},
        {.addr = 8	, .data = 0x00},
        {.addr = 9	, .data = 0x00},
        {.addr = 10	, .data = 0x00},
        {.addr = 11	, .data = 0x0F},
        {.addr = 12	, .data = 0x18},
        {.addr = 13	, .data = 0x03},
        {.addr = 14	, .data = 0x4D},//5D
        {.addr = 15	, .data = 0x7A},
        {.addr = 16	, .data = 0x34},
        {.addr = 17	, .data = 0x00},
        {.addr = 18	, .data = 0x00},
        {.addr = 19	, .data = 0x0A},
        {.addr = 20	, .data = 0x03},
        {.addr = 21	, .data = 0x4D},//5D
        {.addr = 22	, .data = 0x5A}, //0x7A
        {.addr = 23	, .data = 0x34},
        {.addr = 24	, .data = 0x00},
        {.addr = 25	, .data = 0x00},
        {.addr = 26	, .data = 0x0A},
        {.addr = 27	, .data = 0x03},
        {.addr = 28	, .data = 0x4D},//5D
        {.addr = 29	, .data = 0x7A},
        {.addr = 30	, .data = 0x34},
        {.addr = 31	, .data = 0x00},
        {.addr = 32	, .data = 0x00},
        {.addr = 33	, .data = 0x0A},
        {.addr = 34	, .data = 0x03},
        {.addr = 35	, .data = 0x4D},//5D
        {.addr = 36	, .data = 0x7A},
        {.addr = 37	, .data = 0x34},
        {.addr = 38	, .data = 0x00},
        {.addr = 39	, .data = 0x00},
        {.addr = 40	, .data = 0x0A},
        {.addr = 41	, .data = 0x03},
        {.addr = 42	, .data = 0xA0},
        {.addr = 43	, .data = 0x91},
        {.addr = 44	, .data = 0x00},
        {.addr = 45	, .data = 0x00},
        {.addr = 46	, .data = 0x7B},
        {.addr = 47	, .data = 0x91},
        {.addr = 48	, .data = 0x00},
};

void nt1065_config(int fd) {

	int res = 0;
	for(int i = 0; i < SIZE_NT_CONFIG; ++i)	{
		res = spi_write_byte(fd, nt_config_s[i].addr, nt_config_s[i].data);
	}

	usleep(20000);

	uint8_t reg44 = 0;
	res = spi_read_byte(fd, 44, &reg44);
	if(reg44 != 0x1) {
		printf("[E] NT1065 PLL not locked. Reg44 = 0x%0x\r\n", reg44);
	} else {
		printf("[I] NT1065 conf done\r\n");
	}

	uint8_t reg9 = 0;
	res = spi_read_byte(fd, 9, &reg9);
	printf("[I] NT1065 RF_AGC Reg9 = 0x%0x\r\n", reg9);
}

void nt1065_hardreset() {
	pin_set(RCVENPIN, LOW);
	usleep(50000);
	pin_set(RCVENPIN, HIGH);
	usleep(300000);
}
