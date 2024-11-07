#ifndef HW_LTAG_H_
#define HW_LTAG_H_

// Most of the definitions in this header file are GPIO pin mappings.

//---------- Buttons ----------//
#define HW_BTN_A      32
#define HW_BTN_B      33
#define HW_BTN_MENU   13
#define HW_BTN_OPTION  0
#define HW_BTN_SELECT 27
#define HW_BTN_START  39
#define HW_BTN_MASK ( \
	1LLU << HW_BTN_A | \
	1LLU << HW_BTN_B | \
	1LLU << HW_BTN_MENU | \
	1LLU << HW_BTN_OPTION | \
	1LLU << HW_BTN_SELECT | \
	1LLU << HW_BTN_START \
	)

//---------- Battery ----------//
#define HW_BAT_V 36

//---------- Sound (internal DAC) ----------//
#define HW_SND_A  26 // Audio output
#define HW_SND_EN 25 // Sound enable, active high

//---------- LCD ----------//
// The MSP1308 needs a GPIO pin for reset.
// The ESP32 EN pin on the Lolin32 Lite doesn't reset the MSP1308.
#define HW_LCD_MISO -1
#define HW_LCD_MOSI 23
#define HW_LCD_SCLK 18
#define HW_LCD_CS   -1
#define HW_LCD_DC    4
#define HW_LCD_RST   5
#define HW_LCD_BL   14

// esp-idf/components/driver/spi/include/driver/spi_master.h (v5.2, Line:29)
// #define SPI_MASTER_FREQ_40M (80 * 1000 * 1000 / 2) ///< 40MHz

#define HW_LCD_SPI_HOST SPI2_HOST
#define HW_LCD_SPI_FREQ SPI_MASTER_FREQ_40M // MHz

#define HW_LCD_INV 1
#define HW_LCD_DIR 0

#define HW_LCD_W 240
#define HW_LCD_H 240
#define HW_LCD_OFFSETX 0
#define HW_LCD_OFFSETY 0

#define HW_LCD_DRIVER 1

//---------- Laser tag ----------//
#define HW_LTAG_TRIGGER 32
#define HW_LTAG_HITLED  15
#define HW_LTAG_TX      12
#define HW_LTAG_RX      34

// LTAG A
/*
#define HW_LTAG_TRIGGER 35
#define HW_LTAG_HITLED 33
#define HW_LTAG_TX 16
#define HW_LTAG_RX 32
*/

#endif // HW_LTAG_H_
