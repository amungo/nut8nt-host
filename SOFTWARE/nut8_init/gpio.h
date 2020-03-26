#ifndef GPIO_H
#define GPIO_H

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/gpio.h>

#define GPIO_BASE   338
#define GPIO_DIR_IN 0
#define GPIO_DIR_OUT 1
//#define GPIO_BASE   0

class GPIO
{
public:
    int pin;
    GPIO(int _pin, int dir);
    ~GPIO();
    void out(int val);
    int in();
private:
    int fd = 0;
};

#endif // GPIO_H
