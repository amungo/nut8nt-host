#include <iostream>

#include <cstdlib>
#include <iostream>
#include <string>
#include <getopt.h>
#include <inttypes.h>
#include <thread>
#include <unistd.h>

#include <defines.h>
#include <nt1065.h>
#include <max2871.h>
#include <ads5292.h>
#include <max5717.h>
#include <gpio.h>
#include <axidma.h>
#include <ConfigReader.h>


using namespace std;


GPIO** test;

int main(int argc, char *argv[]) {
    printConfigs();

    GPIO LED1(GPIO_PIN_LED1, GPIO_DIR_OUT);
    GPIO LED2(GPIO_PIN_LED2, GPIO_DIR_OUT);
    GPIO LED3(GPIO_PIN_LED3, GPIO_DIR_OUT);
    GPIO LED4(GPIO_PIN_LED4, GPIO_DIR_OUT);

    GPIO AXI_RST(GPIO_PIN_AXI_RST, GPIO_DIR_OUT);
    AXI_RST.out(1);
    AXI_RST.out(0);
    AXI_RST.out(1);

    LED1.out(0);
    LED2.out(0);
    LED3.out(0);
    LED4.out(0);

    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " cfgPath" << std::endl;
//        //GPIO TXO_EN_off

//        //ADS5292 ads(SPI_ADDR_ADS5292);

//        //ads.setData();
//        NT1065 nt1(SPI_ADDR_NT1065_2);

//        while(1) {
//            nt1.writeReg(0x5, 0x55);
//            //nt1.checkDevice();
//            usleep(100);
//        }

        return 0;
    }

    cout << "   === Start NUT8_init ===" << endl;

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



    GPIO GENA(GPIO_PIN_AD9361_GEN_ENA, GPIO_DIR_OUT);
    GENA.out(1);

    ads.setData();

    nt1.config(cr.readNT1065ConfigPath(0));
    nt2.config(cr.readNT1065ConfigPath(1));

    bool hasErrors = false;

    cout << "\n" << endl;

    nt1.checkDevice();
    nt2.checkDevice();
    max2871.checkDevice();

    ads.setData();

    //return 0;
    ads.setGain(cr.readADS5292Gain());

    MAX5717 max5717(SPI_ADDR_MAX5717);
    max5717.setVal(cr.readMAX5717Value());

    GPIO NT_1(GPIO_PIN_CH1_CHNG, GPIO_DIR_OUT),
         NT_2(GPIO_PIN_CH2_CHNG, GPIO_DIR_OUT),
         NT_3(GPIO_PIN_CH3_CHNG, GPIO_DIR_OUT),
         NT_4(GPIO_PIN_CH4_CHNG, GPIO_DIR_OUT);

    if (cr.readNT1065Num()) {
    //if (false) {
        cout << "Using NT1065 N1" << endl;
        NT_1.out(1);
        NT_2.out(1);
        NT_3.out(1);
        NT_4.out(1);
    } else {
        cout << "Using NT1065 N2" << endl;
        NT_1.out(0);
        NT_2.out(0);
        NT_3.out(0);
        NT_4.out(0);
    }

    GPIO COUNTER_MOD(GPIO_PIN_CNT_CHNG, GPIO_DIR_OUT);

    if (cr.counterMod()) {
        cout << "Counter mod" << endl;
        COUNTER_MOD.out(1);
    } else {
        COUNTER_MOD.out(0);
    }

    LED1.out(1);
    LED2.out(!hasErrors);

    //DMAReader reader(DMA_BUF_SIZE, SUPER_BUFFER_SIZE);
    //reader.start();
    //reader.finish();

    return 0;
}
