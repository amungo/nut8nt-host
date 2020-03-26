#ifndef MAX2871_H
#define MAX2871_H

#include "spi_master_ivan.h"
#include "gpio.h"
#include "defines.h"

// this parameters should be changed to suit the application
#define MAX2871_N_VAL (u32)24  // (16 bit feedback divider factor in integer mode!
#define MAX2871_F_VAL 0  // 12 bit  fractional divider !
#define MAX2871_M_VAL 100  // 12 bit modulus value in fractional mode !
#define MAX2871_R_VAL 1  // 10 bit ref. frequency divider value !
#define MAX2871_DIVA_VAL 0b110 //Sets RFOUT_ output divider mode !
#define MAX2871_CDIV_VAL 200 // 12 bit clock divide value (since fPFD = 20.0 MHz, 20.0MHz/100kHz = 100 !
#define MAX2871_B_POWER 0b00 // Sets RFOUTB single-ended output power: 00 = -4dBm, 01 = -1dBm, 10 = +2dBm, 11 = +5dBm ?
#define MAX2871_A_POWER 0b00 // Sets RFOUTA single-ended output power: 00 = -4dBm, 01 = -1dBm, 10 = +2dBm, 11 = +5dBm ?
// less likely to get changed
#define MAX2871_MUX 0b1100
#define MAX2871_MUX_MSB (MAX2871_MUX >> 3) // MSB of MUX !
#define MAX2871_MUX_LSB (MAX2871_MUX & ~(1 << 3)) // lower 3 bits of MUX !
#define MAX2871_CPL_MODE 0b00  //CPL linearity mode: 0b00in integer mode, 0b01 10% in frac mode, 0b10 for 20%, and 0b11 for 30% !
#define MAX2871_CPT_MODE 0b00  // 00 normal mode, 01 long reset, 10 force into source, 11 force into sink !
#define MAX2871_CP_CURRENT 0x0 // 4 bit CP current in mA ?
#define MAX2871_P_VAL 0x0     // 12-bit phase value for adjustment !
#define MAX2871_SD_VAL  0b00 //sigma delta noise mode: 0b00 low noise mode, 0b01 res, 0b10 low spur 1 mode, 0b11 low spur 2 mode !
#define MAX2871_VCO 0x0 // 6 bit VCO selection
#define MAX2871_CDIV_MODE 0b01 // clock divide mode: 0b00 mute until lock delay, 01 fast lock enable, 10 phase adjustment, 11 reserved
#define MAX2871_BS 400 // BS = fPFD / 50kHz = 20.0 MHz / 50 kHz
#define MAX2871_BS_MSB_VAL (MAX2871_BS >> 8) // 2 MSBs of Band select clock divider
#define MAX2871_BS_LSB_VAL (MAX2871_BS & ~(11 << 8))  // 8 LSBs of band select clock divider
#define MAX2871_VAS_DLY_VAL 0b11 // VCO Autoselect Delay:  11 when VAS_TEMP=1, 00 when VAS_TEMP=0
#define MAX2871_LD_VAL 0b01 //  lock-detect pin function: 00 = Low, 01 = Digital lock detect, 10 = Analog lock detect, 11 = High
#define MAX2871_ADC_MODE 0b001 // ADC mode: 001 temperature, 100 tune pin,

// register 0 masks
#define MAX2871_EN_INT (1 << 31)    // enables integer mode
#define MAX2871_N_SET (MAX2871_N_VAL << 15)     // puts value N on its place
#define MAX2871_N_MASK (0xFFFF << 15)   //16 bits at location 30:15
#define MAX2871_F_SET (MAX2871_F_VAL << 3)
#define MAX2871_F_MASK (0xFFF << 3)    //12 bits at location 14:3
#define MAX2871_REG_0 0b000

// register 1 masks
#define MAX2871_CPL (MAX2871_CPL_MODE << 29)  // Sets CP linearity mode
#define MAX2871_CPT (MAX2871_CPT_MODE << 27) // Sets CP test mode
#define MAX2871_PHASE (MAX2871_P_VAL << 15) // Sets phase adjustment
#define MAX2871_M_SET (MAX2871_M_VAL << 3)  // sets modulus value
#define MAX2871_M_MASK (0xFFF << 3)  // mask for M bits
#define MAX2871_REG_1 0b001

// register 2 masks
#define MAX2871_LDS (1 << 31) //Lock detect speed adjustment: 0 fPFD < 32 MHz, 1 pPFD > 32 MHz
#define MAX2871_SDN (MAX2871_SD_VAL << 29) //sets sigma-delta noise
#define MAX2871_MUX_2 (MAX2871_MUX_LSB << 26)  //sets MUX bits
#define MAX2871_DBR (1 << 25) //sets reference doubler mode, 0 disable, 1 enable
#define MAX2871_RDIV2 (1 << 24) //enable reference divide-by-2
#define MAX2871_R_DIV (MAX2871_R_VAL << 14) // set reference divider value
#define MAX2871_R_MASK (0x3FF << 14)
#define MAX2871_REG4DB (1 << 13) // sets double buffer mode
#define MAX2871_CP_SET  (MAX2871_CP_CURRENT << 9)  // sets CP current
#define MAX2871_LDF (1 << 8) // sets lock detecet in integer mode
#define MAX2871_LDP (1 << 7) //sets lock detect precision
#define MAX2871_PDP (1 << 6) // phase detect polarity
#define MAX2871_SHDN (1 << 5) // shutdown mode
#define MAX2871_CP_HZ (1 << 4) // sets CP to high Z mode
#define MAX2871_RST (1 << 3) // R and N counters reset
#define MAX2871_REG_2 0b010

//register 3 masks
#define MAX2871_VCO_SET (MAX2871_VCO << 26) // Manual selection of VCO and VCO sub-band when VAS is disabled.
#define MAX2871_VAS_SHDN (1 << 25)  // VAS shutdown mode
#define MAX2871_VAS_TEMP (1 << 24) // sets VAS temperature compensation
#define MAX2871_CSM (1 << 18) // enable cycle slip mode
#define MAX2871_MUTEDEL (1 << 17) // Delay LD to MTLD function to prevent ﬂickering
#define MAX2871_CDM (MAX2871_CDIV_MODE << 15)  // sets clock divider mode
#define MAX2871_CDIV (MAX2871_CDIV_VAL << 3) // sets clock divider value
#define MAX2871_REG_3 0b011

// register 4 masks
#define MAX2871_REG4HEAD (3 << 29) // Always program to 0b011
#define MAX2871_SDLDO (1 << 28) // Shutdown VCO LDO
#define MAX2871_SDDIV (1 << 27) // shutdown VCO divider
#define MAX2871_SDREF (1 << 26) // shutdown reference input mode
#define MAX2871_BS_MSB (MAX2871_BS_MSB_VAL << 24) // Sets band select 2 MSBs
#define MAX2871_FB (1 << 23)  //Sets VCO to N counter feedback mode
#define MAX2871_DIVA_MASK (7 << 20) // 3 bits at 22:20
#define MAX2871_DIVA (MAX2871_DIVA_VAL << 20) // Sets RFOUT_ output divider mode. Double buffered by register 0 when REG4DB = 1.
#define MAX2871_BS_LSB (MAX2871_BS_LSB_VAL << 12) // Sets band select 8 LSBs
#define MAX2871_SDVCO (1 << 11) // sets VCO shutdown mode
#define MAX2871_MTLD (1 << 10) // Sets RFOUT Mute until Lock Detect Mode
#define MAX2871_BDIV (1 << 9) // Sets RFOUTB output path select. 0 = VCO divided output, 1 = VCO fundamental frequency
#define MAX2871_RFB_EN (1 << 8) // Enable RFOUTB output
#define MAX2871_BPWR (MAX2871_B_POWER << 6) //RFOUTB Power
#define MAX2871_RFA_EN (1 << 5) // Enable RFOUTA output
#define MAX2871_APWR (MAX2871_A_POWER << 3) //RFOUTA Power
#define MAX2871_REG_4 0b100

// register 5 masks
#define MAX2871_VAS_DLY (MAX2871_VAS_DLY_VAL << 29) // VCO Autoselect Delay
#define MAX2871_SDPLL (1 << 25) // Shutdown PLL
#define MAX2871_F01 (1 << 24) // sets integer mode when F = 0
#define MAX2871_LD (MAX2871_LD_VAL << 22) // sets lock detection pin function
#define MAX2871_MUX_5 (MAX2871_MUX_MSB << 18) // sets MSB of MUX bits
#define MAX2871_ADCS (1 << 6) // Starts ADC mode
//#define ADCM ADC_MODE << 3 // ADC Mode  THIS PART IS DEFINED IN USER INTERFACE
#define MAX2871_REG_5 0b101

// register 6 masks (read only values)
#define MAX2871_ID_shift	28
#define MAX2871_POR_shift	23
#define MAX2871_ADC_shift	16
#define MAX2871_ADCV_shift	15
#define MAX2871_VASA_shift	9
#define MAX2871_V_shift		3
#define MAX2871_ADDR_shift	0

#define MAX2871_ID_mask		(0xf << 28)
#define MAX2871_POR_mask	(0x1 << 23)
#define MAX2871_ADC_mask	(0x7F << 16) // mask ADC value in register 6
#define MAX2871_ADCV_mask	(1 << 15) // validity of ADC read
#define MAX2871_VASA_mask	(1 << 9) // determines if VAS is active
#define MAX2871_V_mask		(0x3F << 3)  // Current VCO
#define MAX2871_ADDR_mask	(0x3 << 0)
#define MAX2871_REG_6		0b110


class MAX2871 : SPIMaster
{
public:
    MAX2871(const std::string& dev);
    void checkDevice() override;
    void reset();
    void writeToMAX2871(uint32_t data);
    uint32_t readFromMAX2871();
private:
    GPIO LE, CE, EN, PWR;
};

#endif // MAX2871_H
