/*
 * gpio_pins.h
 *
 *  Created on: Jun 25, 2019
 *      Author: ivan
 */

#ifndef GPIO_PINS_H_
#define GPIO_PINS_H_

// power for antenna
#define ANT_ON_PIN 39

// Power
#define RCVENPIN 44

// turn on internal REF
#define TXOENPIN 91 //91

// emio reset bin to peripheral
#define PL_RESET_PIN 173

// ads pins
#define ADS5292_SYNC_INTF 88 //88
#define ADS5292_RESET_RST_PIN 89 //89
#define ADS5292_RESET_PWDN_PIN 90 //90
#define ADS5292_SYNC_OK_INTF 126

// output to MAX
#define MAX2871_PWR_ON_PIN 84 //84
#define MAX2871_CE_PIN 85 //85
#define MAX2871_EN_PIN 86 //86
#define MAX2871_SS_PIN 87 //87

// input from MAX
#define MAX2871_LD_PIN 124
#define MAX2871_MUX_PIN 125


// ad pins
#define AD9364_RESET_PIN  89 // +
#define AD9364_CTL0_PIN	  78 // + ctrl_in
#define AD9364_CTL1_PIN   79 // +
#define AD9364_CTL2_PIN	  80 // +
#define AD9364_CTL3_PIN	  81 // +

#define AD9364_TX_PIN     92
#define AD9364_RX_PIN     91
#define AD9364_AUTOGAIN   90

#endif
