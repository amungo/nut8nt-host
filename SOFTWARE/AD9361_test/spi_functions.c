#include "spi_functions.h"

static uint8_t mode;
static uint8_t bits = 8;
static uint8_t lsb;
static uint32_t speed = 5000;
static uint16_t delay = 1000;

#define READ_SPI_OP		0x80

#define SPI_MAJOR_MAX	4
#define SPI_MINOR_MAX	4

int spi_write_byte(int fd, uint8_t addr, uint8_t data){


	uint8_t tx_buf[2];
	tx_buf[0] = addr;
	tx_buf[1] = data;

	uint8_t rx_buf[2];
	rx_buf[0] = 0;
	rx_buf[1] = 0;

	struct spi_ioc_transfer tr = {
		.tx_buf = (unsigned long)tx_buf,
		.rx_buf = (unsigned long)rx_buf,
		.len = 2,
		.cs_change = 0,
		.delay_usecs = delay,
		.speed_hz = speed,
		.bits_per_word = bits,
	};

	int res = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
	if (res < 1){
		perror("can't send spi message");
	}

	// printf("[write] %X %X\n", addr, data);

	return res;
}

int spi_read_byte(int fd, uint8_t addr, uint8_t* data) {

	uint8_t tx_buf[2];
	tx_buf[0] = addr | READ_SPI_OP;
	tx_buf[1] = 0xFF;
	// tx_buf[2] = 0xFF;

	uint8_t rx_buf[2];
	rx_buf[0] = 0;
	rx_buf[1] = 0;
	// rx_buf[2] = 0;

	struct spi_ioc_transfer tr = {
		.tx_buf = (unsigned long)tx_buf,
		.rx_buf = (unsigned long)rx_buf,
		.len = 2,
		.cs_change = 0,
		.delay_usecs = delay,
		.speed_hz = speed,
		.bits_per_word = bits,
	};

	int res = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
	if (res < 1){
		perror("can't send spi message");
	}

	*data = rx_buf[1];

	// printf("[read] %X %X %X\n", addr, rx_buf[1], rx_buf[2]);

	return res;
}

int spi_transaction(int fd, uint8_t* tx_buf, uint8_t* rx_buf, int length){

	struct spi_ioc_transfer tr = {
		.tx_buf = (unsigned long)tx_buf,
		.rx_buf = (unsigned long)rx_buf,
		.len = length,
		.cs_change = 0,
		.delay_usecs = delay,
		.speed_hz = speed,
		.bits_per_word = bits,
	};

	int res = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
	if (res < 1){
		perror("can't send spi message");
	}

	return res;
}

int print_register(int fd, uint8_t addr) {

	uint8_t data = 0;
	int res = spi_read_byte(fd, addr, &data);
	if(res >= 0){
		printf("[%X] : %X\n", addr, data);
	} else {
		printf("[%X] : ERR\n", addr);
	}
	return res;
}

int print_registers(int fd, uint8_t addr_start, uint8_t addr_end) {

	int res = 0;
	uint8_t addr = addr_start;
	printf("Reading [%X-%X]\n", addr_start, addr_end);
	for(; addr <= addr_end; addr++){
		res = print_register(fd, addr);
		if(res < 0) break;
		usleep(10000);
	}

	return res;
}

int open_bus(int major, int minor){

	if(major < 0 || major > SPI_MAJOR_MAX){
		return -1;
	}
	
	if(minor < 0 || minor >= SPI_MINOR_MAX){
		return -1;
	}

	char* filename = (char*)malloc(20);
	sprintf(filename, "/dev/spidev%d.%d", major, minor);

	int res = 0;
	int fd = open(filename, O_RDWR);

	printf("Open SPI bus %s\n", filename);
	if(fd < 0) {
		perror("Failed to open SPI bus");
		exit(1);
	}


	// res = ioctl(fd, SPI_IOC_WR_MODE, &mode);
	// if (res < 0) perror("can't set spi mode");

	res = ioctl(fd, SPI_IOC_RD_MODE, &mode);
	if (res < 0) perror("can't get SPI mode");

	printf("SPI mode=0x%X\n", mode);

	res = ioctl(fd, SPI_IOC_RD_LSB_FIRST, &lsb);
	if (res < 0) perror("can't get LSB first");

	res = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
	if (res < 0) perror("can't set bits per word");

	res = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
	if (res < 0) perror("can't get bits per word");

	res = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
	if (res < 0) perror("can't set max speed hz");

	res = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
	if (res < 0) perror("can't get max speed hz");

	// printf("spi mode: %d\n", mode);
	// printf("lsb first: %d\n", lsb);
	// printf("bits per word: %d\n", bits);
	// printf("max speed: %d Hz (%d KHz)\n", speed, speed/1000);

	printf("SPI bus open. Mode %d, lsb first %d, bits %d, speed %d KHz\n", mode, lsb, bits, speed/1000);


	return fd;
}

// int spi_write_word(int fd, uint32_t data){

// 	uint8_t tx_buf[4];
// 	// byte reverse
// 	for (uint8_t i = 0; i < 4; i++) {
// 		tx_buf[i] = (data & (0xFF << 8*(3 - i))) >> ( 8*(3 - i) );
// 	}

// 	uint8_t rx_buf[4];
// 	rx_buf[0] = 0;
// 	rx_buf[1] = 0;
// 	rx_buf[2] = 0;
// 	rx_buf[3] = 0;

// 	struct spi_ioc_transfer tr = {
// 		.tx_buf = (unsigned long)tx_buf,
// 		.rx_buf = (unsigned long)rx_buf,
// 		.len = 4,
// 		.cs_change = 0,
// 		.delay_usecs = delay,
// 		.speed_hz = speed,
// 		.bits_per_word = bits,
// 	};

// 	int res = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
// 	if (res < 1){
// 		perror("can't send spi message");
// 	}

// 	printf("[write word] %X\n", data);

// 	return res;
// }

// void print_state(){
	
// 	int fd = open_bus();

// 	print_registers(fd, 0x3f, 0x42);
// 	print_registers(fd, 0x108, 0x108);
// 	// print_registers(fd, 0x108, 0x1ff);
// 	// print_registers(fd, 0x200, 0x201);
// 	// print_registers(fd, 0x550, 0x5CB);
// 	printf("Clock status: (expecting 11B:01; 56E:30; 56F:80)\n");
// 	print_registers(fd, 0x11B, 0x11B);
// 	print_registers(fd, 0x056E, 0x0574);
// 	printf("Subclass, bits per sample:\n");
// 	print_registers(fd, 0x0590, 0x0590);
// 	printf( " \n");


// 	//printf("Checksums: \t");
// 	//print_registers(fd, 0x05A0, 0x05A3);

// 	//printf("Lanes order: \t");
// 	//print_registers(fd, 0x05B2, 0x05B6);

// }

// void read_reg(uint32_t addr){

// 	int fd = open_bus();

// 	print_register(fd, addr);
// 	close(fd);

// }

// void read_regs(uint32_t addr_start, uint32_t addr_end){

// 	int fd = open_bus();
	
// 	print_registers(fd, addr_start, addr_end);
// 	close(fd);

// }

// void write_reg(uint32_t addr, uint32_t data){
	
// 	int fd = open_bus();

// 	write_byte(fd, addr, data);
// 	print_register(fd, addr);
// 	close(fd);

// }
