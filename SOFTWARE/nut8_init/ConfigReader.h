#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <iostream>

class ConfigReader {
public:
	ConfigReader(std::string fileName);

    bool isOpen();

	std::string readNT1065ConfigPath();
	uint64_t readADS5292Gain();
	uint16_t readMAX5717Value();
private:
	std::ifstream file;

	std::string cfgP;
	uint64_t adsGain;
	uint16_t maxVal;
};

