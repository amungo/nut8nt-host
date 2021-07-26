#ifndef __NTDMA_IOCTL_H__
#define __NTDMA_IOCTL_H__


enum NTDMA_IOC_TYPES {
        NTDMA_IOC_GET_BUFFERS = 0,
        NTDMA_IOC_SET_BUFFERS,
        NTDMA_IOC_DMA_READ,
        NTDMA_IOC_DMA_RESET,
        NTDMA_IOC_DMA_START,
        NTDMA_IOC_MAX
};

/*--------------------------------------
 * IOCTL Interface
 *-------------------------------------*/
/* Use 'x' as magic number */
#define NTDMA_IOC_MAGIC	'x'

//------------------------------------------------------------------------
struct ntdma_buffer_params {
    unsigned int buf_size;  /* Размер одного DMA буфера в байтах  */
    unsigned int buf_count; /* Количество DMA буферов */
};

struct ntdma_transaction {
    unsigned int head;  /* Индекс последнего заполненного буфера */
    unsigned int tail;  /* Индекс первого заполненного буфера */
    unsigned int buf_overrun; /* Счетчик перезаписанных буферов */
};
//-------------------------------------------------------------------------


/* IOCTL коды */
/**
*  Получить информацию о количестве буферов, выделенных драйвером для DMA транзакций
*  и информации о размере этих буферов
*/
#define IOCTL_NTDMA_GET_BUFFERS    _IOR(NTDMA_IOC_MAGIC, NTDMA_IOC_GET_BUFFERS, \
                                        struct ntdma_buffer_params*)

/**
*   Задать размер и количество выделяемых драйвером буферов,
*   используемых для DMA транзакций.
*/
#define IOCTL_NTDMA_SET_BUFFERS    _IOW(NTDMA_IOC_MAGIC, NTDMA_IOC_SET_BUFFERS, \
                                        struct ntdma_buffer_params*)

/**
*   Получить информацию о заполненности DMA буферов
*/
#define IOCTL_NTDMA_DMA_READ       _IOR(NTDMA_IOC_MAGIC, NTDMA_IOC_DMA_READ, \
                                        struct ntdma_transaction*)

/**
*   Сбросить в драйвере информацию о заполненности DMA буферов
*/
#define IOCTL_NTDMA_DMA_RESET      _IO(NTDMA_IOC_MAGIC, NTDMA_IOC_DMA_RESET)


/**
*   Запустить / остановить цикл чтения данных.
*/
#define IOCTL_NTDMA_DMA_START      _IOW(NTDMA_IOC_MAGIC, NTDMA_IOC_DMA_START, int)


#endif // __NTDMA_IOCTL_H__
