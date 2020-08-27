#include "max5717.h"

MAX5717::MAX5717(const std::string& dev) : SPIMaster(dev) {

}

void MAX5717::checkDevice() {

}

void MAX5717::setVal(uint16_t val) {
    uint16_t txd = val;
    uint16_t rxd;

    transmitData((uint8_t*)&txd, (uint8_t*)&rxd, 2);
}
