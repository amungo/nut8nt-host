#ifndef DEFINES_H
#define DEFINES_H

#define SUPER

#ifdef SUPER
    #define SPI_ADDR_NT1065_1       "/dev/spidev0.0"
    #define SPI_ADDR_NT1065_2       "/dev/spidev0.1"
    #define SPI_ADDR_MAX2871        "/dev/spidev1.0"
    #define SPI_ADDR_ADS5292        "/dev/spidev1.1"
    #define SPI_ADDR_AD9361         "/dev/spidev0.2"
    #define SPI_ADDR_MAX5717        "/dev/spidev1.2"

    // +78

    #define GPIO_PIN_RCVENPIN       44
    #define GPIO_PIN_ANT_ON         39
    #define GPIO_PIN_TXO_EN         78  //0

    #define GPIO_PIN_MAX2871_CE     82  //4
    #define GPIO_PIN_MAX2871_EN     81  //3
    #define GPIO_PIN_MAX2871_PWR    83  //5
    #define GPIO_PIN_MAX2871_LE     84  //6

    #define GPIO_PIN_ADS5292_RST    79  //1
    #define GPIO_PIN_ADS5292_PWD    80  //2
    #define GPIO_PIN_ADS5292_CSZ    85

    #define GPIO_PIN_ALIGNER_START  86  //8
    #define GPIO_PIN_ALIGNER_RESET  87  //9

    #define GPIO_PIN_FIFO_RUN       88  //10

    #define GPIO_PIN_AD9361_RESET   89  //11
    #define GPIO_PIN_AD9361_GEN_ENA 97  //19
    #define GPIO_PIN_AD9361_TX_EN   61 //91
    #define GPIO_PIN_AD9361_RX_EN   62 //92

    #define GPIO_PIN_CH1_CHNG       106
    #define GPIO_PIN_CH2_CHNG       107
    #define GPIO_PIN_CH3_CHNG       108
    #define GPIO_PIN_CH4_CHNG       109
    #define GPIO_PIN_CNT_CHNG       110
    #define GPIO_PIN_DMA_RST        111

    #define GPIO_PIN_LED1           114
    #define GPIO_PIN_LED2           115
    #define GPIO_PIN_LED3           116
    #define GPIO_PIN_LED4           117

    #define GPIO_PIN_AXI_RST        118

    #define NUT8AR
#else
    #define SPI_ADDR_NT1065_1       "/dev/spidev2.0"
    #define SPI_ADDR_NT1065_2       "/dev/spidev2.1"
    #define SPI_ADDR_MAX2871        "/dev/spidev3.0"
    #define SPI_ADDR_ADS5292        "/dev/spidev1.0"

    #define GPIO_PIN_RCVENPIN       44
    #define GPIO_PIN_TXO_EN         91

    #define GPIO_PIN_MAX2871_CE     85
    #define GPIO_PIN_MAX2871_EN     86
    #define GPIO_PIN_MAX2871_PWR    84
    #define GPIO_PIN_MAX2871_LE     87

    #define GPIO_PIN_ADS5292_RST    89
    #define GPIO_PIN_ADS5292_PWD    90
    #define GPIO_PIN_ADS5292_CSZ    92
#endif
#endif // DEFINES_H
