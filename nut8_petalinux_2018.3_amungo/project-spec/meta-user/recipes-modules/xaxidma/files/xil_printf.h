 #ifndef XIL_PRINTF_H
 #define XIL_PRINTF_H

#ifdef __cplusplus
extern "C" {
#endif

//#include <stdio.h>
#include <linux/kernel.h>

#define xil_printf(...) printk(__VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif	/* end of protection macro */
