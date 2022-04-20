#include "spi_master_ivan.h"

#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <linux/spi/spidev.h>
#include <errno.h>
#include <string.h>


using namespace std;

SPIMaster::SPIMaster(const std::string& _dev) {
    dev = _dev.data();

    Open(string(dev));
}

void SPIMaster::pabort(const string& s)
{
    perror(s.c_str());
    abort();
}

SPIMaster::SPIMaster() : m_fd(-1)
{
    cout << "Create ConnectionSPI" << endl;
}

SPIMaster::~SPIMaster()
{
    cout << "Destroy ConnectionSPI" << endl;
    Close();
}

int32_t SPIMaster::GetFD() {
    return m_fd;
}

int32_t SPIMaster::Open(const std::string& _dev)
{
    printf("Open %s\n", _dev.data());
    int32_t ret = 0;

    if(m_fd != -1)
        Close();

    m_fd = open(_dev.c_str(), O_RDWR);
    if(m_fd < 0)
        SPIMaster::pabort(" Can't open device");

    m_devInfo.mode |= SPI_CS_HIGH;

    // spi mode
    ret = ioctl(m_fd, SPI_IOC_WR_MODE32, &m_devInfo.mode);
    if(ret == -1)
        pabort(" can't set spi mode");

    ret = ioctl(m_fd, SPI_IOC_RD_MODE32, &m_devInfo.mode);
    if(ret == -1)
        pabort(" can't get spi mode");

    // bits per word
    ret = ioctl(m_fd, SPI_IOC_WR_BITS_PER_WORD, &m_devInfo.bits);
    if(ret == -1)
        pabort(" can't set bits per word");
    ret = ioctl(m_fd, SPI_IOC_RD_BITS_PER_WORD, &m_devInfo.bits);
    if(ret == -1)
        pabort(" can't get bits per word");

    // max speed hz
    ret = ioctl(m_fd, SPI_IOC_WR_MAX_SPEED_HZ, &m_devInfo.speed);
    if(ret == -1)
        pabort(" can't set max speed hz");
    ret = ioctl(m_fd, SPI_IOC_RD_MAX_SPEED_HZ, &m_devInfo.speed);
    if(ret == -1)
        pabort(" can't get max speed hz");

    printf(" spi mode: %x\n", m_devInfo.mode);
    printf(" bits per word: %x\n", m_devInfo.bits);
    printf(" max speed: %d Hz %d(KHz) \n", m_devInfo.speed, m_devInfo.speed / 1000);

    return ret;
}

void SPIMaster::Close()
{
    if(m_fd != -1) {
        close(m_fd);
        m_fd = -1;
    }
}

bool SPIMaster::isOpen()
{
    return (m_fd != -1);
}


SpiDeviceInfo SPIMaster::GetDeviceInfo()
{
    return m_devInfo;
}

int32_t SPIMaster::SetSpiMode(uint32_t _mode)
{
    int32_t ret = ioctl(m_fd, SPI_IOC_WR_MODE32, &_mode);
    if(ret == -1) {
        cerr << "ioctl(m_fd, SPI_IOC_WR_MODE32, &_mode): " << strerror(ret) << endl;
        return ret;
    }
    ret = ioctl(m_fd, SPI_IOC_RD_MODE32, &_mode);
    if(ret == -1) {
        cerr << "ioctl(m_fd, SPI_IOC_RD_MODE32, &_mode): " << strerror(ret) << endl;
        return ret;
    }

    m_devInfo.mode = _mode;
    return ret;
}

int32_t SPIMaster::SetBitsPerMode(uint8_t _bits)
{

    int32_t ret = ioctl(m_fd, SPI_IOC_WR_BITS_PER_WORD, &_bits);
    if(ret == -1) {
        cerr << "ioctl(m_fd, SPI_IOC_WR_BITS_PER_WORD, &_bits): " << strerror(ret) << endl;
        return ret;
    }
    ret = ioctl(m_fd, SPI_IOC_RD_BITS_PER_WORD, &_bits);
    if(ret == -1) {
        cerr << "ioctl(m_fd, SPI_IOC_RD_BITS_PER_WORD, &_bits): " << strerror(ret) << endl;
        return ret;
    }

    m_devInfo.bits = _bits;
    return ret;
}

int32_t SPIMaster::SetSpeed(uint32_t _speed)
{
    int32_t ret = ioctl(m_fd, SPI_IOC_WR_MAX_SPEED_HZ, &_speed);
    if(ret == -1) {
        cerr << "ioctl(m_fd, SPI_IOC_WR_MAX_SPEED_HZ, &_speed): " << strerror(ret) << endl;
        return ret;
    }

    ret = ioctl(m_fd, SPI_IOC_RD_MAX_SPEED_HZ, &_speed);
    if(ret == -1) {
        cerr << "ioctl(m_fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed): " << strerror(ret) << endl;
        return ret;
    }

    m_devInfo.speed = _speed;

    return ret;
}

int32_t SPIMaster::SetDelay(uint16_t _delay)
{
    if(_delay >= 0) {
        m_devInfo.delay = _delay;
        return 0;
    }
}

int32_t SPIMaster::Transfer(const uint8_t* _tx, const uint8_t* _rx, uint32_t _len)
{
    //printf("Transfer %d\n", _len);
    int ret;
    uint32_t mode = m_devInfo.mode;

    struct spi_ioc_transfer txinfo;
    memset(&txinfo, 0, sizeof(struct spi_ioc_transfer));
    txinfo.tx_buf = (__u64)_tx;
    txinfo.rx_buf = (__u64)_rx;
    txinfo.len = _len;
    txinfo.delay_usecs = m_devInfo.delay;
    txinfo.speed_hz = m_devInfo.speed;
    txinfo.bits_per_word = m_devInfo.bits;

    if(mode & SPI_TX_QUAD)
        txinfo.tx_nbits = 4;
    else if(mode & SPI_TX_DUAL)
        txinfo.tx_nbits = 2;
    else if(mode & SPI_RX_QUAD)
        txinfo.rx_nbits = 4;
    else if(mode & SPI_RX_DUAL)
        txinfo.rx_nbits = 2;

    if(!(mode & SPI_LOOP)) {
        if(mode & (SPI_TX_QUAD | SPI_TX_DUAL))
            txinfo.rx_buf = 0;
        else if(mode & (SPI_RX_QUAD | SPI_RX_DUAL))
            txinfo.tx_buf = 0;
    }

    ret = ioctl(m_fd, SPI_IOC_MESSAGE(1), &txinfo);
    if(ret < 1)
        pabort("can't send spi message");

    return 0;
}

int32_t SPIMaster::readReg(uint8_t _address, uint8_t* _data) {
    int ret = 0;
    uint8_t rd[] = {0,0};
    uint8_t wr[2];

    wr[1] = 0xFF;
    wr[0] = _address;
    wr[0] |= 0x80;

    ret = Transfer(wr, rd, 2);

    if(ret == 0)
        *_data = rd[1];

    return ret;
}

int32_t SPIMaster::transmitData(uint8_t* _tx_buf, uint8_t* _rx_buf, int length) {
    return Transfer(_tx_buf, _rx_buf, length);
}

int32_t SPIMaster::writeReg(uint8_t _address, uint8_t _data) {
    uint8_t rd[] = {0,0};
    uint8_t wr[2];

    wr[1] = _data;
    wr[0] = _address & 0x7F;

    return Transfer(wr, rd, 2);
}
