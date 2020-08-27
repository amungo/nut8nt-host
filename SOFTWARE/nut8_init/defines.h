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

    #define GPIO_PIN_RCVENPIN       44
    #define GPIO_PIN_ANT_ON         39
    #define GPIO_PIN_TXO_EN         78

    #define GPIO_PIN_MAX2871_CE     82
    #define GPIO_PIN_MAX2871_EN     81
    #define GPIO_PIN_MAX2871_PWR    83
    #define GPIO_PIN_MAX2871_LE     84

    #define GPIO_PIN_ADS5292_RST    79
    #define GPIO_PIN_ADS5292_PWD    80
    #define GPIO_PIN_ADS5292_CSZ    85

    #define GPIO_PIN_ALIGNER_START  86
    #define GPIO_PIN_ALIGNER_RESET  87

    #define GPIO_PIN_FIFO_RUN       88

    #define GPIO_PIN_AD9361_RESET   89
    #define GPIO_PIN_AD9361_GEN_ENA 97
    #define GPIO_PIN_AD9361_TX_EN   61 //91
    #define GPIO_PIN_AD9361_RX_EN   62 //92
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
