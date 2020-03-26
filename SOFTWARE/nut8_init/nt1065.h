#ifndef NT1065_H
#define NT1065_H

#include <fstream>
#include <string>
#include "spi_master.h"
#include "spi_master_ivan.h"


class NT1065 : public SPIMaster
{
public:
    NT1065(const std::string& dev);
    void checkDevice() override;
    void config(const std::string& cfg);
};

#endif // NT1065_H
