#ifndef MAX5717_H
#define MAX5717_H

#include "spi_master_ivan.h"

class MAX5717 : public SPIMaster
{
public:
    MAX5717(const std::string& dev);
    void setVal(uint16_t val);
    void checkDevice() override;
};

#endif // MAX5717_H
