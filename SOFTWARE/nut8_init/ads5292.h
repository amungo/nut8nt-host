#ifndef ADS5292_H
#define ADS5292_H

#include "spi_master_ivan.h"
#include "gpio.h"
#include "defines.h"

class ADS5292 : public SPIMaster
{
public:
    ADS5292(const std::string& dev);

    void setRamp();
    void setData();
    void checkDevice() override;
    void writeToADS5292(int adr, int val);
    uint16_t readFromADS5292();
    void reset();
private:
    GPIO RST, PWD, CS;
};

#endif // ADS5292_H
