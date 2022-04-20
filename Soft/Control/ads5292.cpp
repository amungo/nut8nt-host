#include "ads5292.h"

ADS5292::ADS5292(const std::string& dev) : SPIMaster(dev), PWD(GPIO_PIN_ADS5292_PWD, GPIO_DIR_OUT), RST(GPIO_PIN_ADS5292_RST, GPIO_DIR_OUT), CS(GPIO_PIN_ADS5292_CSZ, GPIO_DIR_OUT) {
    PWD.out(0);
    RST.out(0);
    CS.out(1);
    usleep(10000);

    reset();

    GPIO al_start(GPIO_PIN_ALIGNER_START, GPIO_DIR_OUT);
    GPIO al_reset(GPIO_PIN_ALIGNER_RESET, GPIO_DIR_OUT);

    writeToADS5292(0x01, 0);
    writeToADS5292(0x00, 1);
    writeToADS5292(0x25, 0x000);	// reset


#ifdef NUT8AR
    writeToADS5292(0x01, 1 << 4); // Enable access to register at address 0xF0
    writeToADS5292(0xF0, 1 << 15); //Enable external reference mode
    //writeToADS5292(0x42, 1 << 15 | 1 << 3);
#endif

    writeToADS5292(0x25, 0x100);	// set SYNC
    writeToADS5292(0x46, 0x8805);	// set EN_2WIRE, EN_16BNit
    usleep(20000);

#ifdef NUT8_KRIO_
    std::cout << "Remap " << std::endl;

    //                   OUT1A 7X  OUT1B 7X        OUT2A 6X
    writeToADS5292(0x50, 0b1001 | (0b1000 << 4) | (0b1101 << 8) | (1 << 15));
    //                   OUT2B 6X  OUT3A 0         OUT3B 0
    writeToADS5292(0x51, 0b1111 | (0b0110 << 4) | (0b0111 << 8) | (1 << 15));
    //                   OUT4A 1 OUT4B 1
    writeToADS5292(0x52, 0b0000 | (0b0001 << 4) | (1 << 15));  // nt1 - 1
    //                   OUT5A 2   OUT5B 2          OUT6A
    writeToADS5292(0x53, 0b0001 | (0b0000 << 4) | (0b1101 << 8) | (1 << 15));
    //                   OUT6B     OUT7A           OUT7B
    writeToADS5292(0x54, 0b1100 | (0b1011 << 4) | (0b1010 << 8) | (1 << 15));
    //                   OUT8A 3   OUT8B 3
    writeToADS5292(0x55, 0b0111 | (0b0110 << 4) | (1 << 15)); // nt1 -3


#endif


    writeToADS5292(0x45, 0x2);	// setting 111111000000 test pattern



    al_reset.out(1);
    usleep(1000);
    al_reset.out(0);
    usleep(1000);
    al_start.out(1);

    setData();
}

bool ADS5292::checkDevice() {
    std::cout << "Check ADS5292 on " << dev << std::endl;
    std::cout << " Yes... i think... I dont know(" << std::endl;
    return true;
}

void ADS5292::setRamp() {
    writeToADS5292(0x45, 0x0);
    writeToADS5292(0x25, 0x140);
    writeToADS5292(0x25, 0x40); // set SYNC0x42
}

void ADS5292::setGain(uint64_t gain) {
    std::cout << "0x2A << 0x" << std::hex << (gain & 0xFFFF) << std::dec << std::endl;
    writeToADS5292(0x2A, gain & 0xFFFF);
    std::cout << "0x2B << 0x" << std::hex << ((gain >> 16) & 0xFFFF) << std::dec << std::endl;
    writeToADS5292(0x2B, (gain >> 16) & 0xFFFF);
}

void ADS5292::setData() {
    writeToADS5292(0x45, 0x0);
    writeToADS5292(0x2A, 0xCCCC);
    writeToADS5292(0x2B, 0xCCCC);
}


void ADS5292::writeToADS5292(int addr, int data) {
    uint8_t tx_buf[3];
    uint8_t rx_buf[3];
    tx_buf[0] = addr;
    tx_buf[1] = (data & 0xff00) >> 8;
    tx_buf[2] = data & 0xff;

    CS.out(0);
    transmitData(tx_buf, rx_buf, 3);
    CS.out(1);
}

uint16_t ADS5292::readFromADS5292() {
    uint8_t tx_buf[3];
    uint8_t rx_buf[3];
    tx_buf[0] = 0x0F;
    tx_buf[1] = 0;
    tx_buf[2] = 0;

    CS.out(0);
    transmitData(tx_buf, rx_buf, 3);
    CS.out(1);

    uint16_t ret = rx_buf[1] | rx_buf[2] << 8;
    return ret;
}

void ADS5292::reset() {
    RST.out(1);
    usleep(1000);
    RST.out(0);
    usleep(10000);
}
