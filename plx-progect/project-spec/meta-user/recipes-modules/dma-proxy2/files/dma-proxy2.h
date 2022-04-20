#define DMA_BUFFER_SIZE (500 * 1024 * 1024)	 	/* must match driver exactly */
#define BUFFER_COUNT 1					/* driver only */

#define TX_BUFFER_COUNT 	1				/* app only, must be <= to the number in the driver */
#define RX_BUFFER_COUNT 	1				/* app only, must be <= to the number in the driver */
#define BUFFER_INCREMENT	1				/* normally 1, but skipping buffers (2) defeats prefetching in the CPU */

#define FINISH_XFER 	_IOW('a','a',int32_t*)
#define START_XFER 		_IOW('a','b',int32_t*)
#define XFER 			_IOR('a','c',int32_t*)

enum PROXY_DMA_REQUEST {
    PROXY_DMA_SYNC_READ = 0, //nu
    PROXY_DMA_LOOP_START = 1, // nu
    PROXY_DMA_SYNC_READ_ALL_BUFFERS = 8,
    PROXY_DMA_CYCLIC_START = 9,
    PROXY_DMA_CYCLIC_STOP = 10,
    PROXY_DMA_LOOP_STOP = 4, // nu
    PROXY_DMA_SET_SIGNAL = 3
};

typedef struct signal_parameters{
    unsigned int on;
    unsigned int data;
    unsigned int period;
} signal_parameters;

struct dma_proxy_channel_interface {
    char buffer[DMA_BUFFER_SIZE];
	enum proxy_status { PROXY_NO_ERROR = 0, PROXY_BUSY = 1, PROXY_TIMEOUT = 2, PROXY_ERROR = 3 } status;
	unsigned int length;
	unsigned int buf_num;
} __attribute__ ((aligned (1024)));		/* 64 byte alignment required for DMA, but 1024 handy for viewing memory */
