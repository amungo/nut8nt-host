#ifndef __NTDMA_H__
#define __NTDMA_H__



enum transfer_state {
        TRANSFER_STATE_NEW = 0,
        TRANSFER_STATE_SUBMITTED,
        TRANSFER_STATE_COMPLETED,
        TRANSFER_STATE_FAILED,
        TRANSFER_STATE_ABORTED
};

enum dma_fsm_state {
    DMA_IDLE = 0,
    DMA_IN_FLIGHT = 1,
    DMA_COMPLETING = 3
};

#define NTDMA_DEV_NAME_MAX_CHARS (16)




#define NTDMA_MAX_BUFFER_SIZE (PAGE_SIZE * 4096 * 10)


enum ntdma_dir {
    NTDMA_DEV_TO_CPU = 1,   // RX
    NTDMA_CPU_TO_DEV = 2,   // TX
};

struct ntdma_pdev_drvdata {
    struct list_head ntdma_list;    // list of ezdma_drvdata instances created in
                                    // relation to this platform device
};

struct ntdma_transfer {
    struct list_head entry; /* queue of non-completed transfers */
    struct dma_async_tx_descriptor* txn_desc;
    dma_cookie_t cookie;

    u8* dma_buffer;             /* Contiguous memory for DMA */
    dma_addr_t dma_buffer_bus;  /* bus addr for dma buffer */
    size_t dma_buffer_size;     /* Size memory for DMA */
    struct scatterlist sglist;
    int index;

    enum transfer_state state;	/* state of the transfer */
    unsigned int flags;
};

struct ntdma_drvdata {
    struct platform_device* pdev;
    char name[NTDMA_DEV_NAME_MAX_CHARS];

    struct semaphore sem;

    bool        in_use;
    atomic_t    accepting;

    struct completion cmp;
    spinlock_t state_lock;  /* protects state below, may be taken from interrupt (tasklet) context */
    enum dma_fsm_state state;

    wait_queue_head_t    transfer_wq; /* wait queue for transfer completion */

    /* dmaengine */
    struct dma_chan* chan;

    /* device accounting */
    dev_t           ntdma_devt;
    struct cdev     ntdma_cdev;
    struct device*  ntdma_dev;
    struct device*  dma_dev;

    /* Statistics */
    atomic_t    packets_rcvd;
    struct list_head node;

    //------------------------------------------------
    /* Transfer list management */
    struct list_head cyclic_transfer_list;  /* queue of transfers */
    struct list_head* current_transfer;     /* current running transfer */

    u32 cyclic_desc_count;
    u32 cyclic_desc_len;

    int rx_head;	/* follows the HW */
    int rx_tail;	/* where the SW reads from */
    int rx_overrun;	/* flag if overrun occured */
    int xfer_complited;

    bool wait_rx_data;
    //------------------------------------------------
};


#endif // __NTDMA_H__
