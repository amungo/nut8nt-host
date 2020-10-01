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
        std::cout << "Read config: " << param << " " << val << std::endl;

        if (param == "nt1065_1_cfg_path") {
            cfgP[0] = val;
		} else

        if (param == "nt1065_2_cfg_path") {
            cfgP[1] = val;
        } else

		if (param == "ads5292_gain") {
			adsGain = std::stoll(val, 0, 16);
		} else

		if (param == "max5292_val") {
			maxVal = std::stoi(val, 0, 16);
		}
        else {
            std::cout << " Wrong parameter: " << param << std::endl;
        }
	}



}

bool ConfigReader::isOpen() {
    return file.is_open();
}

std::string ConfigReader::readNT1065ConfigPath(int nt1065Num) {
    return cfgP[nt1065Num];
}

uint64_t ConfigReader::readADS5292Gain() {
	return adsGain;
}

uint16_t ConfigReader::readMAX5717Value() {
	return maxVal;
}
