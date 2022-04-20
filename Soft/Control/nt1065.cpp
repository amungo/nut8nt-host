#include "nt1065.h"


NT1065::NT1065(const std::string& dev) : SPIMaster(dev) {

}

bool NT1065::checkDevice() {
    bool ret = false;

    std::cout << "Check NT1065 on " << dev << std::endl;
    uint8_t rx;
    readReg(0x00, &rx);

    if (rx == 0b00100001) {
        readReg(0x01, &rx);
        std::cout << " NT1065 ver. " << (rx & 0b111) << std::endl;
    }
    else {
        std::cout << " Not NT1065 detect (" << std::hex << (int)rx << std::dec << ")." << std::endl;
        return false;
    }

    readReg(44, &rx);

    if (rx & 1) {
        std::cout << " PLL locked" << std::endl;
        ret = true;
    } else {
        std::cout << " PLL NOT locked" << std::endl;
    }

    return ret;
}

void NT1065::config(const std::string& cfg) {
    std::cout << "Config NT1065" << std::endl;

    std::ifstream file(cfg);

    if(!file.is_open()) {
        std::cout << " Error open config file: " << cfg << std::endl;
        return;
    }

    std::string regName_s;
    std::string regVal_s;

    int regAddr_i;
    int regVal_i;
    file >> regName_s;

    //for (int i=0; i<48; i++) {
    while (!file.eof()) {
        file >> regName_s >> regVal_s;
        //std::cout << " REG: " << regName_s << "\t Data:   " << std::hex << regVal_s << std::dec << std::endl;
        regAddr_i = std::stoi(regName_s.substr(3));
        regVal_i = std::stoi(regVal_s, 0, 16);
        //std::cout << "      " << regAddr_i << "\t       0x" << std::hex << regVal_i << std::dec << std::endl;
        writeReg(regAddr_i, regVal_i);
    }

    file.close();
}
