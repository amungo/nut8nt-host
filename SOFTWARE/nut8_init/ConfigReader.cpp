#include "ConfigReader.h"

ConfigReader::ConfigReader(std::string fileName) {
	file.open(fileName);

	if (!file.is_open()) {
		std::cout << " Error open config file: " << fileName << std::endl;
        return;
	}

	std::string param, val;

	while (!file.eof()) {
		file >> param >> val;
		//std::cout << param << " " << val << std::endl;

		if (param == "nt1065_cfg_path") {
			cfgP = val;
		} else

		if (param == "ads5292_gain") {
			adsGain = std::stoll(val, 0, 16);
		} else

		if (param == "max5292_val") {
			maxVal = std::stoi(val, 0, 16);
		}
	}



}

bool ConfigReader::isOpen() {
    return file.is_open();
}

std::string ConfigReader::readNT1065ConfigPath() {
	return cfgP;
}

uint64_t ConfigReader::readADS5292Gain() {
	return adsGain;
}

uint16_t ConfigReader::readMAX5717Value() {
	return maxVal;
}
