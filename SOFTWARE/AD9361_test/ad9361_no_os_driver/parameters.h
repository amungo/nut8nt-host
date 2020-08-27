/***************************************************************************//**
 *   @file   parameters.h
 *   @brief  Parameters Definitions.
 *   @author DBogdan (dragos.bogdan@analog.com)
********************************************************************************
 * Copyright 2013(c) Analog Devices, Inc.
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *  - Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  - Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *  - Neither the name of Analog Devices, Inc. nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *  - The use of this software may or may not infringe the patent rights
 *    of one or more patent holders.  This license does not release you
 *    from the requirement that you obtain separate licenses from these
 *    patent holders to use this software.
 *  - Use of the software either in source or binary form, must be run
 *    on or directly connected to an Analog Devices Inc. component.
 *
 * THIS SOFTWARE IS PROVIDED BY ANALOG DEVICES "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, NON-INFRINGEMENT,
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL ANALOG DEVICES BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, INTELLECTUAL PROPERTY RIGHTS, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*******************************************************************************/
#ifndef __PARAMETERS_H__
#define __PARAMETERS_H__

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/
#include <xparameters.h>

/******************************************************************************/
/********************** Macros and Constants Definitions **********************/
/******************************************************************************/
#define XPAR_AXI_AD9361_0_BASEADDR XPAR_M_AXI_AD9364_BASEADDR
#ifdef XPAR_AXI_AD9361_0_BASEADDR
#define AD9361_RX_0_BASEADDR		XPAR_AXI_AD9361_0_BASEADDR
#define AD9361_TX_0_BASEADDR		XPAR_AXI_AD9361_0_BASEADDR + 0x4000
#else
#define AD9361_RX_0_BASEADDR		XPAR_AXI_AD9361_BASEADDR
#define AD9361_TX_0_BASEADDR		XPAR_AXI_AD9361_BASEADDR + 0x4000
#endif
#ifdef XPAR_AXI_AD9361_1_BASEADDR
#define AD9361_RX_1_BASEADDR		XPAR_AXI_AD9361_1_BASEADDR
#define AD9361_TX_1_BASEADDR		XPAR_AXI_AD9361_1_BASEADDR + 0x4000
#else
#ifdef XPAR_AXI_AD9361_0_BASEADDR
#define AD9361_RX_1_BASEADDR		XPAR_AXI_AD9361_0_BASEADDR
#define AD9361_TX_1_BASEADDR		XPAR_AXI_AD9361_0_BASEADDR + 0x4000
#else
#define AD9361_RX_1_BASEADDR		XPAR_AXI_AD9361_BASEADDR
#define AD9361_TX_1_BASEADDR		XPAR_AXI_AD9361_BASEADDR + 0x4000
#endif
#endif
#ifdef XPAR_AXI_DMAC_0_BASEADDR
#define CF_AD9361_RX_DMA_BASEADDR	XPAR_AXI_DMAC_0_BASEADDR
#else
//#define CF_AD9361_RX_DMA_BASEADDR	XPAR_AXI_AD9361_ADC_DMA_BASEADDR
#endif
#ifdef XPAR_AXI_DMAC_1_BASEADDR
#define CF_AD9361_TX_DM0x1400A_BASEADDR	XPAR_AXI_DMAC_1_BASEADDR
#else
//#define CF_AD9361_TX_DMA_BASEADDR	XPAR_AXI_AD9361_DAC_DMA_BASEADDR
#endif

// ------------------------------------------------------------
//
#define GPIO_DEVICE_ID				XPAR_PSU_GPIO_0_DEVICE_ID
#define GPIO_TXOEN_PIN				91 // +
#define GPIO_RESET_PIN				82 // +
//#define GPIO_SYNC_PIN				83 // +
//#define GPIO_ENABLE_AGC_PIN			83 // ON MODULE AD9364
//#define GPIO_TXNRX_PIN        		126 // ON MODULE AD9364
#define ADC_DDR_BASEADDR			NULL
#define DAC_DDR_BASEADDR			NULL
#define SPI_DEVICE_ID				XPAR_SPI_0_DEVICE_ID
//
// ------------------------------------------------------------
#endif // __PARAMETERS_H__
