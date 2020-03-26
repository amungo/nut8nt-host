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
#include "gpio.h"
#include "axi_dma.h"

using namespace std;


GPIO** test;

int main()
{
    GPIO RCVENPIN(GPIO_PIN_RCVENPIN, GPIO_DIR_OUT),
         TXO_EN(GPIO_PIN_TXO_EN, GPIO_DIR_OUT),
         ANT_ON(GPIO_PIN_ANT_ON, GPIO_DIR_OUT);

    RCVENPIN.out(1);
    TXO_EN.out(1);
    ANT_ON.out(1);

    NT1065 nt1(SPI_ADDR_NT1065_1),
           nt2(SPI_ADDR_NT1065_2);
    MAX2871 max(SPI_ADDR_MAX2871);
    ADS5292 ads(SPI_ADDR_ADS5292);

    nt1.config("/home/root/ConfigSet_all_GPS_L1_patched_ldvs_noadc.hex");
    nt2.config("/home/root/ConfigSet_all_GPS_L1_patched_ldvs_noadc.hex");

    nt1.checkDevice();
    nt2.checkDevice();
    max.checkDevice();

    //cout << hex << "read from ads = 0x" << ads.readFromADS5292() << endl;
    ads.setData();

    AXI_DMA dma;
    dma.test();

    return 0;
}
