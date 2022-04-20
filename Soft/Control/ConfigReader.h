#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <iostream>

class ConfigReader {
public:
	ConfigReader(std::string fileName);

    bool isOpen();

    std::string readNT1065ConfigPath(int nt1065Num);
	uint64_t readADS5292Gain();
	uint16_t readMAX5717Value();
    uint8_t readNT1065Num();
    uint8_t counterMod();
private:
	std::ifstream file;

    std::string cfgP[2];
	uint64_t adsGain;
	uint16_t maxVal;
    uint8_t ntNum;
    uint8_t counter = 0;
};

