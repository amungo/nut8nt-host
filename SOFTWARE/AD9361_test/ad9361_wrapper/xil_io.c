#include "xil_io.h"
#include "xparameters.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/mman.h>

#define AD9361_MEM_SIZE 0x10000

static int fd;
static char* ad9361_mem;

void init_ad9361_mem(){
	fd = open("/dev/mem", O_RDWR);
	ad9361_mem = (char*)mmap(NULL, AD9361_MEM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, XPAR_M_AXI_AD9364_BASEADDR);
	
}
void deinit_ad9361_mem(){
	munmap(ad9361_mem, AD9361_MEM_SIZE);
	close(fd);
}

u32 Xil_In32(UINTPTR Addr){
	return *(uint32_t*)(ad9361_mem + (Addr - XPAR_M_AXI_AD9364_BASEADDR));
}

void Xil_Out32(UINTPTR Addr, u32 Value){
	*(uint32_t*)(ad9361_mem + (Addr - XPAR_M_AXI_AD9364_BASEADDR)) = Value;
}

void Xil_DCacheFlush(){

}

