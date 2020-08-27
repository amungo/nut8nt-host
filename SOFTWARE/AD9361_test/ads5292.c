#include "ads5292.h"
#include "gpio_functions.h"
#include "gpio_pins.h"
#include "spi_functions.h"

#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>


#define BUFFER_SIZE 16


uint8_t ReadBuffer[BUFFER_SIZE];
uint8_t WriteBuffer[BUFFER_SIZE];

#define M_AXI_ADS_BASEADDR 0x80010000L
#define M_AXI_ADS_SIZE 0x10000
// #define M_AXI_ADS_SIZE 12

/*
typedef struct packed {
	logic [32 - 2 - 1 :0] dummy;
	logic locked;
	logic synced;
} status_t;

typedef struct packed {
	logic [32 - 1 - 1 - 3 - 1 - 1 - 1 -1 - 12:0] dummy ;
	logic [3:0] [2:0] mux_map;
	logic swap_iq;
	logic [1:0] ddc_gain;
	logic [$clog2(ADS_CH_NUM) -1 :0] mux;
	logic reset_io;
	logic sync;
} control_t;

typedef struct packed  {
	logic [31:0] phinc;
	status_t status ;
	control_t control;
} ads_regs_t;
*/

int ads_config(int fd){

	int fd_mem = open("/dev/mem", O_RDWR | O_SYNC);
	uint32_t* ads_ip_core_regs = (uint32_t*)mmap(NULL, M_AXI_ADS_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd_mem, M_AXI_ADS_BASEADDR);
    printf("ads_ip_core_regs=0x%X\n", ads_ip_core_regs);
	printf("ads_ip_core_regs[0]=0x%08X\n", ads_ip_core_regs[0]);
	printf("ads_ip_core_regs[1]=0x%08X\n", ads_ip_core_regs[1]);
	printf("ads_ip_core_regs[2]=0x%08X\n", ads_ip_core_regs[2]);
    printf("ads_ip_core_regs[3]=0x%08X\n", ads_ip_core_regs[3]);
    printf("+++++++\n");

	ads_hardreset();
	usleep(5000);

	// /*
	//  * Synchronize receive interface by usign special pattern
	//  */
	ReadOutoffADS(fd);
	write_to_ads(fd, 0x25, 0x000);	// reset
	write_to_ads(fd, 0x25, 0x100);	// set SYNC
	// write_to_ads(fd, 0x46, 0x8205);	// set EN_2WIRE, EN_12BNit
	write_to_ads(fd, 0x46, 0x8805);	// set EN_2WIRE, EN_16BNit
	usleep(20000);
    write_to_ads(fd, 0x45, 0x2);	// setting 111111000000 test pattern

	// /*
	//  * Set SYNC command to synchonize ads5292 with fpga
	//  */
	printf("ads_ip_core_regs[0] = 1;");
	fflush(stdout);
	ads_ip_core_regs[0] = 1;
	usleep(1);
	ads_ip_core_regs[0] = 0;
	// ads_ip_core_regs[0] = (0 << 7)| (6 << 2) | (0 << 5); // select channel | select gain to ddc
	ads_ip_core_regs[0] = (3 << 17) | (2 << 14) | (1 << 11) | (0 << 8); //6
	printf("[I] ADS ControlReg=0x%0X\r\n", ads_ip_core_regs[0]);
	

	printf("ads_ip_core_regs[0]=0x%08X\n", ads_ip_core_regs[0]);
	printf("ads_ip_core_regs[1]=0x%08X\n", ads_ip_core_regs[1]);
	printf("ads_ip_core_regs[2]=0x%08X\n", ads_ip_core_regs[2]);	
	uint32_t counter = 0;
    while((ads_ip_core_regs[1] & 0x1) == 0 ){
		counter++;
		if(counter % 100000 == 0)
			printf("[I] ADS StatusReg=0x%0X\r\n", ads_ip_core_regs[1]);
	} // wait for sync

    printf("[I] ADS synced\r\n");
    write_to_ads(fd, 0x45, 0x0);
   // write_to_ads(fd, 0x2A, 0x0);//8888);
    //write_to_ads(fd, 0x2B, 0x0);//8888);

     //write_to_ads(fd, 0x46, 0x8205); // set EN_2WIRE, EN_12BNit
     write_to_ads(fd, 0x25, 0x140);
     double bias = -50.0 * 1000.0 + 36.0*1000.0;
     double ddc_freq = 15.0*1000000.0 + bias;
     int32_t value_step = (int32_t)((ddc_freq/(double)80000000.0)*(double)4294967296.0);
     printf("[I] value_step of LNS: %0d\r\n", value_step);
     ads_ip_core_regs[2] = value_step;
     // set ramp
     write_to_ads(fd, 0x25, 0x40); // set SYNC0x42
     ads_ip_core_regs[2] = value_step;

	munmap(ads_ip_core_regs, M_AXI_ADS_SIZE);
	close(fd_mem);
	return 0;

}

void ads_hardreset(){
	pin_set(ADS5292_RESET_RST_PIN, HIGH);
	usleep(1000);
	pin_set(ADS5292_RESET_RST_PIN, LOW);
	usleep(10000);
}

int ReadOutOnADS(int fd){

	uint8_t tx_buf[3];
	tx_buf[0] = 0x1;
	tx_buf[1] = 0x0;
	tx_buf[2] = 0x1;

	uint8_t rx_buf[3];

	spi_transaction(fd, tx_buf, rx_buf, 3);
	return 0;
}

int ReadOutoffADS(int fd){

	uint8_t tx_buf[3];
	tx_buf[0] = 0x1;
	tx_buf[1] = 0x0;
	tx_buf[2] = 0x0;

	uint8_t rx_buf[3];

	spi_transaction(fd, tx_buf, rx_buf, 3);
	return 0;
}

void write_to_ads(int fd, uint8_t addr, uint16_t data){

	printf("[I] write : addr 0x%0x data 0x%0x\r\n", addr,data);
	
	uint8_t tx_buf[3];
	tx_buf[0] = addr;
	tx_buf[1] = (data & 0xff00) >> 8;
	tx_buf[2] = data & 0xff;

	uint8_t rx_buf[3];
	spi_transaction(fd, tx_buf, rx_buf, 3);
}

uint16_t read_from_ads(int fd, uint8_t addr){
	
	// WriteBuffer[0] = addr;
	// XSpi_Transfer(&SpiInstance, WriteBuffer, ReadBuffer, 3);
	
	// uint16_t tmp = (ReadBuffer[1] << 8) | ReadBuffer[2];
	// return tmp;
	return 0;
}

void fifo_enable(){
	int fd_mem = open("/dev/mem", O_RDWR | O_SYNC);
	uint32_t* ads_ip_core_regs = (uint32_t*)mmap(NULL, M_AXI_ADS_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd_mem, M_AXI_ADS_BASEADDR);

	printf("ads_ip_core_regs[0]=0x%08X\n", ads_ip_core_regs[0]);
	ads_ip_core_regs[0] |= (1 << 21);
	usleep(5000);
	printf("ads_ip_core_regs[0]=0x%08X\n", ads_ip_core_regs[0]);
	munmap(ads_ip_core_regs, M_AXI_ADS_SIZE);
	close(fd_mem);

}

void pl_axistream_counter_enable(){
	int fd_mem = open("/dev/mem", O_RDWR | O_SYNC);
	uint32_t* ads_ip_core_regs = (uint32_t*)mmap(NULL, M_AXI_ADS_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd_mem, M_AXI_ADS_BASEADDR);

	printf("ads_ip_core_regs[0]=0x%08X\n", ads_ip_core_regs[0]);
	ads_ip_core_regs[0] |= (1 << 20);
	usleep(5000);
	printf("ads_ip_core_regs[0]=0x%08X\n", ads_ip_core_regs[0]);
	munmap(ads_ip_core_regs, M_AXI_ADS_SIZE);
	close(fd_mem);

}
