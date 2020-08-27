#include <platform.h>

#include <stdint.h>
#include <sleep.h>
#include "../gpio_functions.h"

static int fd_ad9361;


int32_t spi_init(int fd){
    fd_ad9361 = fd;
    printf("spi_init fd_ad9361=%d\n", fd_ad9361);
}

int spi_write_then_read(
	struct spi_device *spi,
	const unsigned char *txbuf,
	unsigned n_tx,
	unsigned char *rxbuf,
	unsigned n_rx
){
	// printf("spi_write_then_read(fd_ad9361=%d, n_tx=%d, n_rx=%d)\n", fd_ad9361, n_tx, n_rx);
	uint8_t tx_buffer[20] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00
	};
	uint8_t rx_buffer[20] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00
	};
	uint8_t byte;

	for(byte = 0; byte < n_tx; byte++) {
		tx_buffer[byte] = (unsigned char)txbuf[byte];
	}

	int res = spi_transaction(fd_ad9361, tx_buffer, rx_buffer, n_tx + n_rx);

	for(byte = 0; byte < n_rx; byte++) {
		rxbuf[byte] = rx_buffer[byte + n_tx];
	}

	// printf("tx: [");
	// for(byte = 0; byte < n_tx; byte++){
	// 	printf("%X, ", txbuf[byte]);
	// }
	// printf("] ");

	// printf("rx: [");
	// for(byte = 0; byte < n_rx; byte++){
	// 	printf("%X, ", rxbuf[byte]);
	// }
	// printf("]\n");

	// printf("spi_transaction res==%d\n", res);

	return SUCCESS;
}


bool gpio_is_valid(int number)
{
	if(number >= 0){
		return 1;
	} else {
		return 0;
	}
}


void gpio_set_value(unsigned gpio, int value)
{
	printf("gpio_set_value(gpio=%u, value=%d)\n", gpio, value);
	if(value){
		pin_set(gpio, HIGH);
	} else {
		pin_set(gpio, LOW);
	}
}


void udelay(unsigned long usecs)
{
	usleep(usecs);
}

void mdelay(unsigned long msecs)
{
	usleep(msecs * 1000);
}
