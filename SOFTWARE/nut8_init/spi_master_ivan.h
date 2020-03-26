#ifndef SPI_MASTER_IVAN_H
#define SPI_MASTER_IVAN_H

#include <string>
#include <iostream>

struct SpiDeviceInfo {
    uint32_t mode = 0;
    uint8_t	 bits = 8;
    uint32_t speed = 10000000;
    uint16_t delay = 0;
};

class SPIMaster
{
public:
    std::string dev;
    SPIMaster(const std::string& dev);
    virtual ~SPIMaster();

    int32_t Open(const std::string& _dev);
    void Close();
    bool isOpen();

    SpiDeviceInfo GetDeviceInfo();
    int32_t SetSpiMode(uint32_t _mode);
    int32_t SetBitsPerMode(uint8_t _bits);
    int32_t SetSpeed(uint32_t _speed);
    int32_t SetDelay(uint16_t _delay);

    int32_t readReg(uint8_t _address, uint8_t* _data);
    int32_t transmitData(uint8_t* _tx_buf, uint8_t* _rx_buf, int length);
    int32_t writeReg(uint8_t _address, uint8_t _data);

    virtual void checkDevice() = 0;
protected:
    SPIMaster();

    int32_t Transfer(const uint8_t* _tx, const uint8_t* _rx, uint32_t _len);

    static void pabort(const std::string& s);

private:

    int32_t m_fd;
    SpiDeviceInfo m_devInfo;

};

#endif // SPI_MASTER_IVAN_H
