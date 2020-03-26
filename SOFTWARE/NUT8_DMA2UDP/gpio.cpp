#include "gpio.h"


GPIO::GPIO(int _pin, int dir) {
    pin = _pin;

    char buffer[100];
    int fd = 0;

    fd = open("/sys/class/gpio/export", O_WRONLY);

    if (fd < 0){
        sprintf(buffer, " Can't open /sys/class/gpio/export for GPIO %d", pin);
        perror(buffer);
    }

    char gpio_num[4];
    sprintf(gpio_num, "%03d", pin + GPIO_BASE);

    int res = write(fd, gpio_num, 4);

    close(fd);
    if(res < 0){
        sprintf(buffer, " Can't write /sys/class/gpio/export for GPIO %d", pin);
        perror(buffer);
        //perror(" Can't write /sys/class/gpio/export");
        return;
    }



    sprintf(buffer, "/sys/class/gpio/gpio%d/direction", pin + GPIO_BASE);

    fd = 0;
    fd = open(buffer, O_WRONLY);

    if(fd <= 0){
        perror(" Failed to open GPIO");
    }

    if (dir == GPIO_DIR_OUT)
        res = write(fd, "out", 4);
    else
        res = write(fd, "in", 3);

    close(fd);
}

GPIO::~GPIO() {
    char buffer[100];
    int fd = open("/sys/class/gpio/unexport", O_WRONLY);

    if (fd < 0){
        sprintf(buffer, " Can't open /sys/class/gpio/unexport for GPIO %d", pin);
        perror(buffer);
    }

    char gpio_num[4];
    sprintf(gpio_num, "%03d", pin + GPIO_BASE);
    int res = write(fd, gpio_num, 4);

    close(fd);
    if(res < 0){
        sprintf(buffer, " Can't write /sys/class/gpio/unexport for GPIO %d", pin);
        perror(buffer);
        //perror(" Can't write /sys/class/gpio/unexport");
        return;
    }
}

void GPIO::out(int val) {
    char buffer[100];
    sprintf(buffer, "/sys/class/gpio/gpio%d/value", pin + GPIO_BASE);

    int fd = 0;
    fd = open(buffer, O_WRONLY);

    if(fd <= 0){
        perror(" Failed to write GPIO");
        return;
    }

    char dir_str[2] = {0};
    dir_str[0] = '0' + val;
    int res = write(fd, &dir_str, 2);

    close(fd);
}

int GPIO::in() {
    char buffer[100];
    sprintf(buffer, "/sys/class/gpio/gpio%d/value", pin + GPIO_BASE);

    int fd = 0;
    fd = open(buffer, O_RDONLY);

    if(fd <= 0){
        perror(" Failed to read GPIO");
    }

    char ch;
    read(fd, &ch, 1);
    ch -= '0';

    close(fd);

    return ch;
}
