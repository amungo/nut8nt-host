#ifndef XIL_IO_H
#define XIL_IO_H

#include <stdint.h>

typedef uint32_t u32;
typedef uintptr_t UINTPTR;

void init_ad9361_mem();
void deinit_ad9361_mem();

u32 Xil_In32(UINTPTR Addr);
void Xil_Out32(UINTPTR Addr, u32 Value);
void Xil_DCacheFlush();

#endif
