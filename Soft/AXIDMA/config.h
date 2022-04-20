
#define NUT8_KRIO

#ifdef NUT8_KRIO
#define NUT_NAME                "NUT8_KRIO"
#define BUFF_COUNT              (250)
#define DMA_PROXY2
#define SAMPLES_NUM				(5000)
#define DMA_BUF_SIZE            (SAMPLES_NUM * SAMPLE_SIZE * CHANNELS_NUM)
#define SAMPLE_SIZE             (sizeof(int16_t) * 2)
#define CHANNELS_NUM            4
#define SUPER_BUFFER_SIZE       1
#endif

