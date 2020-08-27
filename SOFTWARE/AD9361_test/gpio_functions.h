#ifndef GPIO_FUNCTIONS_H_
#define GPIO_FUNCTIONS_H_

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

#define GPIO_BASE 338

typedef enum Direction{
	DIR_IN = 0,
	DIR_OUT = 1
} Direction_t;

typedef enum Export{
	EXPORT = 0,
	UNEXPORT = 1
} Export_t;

typedef enum State{
	LOW = 0,
	HIGH = 1
} State_t;

int gpio_number(int gpio_pin);
int gpio_export(int gpio_pin, Export_t ex);
int set_direction(int gpio_pin, Direction_t dir);
int pin_set(int gpio_pin, State_t state);
int all_gpio_init();
void gpio_deinit();

#endif
