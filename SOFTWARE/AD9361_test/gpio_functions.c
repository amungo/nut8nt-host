#include "gpio_functions.h"
#include "gpio_pins.h"

char* export_nodes[] = {
	"/sys/class/gpio/export",
	"/sys/class/gpio/unexport"
};

char* direction_strings[] = {
	"in",
	"out"
};

int direction_strings_sizes[] = {
	3,
	4
};

char* state_strings[] = {
	"0",
	"1"
};


int gpio_number(int gpio_pin){
	return GPIO_BASE + gpio_pin;
}

int gpio_export(int gpio_pin, Export_t ex){

	// printf("export pin %d\n", gpio_pin);
	int fd = 0;

	fd = open(export_nodes[ex], O_WRONLY);
	
	if (fd < 0){
		printf("[E] open(%s)==%d\n", export_nodes[ex], fd);
		return fd;
	} else {
		// printf("[I] open(%s)==%d\n", export_nodes[ex], fd);
	}

	char gpio_num[4];
	sprintf(gpio_num, "%03d", gpio_number(gpio_pin));
	
	int res = write(fd, gpio_num, 4);

	close(fd);
	if(res == 4){
		return 0;
	} else {
		return -1;
	}	
}

int set_direction(int gpio_pin, Direction_t dir){

	char buffer[100];
	sprintf(buffer, "/sys/class/gpio/gpio%d/direction", gpio_number(gpio_pin));
	
	int fd = 0;
	fd = open(buffer, O_WRONLY);
	
	if(fd <= 0){
		printf("[E] open(%s)==%d\n", buffer, fd);
		return fd;
	} else {
		// printf("[I] open(%s)==%d\n", buffer, fd);
	}

	int res = write(fd, direction_strings[dir], 4);
	int res_should_be = direction_strings_sizes[dir];

	if(res != res_should_be){
		printf("[E] can't set pin %d %s\n", gpio_pin, direction_strings[dir]);
	} else {
		printf("[I] set pin %d %s ok\n", gpio_pin, direction_strings[dir]);
	}

	close(fd);
	if(res == res_should_be){
		return 0;
	} else {
		return -1;
	}
}

int pin_set(int gpio_pin, State_t state){

	char buffer[100];
	sprintf(buffer, "/sys/class/gpio/gpio%d/value", gpio_number(gpio_pin));
	
	int fd = 0;
	fd = open(buffer, O_WRONLY);
	
	if(fd <= 0){
		printf("[E] open(%s)==%d\n", buffer, fd);
		return fd;
	} else {
		// printf("[I] open(%s)==%d\n", buffer, fd);
	}

	int res = write(fd, state_strings[state], 2);
	int res_should_be = 2;

	if(res != res_should_be){
		printf("[E] can't set pin %d %s\n", gpio_pin, state_strings[state]);
	} else {
		printf("[I] set pin %d %s ok\n", gpio_pin, state_strings[state]);
	}

	close(fd);
	if(res == res_should_be){
		return 0;
	} else {
		return -1;
	}
}

int pin_init(int gpio_pin, Direction_t dir){
	
	int res = 0;
	res = gpio_export(gpio_pin, EXPORT);
	if(res) goto exit;
	
	res = set_direction(gpio_pin, dir);
	if(res) goto set_dir_error;
	
	goto exit;

set_dir_error:
	gpio_export(gpio_pin, UNEXPORT);

exit:
     printf("pin_init(%d, %d)=%d\n", gpio_pin, dir, res);
	return res;
}

int all_gpio_init(){

    int res;
	res = pin_init(AD9364_RESET_PIN, DIR_OUT);

    res = pin_init(AD9364_TX_PIN, DIR_OUT);
    res = pin_init(AD9364_RX_PIN, DIR_OUT);
	
	goto exit;

unexport_tx_oen:
	gpio_export(TXOENPIN, UNEXPORT);
unexport_recven:
	gpio_export(RCVENPIN, UNEXPORT);
exit:
	return 0;

}

void gpio_deinit(){
    int res;
    res = gpio_export(AD9364_RESET_PIN, UNEXPORT);

    res = gpio_export(AD9364_TX_PIN, UNEXPORT);
    res = gpio_export(AD9364_RX_PIN, UNEXPORT);
}
