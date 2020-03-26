 #include "max2871.h"

#define SIZE_MAX2871_CONFIG		6

static uint32_t max_config[SIZE_MAX2871_CONFIG] = {
    0x00800000,
    0x200103E9,
    0x12005F42,
    0x01009F23,
    0x61E900DC,//0x61E900DC - 80; 0x61D900DC - 160
    0x00440005
};

static uint32_t max_registers [SIZE_MAX2871_CONFIG];

MAX2871::MAX2871(const std::string& dev) : SPIMaster(dev),
                              CE(GPIO_PIN_MAX2871_CE, GPIO_DIR_OUT),
                              EN(GPIO_PIN_MAX2871_EN, GPIO_DIR_OUT),
                              PWR(GPIO_PIN_MAX2871_PWR, GPIO_DIR_OUT),
                              LE(GPIO_PIN_MAX2871_LE, GPIO_DIR_OUT) {
    reset();
    EN.out(0);
    std::cout << "Configure MAX2871..." << std::endl;
    for (int j = 0; j < 1; j++)	{
        for (int i = SIZE_MAX2871_CONFIG-1; i >= 0; i--){   // 6 write registers
            char b[100];
            //max_config[i] = 0x00800000;
            //sprintf(b, "read -p %x",max_config[i]);

            writeToMAX2871(max_config[i]);
            //system(b);
            max_registers[i] = max_config[i];
        }
        usleep(10000);
    }
    EN.out(1);
    max_registers[4] =  MAX2871_RFA_EN | max_registers[4];
    writeToMAX2871(max_registers[4]);
    writeToMAX2871(max_registers[4]);
    usleep(50000);

    std::cout << "Done!" << std::endl;
    // Enable RF output
    //EN.out(1);
}

void MAX2871::checkDevice() {
    std::cout << "Check MAX2871 on " << dev << std::endl;

    uint32_t read_data = readFromMAX2871();
    uint32_t id = (read_data & MAX2871_ID_mask) >> MAX2871_ID_shift;

    if (id == 0b0110 || id == 0b0111) {
        std::cout << " MAX2871" << std::endl;
    }
    else {
        std::cout << " Not MAX2871 detect (" << std::hex << (int)read_data << std::dec << ")." << std::endl;
    }
}

void MAX2871::reset() {
    LE.out(1);
    CE.out(0);
    EN.out(0);
    PWR.out(1);
    usleep(10000);
    CE.out(1);
    EN.out(1);
    PWR.out(1);
    usleep(10000);
}

void MAX2871::writeToMAX2871(uint32_t data) {
    uint8_t tx_buf[4];
    uint8_t rx_buf[4];

    for(int i = 0; i < 4; i++){
        tx_buf[i] = (data & (0xFF << 8*(3 - i))) >> ( 8*(3 - i) );
    }

    LE.out(0);
    int ret = transmitData(tx_buf, rx_buf, 4);
    LE.out(1);
    usleep(50000);
}

uint32_t MAX2871::readFromMAX2871() {
    uint8_t tx_buf[5] = {0x0,0x0,0x0,MAX2871_REG_6,0x0};
    uint8_t rx_buf[5] = {0x0,0x0,0x0,0x0,0x0};

    LE.out(0);
    transmitData(tx_buf, rx_buf, 4);
    LE.out(1);
    transmitData(tx_buf, rx_buf, 5);

    uint32_t res = 0;
    for(int i = 0; i < 4; i++){
        res = res | (rx_buf[i] << (8*(3 - i) + 2)) ;
    }
    res = res | (((rx_buf[5] & 0x80) >> 6) );
    return res;
}
