#include "max2871.h"
#include "gpio_functions.h"
#include "spi_functions.h"
#include "gpio_pins.h"

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

void write_to_synt(int fd, uint32_t data){
	/*
	* This function takes 32-bit data and parses it on four 8-bit transmitts. The 29 most-significant
	* bits (MSBs) are data, and the three least-significant bits (LSBs) are the register address.
	*/
	
	uint8_t tx_buf[4];
	uint8_t rx_buf[4];

	for(int i = 0; i < 4; i++){
		tx_buf[i] = (data & (0xFF << 8*(3 - i))) >> ( 8*(3 - i) );
	}

	pin_set(MAX2871_SS_PIN, LOW);
	spi_transaction(fd, tx_buf, rx_buf, 4);
	pin_set(MAX2871_SS_PIN, HIGH);
	usleep(50000);
}

uint32_t read_from_synt(int fd){

	uint8_t tx_buf[5] = {0x0,0x0,0x0,MAX2871_REG_6,0x0};
	uint8_t rx_buf[5] = {0x0,0x0,0x0,0x0,0x0};

	pin_set(MAX2871_SS_PIN, LOW);
	spi_transaction(fd, tx_buf, rx_buf, 4);
	pin_set(MAX2871_SS_PIN, HIGH);
	spi_transaction(fd, tx_buf, rx_buf, 5);

	uint32_t res = 0;
	for(int i = 0; i < 4; i++){
		res = res | (rx_buf[i] << (8*(3 - i) + 2)) ;
	}
	res = res | (((rx_buf[5] & 0x80) >> 6) );
	return res;
}

void max2871_config(int fd){

	// disable RF output
	pin_set(MAX2871_EN_PIN, LOW);

	for (int j = 0; j < 3; j++)	{
		for (int i = SIZE_MAX2871_CONFIG-1; i >= 0; i--){   // 6 write registers
			write_to_synt(fd, max_config[i]);
			max_registers[i] = max_config[i];
		}
		usleep(500000);
	}

	// Enable RF output
	pin_set(MAX2871_EN_PIN, HIGH);
	max_registers[4] =  MAX2871_RFA_EN | max_registers[4];
	write_to_synt(fd, max_registers[4]);
	write_to_synt(fd, max_registers[4]);
	usleep(50000);
	uint32_t read_data = read_from_synt(fd);

	uint32_t id = (read_data & MAX2871_ID_mask) >> MAX2871_ID_shift;
	uint32_t por = (read_data & MAX2871_POR_mask) >> MAX2871_POR_shift;
	uint32_t adc_code = (read_data & MAX2871_ADC_mask) >> MAX2871_ADC_shift;
	float adc_value = 0.315 + 0.0165;
	uint32_t adc_valid = (read_data & MAX2871_ADCV_mask) >> MAX2871_ADCV_shift;
	uint32_t vasa = (read_data & MAX2871_VASA_mask) >> MAX2871_VASA_shift;
	uint32_t v = (read_data & MAX2871_V_mask) >> MAX2871_V_shift;
	uint32_t addr = (read_data & MAX2871_ADDR_mask) >> MAX2871_ADDR_mask;

	printf("id        = ");
	if(id == 0b0110){
		printf("MAX2870\n");
	} else if(id == 0b0111){
		printf("MAX2871\n");
	} else {
		printf("wrong id\n");
	}
	printf("por       = 0x%X\n", por);
	printf("adc_code  = 0x%X = %f\n", adc_code, adc_value);
	printf("adc_valid = 0x%X\n", adc_valid);
	printf("vasa      = 0x%X\n", vasa);
	printf("v         = 0x%X\n", v);
	printf("addr      = 0x%X\n", addr);


	// printf("Version MAX = 0x%0x \r\n",(read_data & MAX2871_ID_MASK ) >> 28);
	// printf("POR = %0d \r\n",(read_data & MAX2871_POR_MASK ) >> 23);
	// float VCO = 0.315 + 0.0165 * (float)((read_data & MAX2871_ADC_mask ) >> 16);
	// printf("VCO voltage:  = %0f \r\n",VCO);
	// printf("VCO voltage = %0d \r\n",(read_data & MAX2871_ADCV ) >> 15);
	// printf("VASA is active = %0d \r\n",(read_data & MAX2871_VASA ) >> 9);
	// printf("Current VCO = 0x%0x \r\n",(read_data & MAX2871_V ) >> 3);

}

void max2871_hard_reset() {
	pin_set(MAX2871_SS_PIN, HIGH);
	pin_set(MAX2871_EN_PIN, LOW);
	pin_set(MAX2871_CE_PIN, LOW);
	pin_set(MAX2871_PWR_ON_PIN, LOW);
	usleep(500000);
	pin_set(MAX2871_SS_PIN, HIGH);
	pin_set(MAX2871_EN_PIN, HIGH);
	pin_set(MAX2871_PWR_ON_PIN, HIGH);
	pin_set(MAX2871_CE_PIN, HIGH);
}
