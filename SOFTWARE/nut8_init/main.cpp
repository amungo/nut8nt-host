#include <iostream>

#include <cstdlib>
#include <iostream>
#include <string>
#include <getopt.h>
#include <inttypes.h>
#include <thread>
#include <unistd.h>

#include "defines.h"
#include "nt1065.h"
#include "max2871.h"
#include "ads5292.h"
#include "ad9361class.h"
#include "max5717.h"
#include "gpio.h"
#include "axi_dma.h"
#include "ConfigReader.h"


using namespace std;


GPIO** test;

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " cfgPath" << std::endl;
        return 0;
    }


    cout << "   === Start NUT8_init ===" << endl;
    AD9361_C ad(SPI_ADDR_AD9361);

    ConfigReader cr(argv[1]);

    if (!cr.isOpen()) {
        cout << "Exit" << endl;
        return -1;
    }

    GPIO RCVENPIN(GPIO_PIN_RCVENPIN, GPIO_DIR_OUT),
         TXO_EN(GPIO_PIN_TXO_EN, GPIO_DIR_OUT),
         ANT_ON(GPIO_PIN_ANT_ON, GPIO_DIR_OUT);

    RCVENPIN.out(1);
    TXO_EN.out(1);
    ANT_ON.out(1);

    NT1065 nt1(SPI_ADDR_NT1065_1),
           nt2(SPI_ADDR_NT1065_2);
    MAX2871 max2871(SPI_ADDR_MAX2871);
    ADS5292 ads(SPI_ADDR_ADS5292);


    //sleep(1);
    //cout << "   === ad test ===" << endl;
    //AD9361_C ad(SPI_ADDR_AD9361);

    GPIO GENA(GPIO_PIN_AD9361_GEN_ENA, GPIO_DIR_OUT);
    GENA.out(1);

    nt1.config(cr.readNT1065ConfigPath(0));
    nt2.config(cr.readNT1065ConfigPath(1));

    nt1.checkDevice();
    nt2.checkDevice();
    max2871.checkDevice();

    //cout << hex << "read from ads = 0x" << ads.readFromADS5292() << endl;
    ads.setData();
    ads.setGain(cr.readADS5292Gain());

    MAX5717 max5717(SPI_ADDR_MAX5717);
    max5717.setVal(cr.readMAX5717Value());

    AXI_DMA dma;
    dma.test();

    return 0;
}
