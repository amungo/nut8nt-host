#ifndef AD9361CLASS_H
#define AD9361CLASS_H

#include <fstream>
#include <string>
#include "spi_master_ivan.h"
#include "ad9361_api.h"
#include "ad9361cfg.h"
#include "defines.h"
#include "gpio.h"
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>

class AD9361_C : public SPIMaster
{
public:
    AD9361_C(const std::string& dev);
    void checkDevice() override;
private:
    struct ad9361_rf_phy *ad9361_phy;
    GPIO RST, TX_EN, RX_EN;
};

#endif // AD9361CLASS_H
