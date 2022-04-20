
#define BUFFER_SIZE	(500 * 1024 * 1024)
//#define BUFFER_SIZE	(1024)

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
    unsigned char buffer[BUFFER_SIZE];
    enum proxy_status { PROXY_NO_ERROR = 0, PROXY_BUSY = 1, PROXY_TIMEOUT = 2, PROXY_ERROR = 3 } status;
    unsigned int length;
    unsigned int buf_num;
};

