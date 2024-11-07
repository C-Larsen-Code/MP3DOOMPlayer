// Modified from: https://github.com/nopnop2002/esp-idf-st7789
// See also:
//   https://learn.adafruit.com/adafruit-gfx-graphics-library
//   https://github.com/adafruit/Adafruit-GFX-Library
//   https://github.com/adafruit/TFTLCD-Library
//   https://github.com/adafruit/Adafruit_ILI9341

#include <string.h> // strlen, memcpy
#include <math.h> // cosf, sinf

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_heap_caps.h"
#include "esp_log.h"

#include "hw.h"
#include "lcd.h"

#define _DEBUG_ 0

#define LCD_MOSI HW_LCD_MOSI
#define LCD_SCLK HW_LCD_SCLK
#define LCD_CS   HW_LCD_CS
#define LCD_DC   HW_LCD_DC
#define LCD_RST  HW_LCD_RST
#define LCD_BL   HW_LCD_BL

#define LCD_INV      HW_LCD_INV
#define LCD_SPI_HOST HW_LCD_SPI_HOST
#define LCD_SPI_FREQ HW_LCD_SPI_FREQ

#define LCD_OFFSETX HW_LCD_OFFSETX
#define LCD_OFFSETY HW_LCD_OFFSETY

#define LCD_DRIVER HW_LCD_DRIVER

#define swap(T,a,b) {T t = (a); (a) = (b); (b) = t;}

#define M_PIf 3.14159265358979323846f

#define SWAP16(c) (((c) << 8) | ((c) >> 8))

typedef struct {
	coord_t     width;
	coord_t     height;
	coord_t     offsetx;
	coord_t     offsety;
	direction_t font_direction;
	uint8_t     font_size;
	bool        font_back_en;
	color_t     font_back_color;
	int8_t      res;
	int8_t      dc;
	int8_t      bl;
	spi_device_handle_t SPIHandle;
	bool        use_frame_buffer;
	color_t   *frame_buffer;
} TFT_t;

typedef enum {
	SPI_Command_Mode = 0,
	SPI_Data_Mode = 1
} spi_mode_t;

static TFT_t device;
static TFT_t *dev = &device;

static const char *TAG = "lcd";

static int32_t clock_freq_hz = LCD_SPI_FREQ;

#include "glcdfont.c" // unsigned char font[];

#define delayMS(ms) \
	vTaskDelay(((ms)+(portTICK_PERIOD_MS-1))/portTICK_PERIOD_MS)

//----------------------------------------------------------------------------//
// SPI
//----------------------------------------------------------------------------//

#define BUF_LEN 512
static uint16_t buffer[BUF_LEN];

static void spi_master_init(TFT_t *dev, int16_t GPIO_MOSI, int16_t GPIO_SCLK, int16_t GPIO_CS, int16_t GPIO_DC, int16_t GPIO_RST, int16_t GPIO_BL)
{
	esp_err_t ret;

	ESP_LOGI(TAG, "GPIO_BL=%hd", GPIO_BL);
	if ( GPIO_BL >= 0 ) {
		gpio_reset_pin(GPIO_BL);
		gpio_set_direction( GPIO_BL, GPIO_MODE_OUTPUT );
		gpio_set_level( GPIO_BL, 0 );
	}

	ESP_LOGI(TAG, "GPIO_CS=%hd",GPIO_CS);
	if ( GPIO_CS >= 0 ) {
		gpio_reset_pin( GPIO_CS );
		gpio_set_direction( GPIO_CS, GPIO_MODE_OUTPUT );
		gpio_set_level( GPIO_CS, 0 );
	}

	ESP_LOGI(TAG, "GPIO_DC=%hd",GPIO_DC);
	gpio_reset_pin( GPIO_DC );
	gpio_set_direction( GPIO_DC, GPIO_MODE_OUTPUT );
	gpio_set_level( GPIO_DC, 0 );

	ESP_LOGI(TAG, "GPIO_RST=%hd", GPIO_RST);
	if ( GPIO_RST >= 0 ) {
		gpio_reset_pin( GPIO_RST );
		gpio_set_direction( GPIO_RST, GPIO_MODE_OUTPUT );
		gpio_set_level( GPIO_RST, 1 );
	}

	ESP_LOGI(TAG, "GPIO_MOSI=%hd", GPIO_MOSI);
	ESP_LOGI(TAG, "GPIO_SCLK=%hd", GPIO_SCLK);
	spi_bus_config_t buscfg = {
		.mosi_io_num = GPIO_MOSI,
		.miso_io_num = -1,
		.sclk_io_num = GPIO_SCLK,
		.quadwp_io_num = -1,
		.quadhd_io_num = -1,
		.max_transfer_sz = 0,
		.flags = 0
	};

	ret = spi_bus_initialize( LCD_SPI_HOST, &buscfg, SPI_DMA_CH_AUTO );
	ESP_LOGD(TAG, "spi_bus_initialize=%d",(int)ret);
	assert(ret==ESP_OK);

	spi_device_interface_config_t devcfg;
	memset(&devcfg, 0, sizeof(devcfg));
	devcfg.clock_speed_hz = clock_freq_hz;
	devcfg.queue_size = 7;
	devcfg.mode = 3;
	devcfg.flags = SPI_DEVICE_NO_DUMMY;

	if ( GPIO_CS >= 0 ) {
		devcfg.spics_io_num = GPIO_CS;
	} else {
		devcfg.spics_io_num = -1;
	}

	spi_device_handle_t handle;
	ret = spi_bus_add_device( LCD_SPI_HOST, &devcfg, &handle);
	ESP_LOGD(TAG, "spi_bus_add_device=%d",(int)ret);
	assert(ret==ESP_OK);
	dev->res = GPIO_RST;
	dev->dc = GPIO_DC;
	dev->bl = GPIO_BL;
	dev->SPIHandle = handle;
}

static bool spi_master_write_bytes(spi_device_handle_t SPIHandle, const uint8_t* Data, size_t DataLength)
{
	spi_transaction_t SPITransaction;
	esp_err_t ret;

	if ( DataLength > 0 ) {
		memset( &SPITransaction, 0, sizeof( spi_transaction_t ) );
		SPITransaction.length = DataLength * 8;
		SPITransaction.tx_buffer = Data;
#if 0
		ret = spi_device_transmit( SPIHandle, &SPITransaction );
#else
		ret = spi_device_polling_transmit( SPIHandle, &SPITransaction );
#endif
		assert(ret==ESP_OK);
	}

	return true;
}

static bool spi_master_write_command(TFT_t *dev, uint8_t cmd)
{
	static uint8_t Byte = 0;
	Byte = cmd;
	gpio_set_level( dev->dc, SPI_Command_Mode );
	return spi_master_write_bytes( dev->SPIHandle, &Byte, 1 );
}

static bool spi_master_write_data_byte(TFT_t *dev, uint8_t data)
{
	static uint8_t Byte = 0;
	Byte = data;
	gpio_set_level( dev->dc, SPI_Data_Mode );
	return spi_master_write_bytes( dev->SPIHandle, &Byte, 1 );
}

#if 0
static bool spi_master_write_data_word(TFT_t *dev, uint16_t data)
{
	static uint8_t Byte[2];
	Byte[0] = (data >> 8) & 0xFF;
	Byte[1] = data & 0xFF;
	gpio_set_level( dev->dc, SPI_Data_Mode );
	return spi_master_write_bytes( dev->SPIHandle, Byte, 2);
}
#endif

static bool spi_master_write_addr(TFT_t *dev, uint16_t addr1, uint16_t addr2)
{
	static uint8_t Byte[4];
	Byte[0] = (addr1 >> 8) & 0xFF;
	Byte[1] = addr1 & 0xFF;
	Byte[2] = (addr2 >> 8) & 0xFF;
	Byte[3] = addr2 & 0xFF;
	gpio_set_level( dev->dc, SPI_Data_Mode );
	return spi_master_write_bytes( dev->SPIHandle, Byte, 4);
}

// size is number of color elements, not bytes.
inline static bool spi_master_write_color(TFT_t *dev, color_t color, size_t size)
{
	uint16_t temp = SWAP16(color);
	size_t n = (size < BUF_LEN) ? size : BUF_LEN;
	for (size_t i = 0; i < n; i++) buffer[i] = temp;
	gpio_set_level(dev->dc, SPI_Data_Mode);
	while (size) {
		n = (size < BUF_LEN) ? size : BUF_LEN;
		spi_master_write_bytes(dev->SPIHandle, (uint8_t *)buffer, n*sizeof(uint16_t));
		size -= n;
	}
	return true;
}

// size is number of color elements, not bytes.
inline static bool spi_master_write_colors(TFT_t *dev, const color_t *colors, size_t size)
{
	gpio_set_level(dev->dc, SPI_Data_Mode);
	while (size) {
		size_t n = (size < BUF_LEN) ? size : BUF_LEN;
		for (size_t i = 0; i < n; i++) buffer[i] = SWAP16(colors[i]);
		spi_master_write_bytes(dev->SPIHandle, (uint8_t *)buffer, n*sizeof(uint16_t));
		colors += n;
		size -= n;
	}
	return true;
}


//----------------------------------------------------------------------------//
// LCD
//----------------------------------------------------------------------------//

void lcd_init(void)
{
	spi_master_init(dev,
		LCD_MOSI,
		LCD_SCLK,
		LCD_CS,
		LCD_DC,
		LCD_RST,
		LCD_BL);

	if (dev->res >= 0) {
		gpio_set_level(dev->res, 0);
		delayMS(20);
		gpio_set_level(dev->res, 1);
		delayMS(120);
	}

	dev->width = LCD_W;
	dev->height = LCD_H;
	dev->offsetx = LCD_OFFSETX;
	dev->offsety = LCD_OFFSETY;
	dev->font_direction = DIRECTION0;
	dev->font_size = 1;
	dev->font_back_en = false;
	dev->font_back_color = BLACK;
	dev->use_frame_buffer = false;
	dev->frame_buffer = NULL;

#if LCD_DRIVER == 0
	// spi_master_write_command(dev, 0x01);    // ILI:Software Reset (01h), ST:SWRESET (01h): Software Reset
	// delayMS(5);

	spi_master_write_command(dev, 0x3A);    // ILI:COLMOD: Pixel Format Set (3Ah), ST:COLMOD (3Ah): Interface Pixel Format
	spi_master_write_data_byte(dev, 0x55);
	// delayMS(10);

	spi_master_write_command(dev, 0x36);    // ILI:Memory Access Control (36h), ST:MADCTL (36h): Memory Data Access Control
	spi_master_write_data_byte(dev, 0x08);  // 0x00

	spi_master_write_command(dev, 0xCF);    // ILI:Power control B (CFh), ILI9341 only
	spi_master_write_data_byte(dev, 0x00);
	spi_master_write_data_byte(dev, 0xc3);
	spi_master_write_data_byte(dev, 0x30);
	spi_master_write_command(dev, 0xED);    // ILI:Power on sequence control (EDh), ILI9341 only
	spi_master_write_data_byte(dev, 0x64);
	spi_master_write_data_byte(dev, 0x03);
	spi_master_write_data_byte(dev, 0x12);
	spi_master_write_data_byte(dev, 0x81);
	spi_master_write_command(dev, 0xE8);    // ILI:Driver timing control A (E8h), ST:PWCTRL2 (E8h): Power Control 2
	spi_master_write_data_byte(dev, 0x85);
	spi_master_write_data_byte(dev, 0x00);
	spi_master_write_data_byte(dev, 0x78);
	spi_master_write_command(dev, 0xCB);    // ILI:Power control A (CBh), ILI9341 only
	spi_master_write_data_byte(dev, 0x39);
	spi_master_write_data_byte(dev, 0x2c);
	spi_master_write_data_byte(dev, 0x00);
	spi_master_write_data_byte(dev, 0x34);
	spi_master_write_data_byte(dev, 0x02);
	spi_master_write_command(dev, 0xF7);    // ILI:Pump ratio control (F7h), ILI9341 only
	spi_master_write_data_byte(dev, 0x20);
	spi_master_write_command(dev, 0xEA);    // ILI:Driver timing control B (EAh), ILI9341 only
	spi_master_write_data_byte(dev, 0x00);
	spi_master_write_data_byte(dev, 0x00);
	spi_master_write_command(dev, 0xC0);    // ILI:Power Control 1 (C0h), ST:LCMCTRL (C0h): LCM Control
	spi_master_write_data_byte(dev, 0x1B);
	spi_master_write_command(dev, 0xC1);    // ILI:Power Control 2 (C1h), ST:IDSET (C1h): ID Code Setting
	spi_master_write_data_byte(dev, 0x12);
	spi_master_write_command(dev, 0xC5);    // ILI:VCOM Control 1(C5h), ST:VCMOFSET (C5h): VCOM Offset Set
	spi_master_write_data_byte(dev, 0x32);
	spi_master_write_data_byte(dev, 0x3C);
	spi_master_write_command(dev, 0xC7);    // ILI:VCOM Control 2(C7h), ST:CABCCTRL (C7h): CABC Control
	spi_master_write_data_byte(dev, 0x91);
	spi_master_write_command(dev, 0xB1);    // ILI:Frame Rate Control (In Normal Mode/Full Colors) (B1h), ST:RGBCTRL (B1h): RGB Interface Control
	spi_master_write_data_byte(dev, 0x00);
	spi_master_write_data_byte(dev, 0x10);
	spi_master_write_command(dev, 0xB6);    // ILI:Display Function Control (B6h), ILI9341 only
	spi_master_write_data_byte(dev, 0x0A);
	spi_master_write_data_byte(dev, 0xA2);
	spi_master_write_command(dev, 0xF6);    // ILI:Interface Control (F6h), ILI9341 only
	spi_master_write_data_byte(dev, 0x01);
	spi_master_write_data_byte(dev, 0x30);

	spi_master_write_command(dev, 0x11);    // ILI:Sleep Out (11h), ST:SLPOUT (11h): Sleep Out
	delayMS(5);

#elif LCD_DRIVER == 1

	// spi_master_write_command(dev, 0x01);  // SWRESET (01h): Software Reset
	// delayMS(5);

	spi_master_write_command(dev, 0x36);  // MADCTL (36h): Memory Data Access Control
	spi_master_write_data_byte(dev, 0x00);

	spi_master_write_command(dev, 0x3A);  // COLMOD (3Ah): Interface Pixel Format
	spi_master_write_data_byte(dev, 0x05);

	spi_master_write_command(dev, 0xB2);  // PORCTRL (B2h): Porch Setting
	spi_master_write_data_byte(dev, 0x0C);
	spi_master_write_data_byte(dev, 0x0C);
	spi_master_write_data_byte(dev, 0x00);
	spi_master_write_data_byte(dev, 0x33);
	spi_master_write_data_byte(dev, 0x33);

	spi_master_write_command(dev, 0xB7);  // GCTRL (B7h): Gate Control
	spi_master_write_data_byte(dev, 0x35);

	spi_master_write_command(dev, 0xBB);  // VCOMS (BBh): VCOM Setting
	spi_master_write_data_byte(dev, 0x19);

	spi_master_write_command(dev, 0xC0);  // LCMCTRL (C0h): LCM Control
	spi_master_write_data_byte(dev, 0x2C);

	spi_master_write_command(dev, 0xC2);  // VDVVRHEN (C2h): VDV and VRH Command Enable
	spi_master_write_data_byte(dev, 0x01);

	spi_master_write_command(dev, 0xC3);  // VRHS (C3h): VRH Set
	spi_master_write_data_byte(dev, 0x12);

	spi_master_write_command(dev, 0xC4);  // VDVS (C4h): VDV Set
	spi_master_write_data_byte(dev, 0x20);

	spi_master_write_command(dev, 0xC6);  // FRCTRL2 (C6h): Frame Rate Control in Normal Mode
	spi_master_write_data_byte(dev, 0x0F);

	spi_master_write_command(dev, 0xD0);  // PWCTRL1 (D0h): Power Control 1
	spi_master_write_data_byte(dev, 0xA4);
	spi_master_write_data_byte(dev, 0xA1);

	spi_master_write_command(dev, 0xE0);  // PVGAMCTRL (E0h): Positive Voltage Gamma Control
	spi_master_write_data_byte(dev, 0xD0);
	spi_master_write_data_byte(dev, 0x04);
	spi_master_write_data_byte(dev, 0x0D);
	spi_master_write_data_byte(dev, 0x11);
	spi_master_write_data_byte(dev, 0x13);
	spi_master_write_data_byte(dev, 0x2B);
	spi_master_write_data_byte(dev, 0x3F);
	spi_master_write_data_byte(dev, 0x54);
	spi_master_write_data_byte(dev, 0x4C);
	spi_master_write_data_byte(dev, 0x18);
	spi_master_write_data_byte(dev, 0x0D);
	spi_master_write_data_byte(dev, 0x0B);
	spi_master_write_data_byte(dev, 0x1F);
	spi_master_write_data_byte(dev, 0x23);

	spi_master_write_command(dev, 0xE1);  // NVGAMCTRL (E1h): Negative Voltage Gamma Control
	spi_master_write_data_byte(dev, 0xD0);
	spi_master_write_data_byte(dev, 0x04);
	spi_master_write_data_byte(dev, 0x0C);
	spi_master_write_data_byte(dev, 0x11);
	spi_master_write_data_byte(dev, 0x13);
	spi_master_write_data_byte(dev, 0x2C);
	spi_master_write_data_byte(dev, 0x3F);
	spi_master_write_data_byte(dev, 0x44);
	spi_master_write_data_byte(dev, 0x51);
	spi_master_write_data_byte(dev, 0x2F);
	spi_master_write_data_byte(dev, 0x1F);
	spi_master_write_data_byte(dev, 0x1F);
	spi_master_write_data_byte(dev, 0x20);
	spi_master_write_data_byte(dev, 0x23);

	spi_master_write_command(dev, 0x11);  // SLPOUT (11h): Sleep Out
	delayMS(5);

#endif

#if LCD_INV
	ESP_LOGI(TAG, "Enable Display Inversion");
	lcd_inversionOn();
#else
	lcd_inversionOff();
#endif
	lcd_fillScreen(BLACK); // assume use_frame_buffer is false
	lcd_displayOn();
	lcd_backlightOn();
}

//----------------------------------------------------------------------------//
// Draw (outline) and fill primitives
//----------------------------------------------------------------------------//

void lcd_fillScreen(color_t color)
{
	if (dev->use_frame_buffer) {
		color_t *ptr = dev->frame_buffer;
		size_t len = (size_t)dev->width*dev->height;
		*ptr++ = color; len--;
		while (len) {
			size_t n = (len < ptr - dev->frame_buffer) ? len : ptr - dev->frame_buffer;
			memcpy(ptr, dev->frame_buffer, n*sizeof(color_t));
			ptr += n; len -= n;
		}
	} else {
		spi_master_write_command(dev, 0x2A); // Column(x) Address Set
		spi_master_write_addr(dev, 0, dev->width-1);
		spi_master_write_command(dev, 0x2B); // Page(y) Address Set
		spi_master_write_addr(dev, 0, dev->height-1);
		spi_master_write_command(dev, 0x2C); // Memory Write
		spi_master_write_color(dev, color, (size_t)dev->width*dev->height);
	}
}

void lcd_drawPixel(coord_t x, coord_t y, color_t color)
{
	if (x < 0 || x >= dev->width) return; // off screen
	if (y < 0 || y >= dev->height) return;

	if (dev->use_frame_buffer) {
		dev->frame_buffer[y*dev->width+x] = color;
	} else {
		coord_t _x = x + dev->offsetx;
		coord_t _y = y + dev->offsety;

		spi_master_write_command(dev, 0x2A); // Column(x) Address Set
		spi_master_write_addr(dev, _x, _x);
		spi_master_write_command(dev, 0x2B); // Page(y) Address Set
		spi_master_write_addr(dev, _y, _y);
		spi_master_write_command(dev, 0x2C); // Memory Write
		spi_master_write_colors(dev, &color, 1);
	}
}

void lcd_drawHPixels(coord_t x, coord_t y, coord_t w, const color_t *colors)
{
	if (x+w <= 0 || x >= dev->width) return; // off screen
	if (y < 0 || y >= dev->height) return;

	if (x < 0) {w += x; x = 0;} // clip
	if (x+w > dev->width) w = dev->width-x;

	if (dev->use_frame_buffer) {
		coord_t _x1 = x;
		coord_t _x2 = _x1 + (w-1);
		coord_t index = 0;
		size_t fbidx = (size_t)y*dev->width;
		for (coord_t i = _x1; i <= _x2; i++){
			dev->frame_buffer[fbidx+i] = colors[index++];
		}
	} else {
		coord_t _x1 = x + dev->offsetx;
		coord_t _x2 = _x1 + (w-1);
		coord_t _y1 = y + dev->offsety;
		coord_t _y2 = _y1;

		spi_master_write_command(dev, 0x2A); // Column(x) Address Set
		spi_master_write_addr(dev, _x1, _x2);
		spi_master_write_command(dev, 0x2B); // Page(y) Address Set
		spi_master_write_addr(dev, _y1, _y2);
		spi_master_write_command(dev, 0x2C); // Memory Write
		spi_master_write_colors(dev, colors, w);
	}
}

void lcd_drawHLine(coord_t x, coord_t y, coord_t w, color_t color)
{
	if (x+w <= 0 || x >= dev->width) return; // off screen
	if (y < 0 || y >= dev->height) return;

	if (x < 0) {w += x; x = 0;} // clip
	if (x+w > dev->width) w = dev->width-x;

	if (dev->use_frame_buffer) {
		coord_t _x1 = x;
		coord_t _x2 = _x1 + (w-1);
		size_t fbidx = (size_t)y*dev->width;
		for (coord_t i = _x1; i <= _x2; i++){
			dev->frame_buffer[fbidx+i] = color;
		}
	} else {
		coord_t _x1 = x + dev->offsetx;
		coord_t _x2 = _x1 + (w-1);
		coord_t _y1 = y + dev->offsety;
		coord_t _y2 = _y1;

		spi_master_write_command(dev, 0x2A); // Column(x) Address Set
		spi_master_write_addr(dev, _x1, _x2);
		spi_master_write_command(dev, 0x2B); // Page(y) Address Set
		spi_master_write_addr(dev, _y1, _y2);
		spi_master_write_command(dev, 0x2C); // Memory Write
		spi_master_write_color(dev, color, w);
	}
}

void lcd_drawVLine(coord_t x, coord_t y, coord_t h, color_t color)
{
	coord_t y2 = y+h-1;
	if (x < 0 || x  >= dev->width) return; // off screen
	if (y2 < 0 || y >= dev->height) return;

	if (y < 0) y = 0; // clip
	if (y2 >= dev->height) y2 = dev->height-1;

	if (dev->use_frame_buffer) {
		for (size_t j = y; j <= y2; j++){
			dev->frame_buffer[j*dev->width+x] = color;
		}
	} else {
		coord_t _x1 =  x  + dev->offsetx;
		coord_t _x2 = _x1 + dev->offsetx;
		coord_t _y1 =  y  + dev->offsety;
		coord_t _y2 =  y2 + dev->offsety;
		size_t size = _y2-_y1+1;

		spi_master_write_command(dev, 0x2A); // Column(x) Address Set
		spi_master_write_addr(dev, _x1, _x2);
		spi_master_write_command(dev, 0x2B); // Page(y) Address Set
		spi_master_write_addr(dev, _y1, _y2);
		spi_master_write_command(dev, 0x2C); // Memory Write
		spi_master_write_color(dev, color, size);
	}
}

/**
 * @note Bresenham's algorithm from Wikipedia. Speed enhanced by Bodmer to use
 *  efficient H/V Line draw routines for line segments of 2 pixels or more.
 */
void lcd_drawLine(coord_t x0, coord_t y0, coord_t x1, coord_t y1, color_t color)
{
	bool steep = abs(y1 - y0) > abs(x1 - x0);
	if (steep) {
		swap(coord_t, x0, y0);
		swap(coord_t, x1, y1);
	}

	if (x0 > x1) {
		swap(coord_t, x0, x1);
		swap(coord_t, y0, y1);
	}

	coord_t dx = x1 - x0, dy = abs(y1 - y0);;

	coord_t err = dx >> 1, ystep = -1, xs = x0, dlen = 0;

	if (y0 < y1) ystep = 1;

	// Split into steep and not steep for FastH/V separation
	if (steep) {
		for (; x0 <= x1; x0++) {
			dlen++;
			err -= dy;
			if (err < 0) {
				if (dlen == 1) lcd_drawPixel(y0, xs, color);
				else lcd_drawVLine(y0, xs, dlen, color);
				dlen = 0;
				y0 += ystep; xs = x0 + 1;
				err += dx;
			}
		}
		if (dlen) lcd_drawVLine(y0, xs, dlen, color);
	}
	else
	{
		for (; x0 <= x1; x0++) {
			dlen++;
			err -= dy;
			if (err < 0) {
				if (dlen == 1) lcd_drawPixel(xs, y0, color);
				else lcd_drawHLine(xs, y0, dlen, color);
				dlen = 0;
				y0 += ystep; xs = x0 + 1;
				err += dx;
			}
		}
		if (dlen) lcd_drawHLine(xs, y0, dlen, color);
	}
}

void lcd_drawRect(coord_t x, coord_t y, coord_t w, coord_t h, color_t color)
{
	lcd_drawHLine(x,     y,     w, color);
	lcd_drawHLine(x,     y+h-1, w, color);
	lcd_drawVLine(x,     y,     h, color);
	lcd_drawVLine(x+w-1, y,     h, color);
}

void lcd_fillRect(coord_t x, coord_t y, coord_t w, coord_t h, color_t color)
{
	coord_t x1 = x+w-1;
	coord_t y1 = y+h-1;

	if (x1 < 0 || x >= dev->width) return; // off screen
	if (y1 < 0 || y >= dev->height) return;

	if (x < 0) x = 0; // clip
	if (x1 >= dev->width) x1=dev->width-1;
	if (y < 0) y = 0;
	if (y1 >= dev->height) y1=dev->height-1;

	if (dev->use_frame_buffer) {
		for (size_t j = y; j <= y1; j++){
			for (size_t i = x; i <= x1; i++){
				dev->frame_buffer[j*dev->width+i] = color;
			}
		}
	} else {
		coord_t _x0 = x  + dev->offsetx;
		coord_t _x1 = x1 + dev->offsetx;
		coord_t _y0 = y  + dev->offsety;
		coord_t _y1 = y1 + dev->offsety;
		size_t size = (size_t)(_x1-_x0+1)*(_y1-_y0+1);

		spi_master_write_command(dev, 0x2A); // Column(x) Address Set
		spi_master_write_addr(dev, _x0, _x1);
		spi_master_write_command(dev, 0x2B); // Page(y) Address Set
		spi_master_write_addr(dev, _y0, _y1);
		spi_master_write_command(dev, 0x2C); // Memory Write
		spi_master_write_color(dev, color, size);
	}
}

void lcd_drawTriangle(coord_t x0, coord_t y0, coord_t x1, coord_t y1, coord_t x2, coord_t y2, color_t color)
{
	lcd_drawLine(x0, y0, x1, y1, color);
	lcd_drawLine(x1, y1, x2, y2, color);
	lcd_drawLine(x2, y2, x0, y0, color);
}

/**
 * @note Original Adafruit function works well and code footprint is small.
 */
void lcd_fillTriangle(coord_t x0, coord_t y0, coord_t x1, coord_t y1, coord_t x2, coord_t y2, color_t color)
{
	coord_t a, b, y, last;

	// Sort coordinates by Y order (y2 >= y1 >= y0)
	if (y0 > y1) {
		swap(coord_t, y0, y1); swap(coord_t, x0, x1);
	}
	if (y1 > y2) {
		swap(coord_t, y2, y1); swap(coord_t, x2, x1);
	}
	if (y0 > y1) {
		swap(coord_t, y0, y1); swap(coord_t, x0, x1);
	}

	if (y0 == y2) { // Handle awkward all-on-same-line case as its own thing
		a = b = x0;
		if (x1 < a)      a = x1;
		else if (x1 > b) b = x1;
		if (x2 < a)      a = x2;
		else if (x2 > b) b = x2;
		lcd_drawHLine(a, y0, b - a + 1, color);
		return;
	}

	coord_t
	dx01 = x1 - x0,
	dy01 = y1 - y0,
	dx02 = x2 - x0,
	dy02 = y2 - y0,
	dx12 = x2 - x1,
	dy12 = y2 - y1,
	sa   = 0,
	sb   = 0;

	// For upper part of triangle, find scanline crossings for segments
	// 0-1 and 0-2. If y1=y2 (flat-bottomed triangle), the scanline y1
	// is included here (and second loop will be skipped, avoiding a /0
	// error there), otherwise scanline y1 is skipped here and handled
	// in the second loop...which also avoids a /0 error here if y0=y1
	// (flat-topped triangle).
	if (y1 == y2) last = y1; // Include y1 scanline
	else          last = y1 - 1; // Skip it

	for (y = y0; y <= last; y++) {
		a   = x0 + sa / dy01;
		b   = x0 + sb / dy02;
		sa += dx01;
		sb += dx02;

		if (a > b) swap(coord_t, a, b);
		lcd_drawHLine(a, y, b - a + 1, color);
	}

	// For lower part of triangle, find scanline crossings for segments
	// 0-2 and 1-2. This loop is skipped if y1=y2.
	sa = dx12 * (y - y1);
	sb = dx02 * (y - y0);
	for (; y <= y2; y++) {
		a   = x1 + sa / dy12;
		b   = x0 + sb / dy02;
		sa += dx12;
		sb += dx02;

		if (a > b) swap(coord_t, a, b);
		lcd_drawHLine(a, y, b - a + 1, color);
	}
}

void lcd_drawCircle(coord_t xc, coord_t yc, coord_t r, color_t color)
{
	coord_t x;
	coord_t y;
	coord_t err;
	coord_t old_err;

	x=0;
	y=-r;
	err=2-2*r;
	do {
		lcd_drawPixel(xc-x, yc+y, color);
		lcd_drawPixel(xc-y, yc-x, color);
		lcd_drawPixel(xc+x, yc-y, color);
		lcd_drawPixel(xc+y, yc+x, color);
		if ((old_err=err)<=x)   err+=++x*2+1;
		if (old_err>y || err>x) err+=++y*2+1;
	} while (y<0);
}

void lcd_fillCircle(coord_t xc, coord_t yc, coord_t r, color_t color)
{
	coord_t x;
	coord_t y;
	coord_t err;
	coord_t old_err;
	coord_t ChangeX;

	x=0;
	y=-r;
	err=2-2*r;
	ChangeX=1;
	do {
		if (ChangeX) {
			lcd_drawVLine(xc-x, yc+y, (-y<<1)+1, color);
			lcd_drawVLine(xc+x, yc+y, (-y<<1)+1, color);
		}
		ChangeX=(old_err=err)<=x;
		if (ChangeX)            err+=++x*2+1;
		if (old_err>y || err>x) err+=++y*2+1;
	} while (y<=0);
}

void lcd_drawRoundRect(coord_t x, coord_t y, coord_t w, coord_t h, coord_t r, color_t color)
{
	coord_t x1 = x+w-1;
	coord_t y1 = y+h-1;
	coord_t xa;
	coord_t ya;
	coord_t err;
	coord_t old_err;

	w -= (r<<1);
	h -= (r<<1);
	if (w < 1 || h < 1) return;

	xa=0;
	ya=-r;
	err=2-2*r;

	do {
		if (xa) {
			lcd_drawPixel(x+r-xa,  y+r+ya,  color);
			lcd_drawPixel(x1-r+xa, y+r+ya,  color);
			lcd_drawPixel(x+r-xa,  y1-r-ya, color);
			lcd_drawPixel(x1-r+xa, y1-r-ya, color);
		}
		if ((old_err=err)<=xa)    err+=++xa*2+1;
		if (old_err>ya || err>xa) err+=++ya*2+1;
	} while (ya<0);
	lcd_drawHLine(x+r, y,   w, color);
	lcd_drawHLine(x+r, y1,  w, color);
	lcd_drawVLine(x,   y+r, h, color);
	lcd_drawVLine(x1,  y+r, h, color);
}

void lcd_fillRoundRect(coord_t x, coord_t y, coord_t w, coord_t h, coord_t r, color_t color)
{
	// coord_t x1 = x+w-1;
	coord_t y1 = y+h-1;
	coord_t xa;
	coord_t ya;
	coord_t err;
	coord_t old_err;

	coord_t w1 = w-(r<<1);
	coord_t h1 = h-(r<<1);
	if (w1 < 1 || h1 < 1) return;

	xa=0;
	ya=-r;
	err=2-2*r;

	do {
		if (xa) {
			lcd_drawHLine(x+r-xa, y +r+ya, w1+(xa<<1), color);
			lcd_drawHLine(x+r-xa, y1-r-ya, w1+(xa<<1), color);
		}
		if ((old_err=err)<=xa)    err+=++xa*2+1;
		if (old_err>ya || err>xa) err+=++ya*2+1;
	} while (ya<0);
	lcd_fillRect(x, y+r, w, h1, color);
}

/**
 * @details See this [link](http://k-hiura.cocolog-nifty.com/blog/2010/11/post-2a62.html)
    for implementation details.
 */
void lcd_drawArrow(coord_t x0, coord_t y0, coord_t x1, coord_t y1, coord_t w, color_t color)
{
	float Vx = x1 - x0; // basic vector
	float Vy = y1 - y0;
	float v  = sqrtf(Vx*Vx+Vy*Vy); // basic vector length
	float Ux = Vx/v; // basic unit vector
	float Uy = Vy/v;
	float h  = w*3; // arrow head height

	if (h > v) h = v; // clip arrow head height to vector length

	coord_t L[2],R[2],C[2]; // left, right, center arrow head base
	L[0] = x1 - Uy*w - Ux*h + 0.5f;
	L[1] = y1 + Ux*w - Uy*h + 0.5f;
	R[0] = x1 + Uy*w - Ux*h + 0.5f;
	R[1] = y1 - Ux*w - Uy*h + 0.5f;
	C[0] = x1 - Ux*h + 0.5f;
	C[1] = y1 - Uy*h + 0.5f;

	lcd_drawLine(x0, y0, C[0], C[1], color);
	lcd_drawTriangle(x1, y1, L[0], L[1], R[0], R[1], color);
}

/**
 * @details See this [link](http://k-hiura.cocolog-nifty.com/blog/2010/11/post-2a62.html)
    for implementation details.
 */
void lcd_fillArrow(coord_t x0, coord_t y0, coord_t x1, coord_t y1, coord_t w, color_t color)
{
	float Vx = x1 - x0; // basic vector
	float Vy = y1 - y0;
	float v  = sqrtf(Vx*Vx+Vy*Vy); // basic vector length
	float Ux = Vx/v; // basic unit vector
	float Uy = Vy/v;
	float h  = w*3; // arrow head height

	if (h > v) h = v; // clip arrow head height to vector length

	coord_t L[2],R[2],C[2]; // left, right, center arrow head base
	L[0] = x1 - Uy*w - Ux*h + 0.5f;
	L[1] = y1 + Ux*w - Uy*h + 0.5f;
	R[0] = x1 + Uy*w - Ux*h + 0.5f;
	R[1] = y1 - Ux*w - Uy*h + 0.5f;
	C[0] = x1 - Ux*h + 0.5f;
	C[1] = y1 - Uy*h + 0.5f;

	lcd_drawLine(x0, y0, C[0], C[1], color);
	lcd_fillTriangle(x1, y1, L[0], L[1], R[0], R[1], color);
}

void lcd_drawBitmap(coord_t x, coord_t y, const uint8_t *bitmap, coord_t w, coord_t h, color_t color)
{
	coord_t byteWidth = (w + 7) / 8; // pad bitmap scanline to whole byte
	uint8_t b = 0;

	if (x+w <= 0 || x >= dev->width) return; // off screen
	if (y+h <= 0 || y >= dev->height) return;

	for (size_t j = 0; j < h; j++, y++) {
		for (size_t i = 0; i < w; i++) {
			if (i & 7) b <<= 1;
			else b = bitmap[j * byteWidth + i / 8];
			if (b & 0x80) lcd_drawPixel(x + i, y, color);
		}
	}
}

void lcd_drawRGBBitmap(coord_t x, coord_t y, const color_t *bitmap, coord_t w, coord_t h)
{
	if (x+w <= 0 || x >= dev->width) return; // off screen
	if (y+h <= 0 || y >= dev->height) return;

	for (size_t j = 0; j < h; j++, y++) {
		lcd_drawHPixels(x, y, w, bitmap+j*w);
	}
}

//----------------------------------------------------------------------------//
// Rectangle variants that specify two diagonal corners
//----------------------------------------------------------------------------//

void lcd_drawRect2(coord_t x0, coord_t y0, coord_t x1, coord_t y1, color_t color)
{
	if (x0>x1) swap(coord_t, x0, x1);
	if (y0>y1) swap(coord_t, y0, y1);

	lcd_drawHLine(x0, y0, x1-x0+1, color);
	lcd_drawHLine(x0, y1, x1-x0+1, color);
	lcd_drawVLine(x1, y0, y1-y0+1, color);
	lcd_drawVLine(x0, y0, y1-y0+1, color);
}

void lcd_fillRect2(coord_t x0, coord_t y0, coord_t x1, coord_t y1, color_t color)
{
	if (x0>x1) swap(coord_t, x0, x1);
	if (y0>y1) swap(coord_t, y0, y1);

	if (x1 < 0 || x0 >= dev->width) return; // off screen
	if (y1 < 0 || y0 >= dev->height) return;

	if (x0 < 0) x0 = 0; // clip
	if (x1 >= dev->width) x1=dev->width-1;
	if (y0 < 0) y0 = 0;
	if (y1 >= dev->height) y1=dev->height-1;

	if (dev->use_frame_buffer) {
		for (size_t j = y0; j <= y1; j++){
			for (size_t i = x0; i <= x1; i++){
				dev->frame_buffer[j*dev->width+i] = color;
			}
		}
	} else {
		coord_t _x0 = x0 + dev->offsetx;
		coord_t _x1 = x1 + dev->offsetx;
		coord_t _y0 = y0 + dev->offsety;
		coord_t _y1 = y1 + dev->offsety;
		size_t size = (size_t)(_x1-_x0+1)*(_y1-_y0+1);

		spi_master_write_command(dev, 0x2A); // Column(x) Address Set
		spi_master_write_addr(dev, _x0, _x1);
		spi_master_write_command(dev, 0x2B); // Page(y) Address Set
		spi_master_write_addr(dev, _y0, _y1);
		spi_master_write_command(dev, 0x2C); // Memory Write
		spi_master_write_color(dev, color, size);
	}
}

void lcd_drawRoundRect2(coord_t x0, coord_t y0, coord_t x1, coord_t y1, coord_t r, color_t color)
{
	coord_t xa;
	coord_t ya;
	coord_t err;
	coord_t old_err;

	if (x0>x1) swap(coord_t, x0, x1);
	if (y0>y1) swap(coord_t, y0, y1);

	coord_t w = x1-x0+1-(r<<1);
	coord_t h = y1-y0+1-(r<<1);
	if (w < 1 || h < 1) return;

	xa=0;
	ya=-r;
	err=2-2*r;

	do {
		if (xa) {
			lcd_drawPixel(x0+r-xa, y0+r+ya, color);
			lcd_drawPixel(x1-r+xa, y0+r+ya, color);
			lcd_drawPixel(x0+r-xa, y1-r-ya, color);
			lcd_drawPixel(x1-r+xa, y1-r-ya, color);
		}
		if ((old_err=err)<=xa)    err+=++xa*2+1;
		if (old_err>ya || err>xa) err+=++ya*2+1;
	} while (ya<0);
	lcd_drawHLine(x0+r, y0,   w, color);
	lcd_drawHLine(x0+r, y1,   w, color);
	lcd_drawVLine(x0,   y0+r, h, color);
	lcd_drawVLine(x1,   y0+r, h, color);
}

void lcd_fillRoundRect2(coord_t x0, coord_t y0, coord_t x1, coord_t y1, coord_t r, color_t color)
{
	coord_t xa;
	coord_t ya;
	coord_t err;
	coord_t old_err;

	if (x0>x1) swap(coord_t, x0, x1);
	if (y0>y1) swap(coord_t, y0, y1);

	coord_t w1 = x1-x0+1-(r<<1);
	coord_t h1 = y1-y0+1-(r<<1);
	if (w1 < 1 || h1 < 1) return;

	xa=0;
	ya=-r;
	err=2-2*r;

	do {
		if (xa) {
			lcd_drawHLine(x0+r-xa, y0+r+ya, w1+(xa<<1), color);
			lcd_drawHLine(x0+r-xa, y1-r-ya, w1+(xa<<1), color);
		}
		if ((old_err=err)<=xa)    err+=++xa*2+1;
		if (old_err>ya || err>xa) err+=++ya*2+1;
	} while (ya<0);
	lcd_fillRect(x0, y0+r, x1-x0+1, h1, color);
}

//----------------------------------------------------------------------------//
// Specify center, size, and rotation angle of primitive shape
//----------------------------------------------------------------------------//

/**
 * @details A vertex's final position is calculated by rotating it
 *  around the center point of the primitive by the angle specified.
 * x1 = x * cos(angle) - y * sin(angle) + xc
 * y1 = x * sin(angle) + y * cos(angle) + yc
 */
void lcd_drawRectC(coord_t xc, coord_t yc, coord_t w, coord_t h, angle_t angle, color_t color)
{
	float xd, yd, rd;
	coord_t x1, y1;
	coord_t x2, y2;
	coord_t x3, y3;
	coord_t x4, y4;
	rd = -angle * M_PIf / 180.0f; // degrees to radians
	xd = 0.0f - w/2;
	yd = h/2;
	x1 = (coord_t)(xd * cosf(rd) - yd * sinf(rd) + xc);
	y1 = (coord_t)(xd * sinf(rd) + yd * cosf(rd) + yc);

	yd = 0.0f - yd;
	x2 = (coord_t)(xd * cosf(rd) - yd * sinf(rd) + xc);
	y2 = (coord_t)(xd * sinf(rd) + yd * cosf(rd) + yc);

	xd = w/2;
	yd = h/2;
	x3 = (coord_t)(xd * cosf(rd) - yd * sinf(rd) + xc);
	y3 = (coord_t)(xd * sinf(rd) + yd * cosf(rd) + yc);

	yd = 0.0f - yd;
	x4 = (coord_t)(xd * cosf(rd) - yd * sinf(rd) + xc);
	y4 = (coord_t)(xd * sinf(rd) + yd * cosf(rd) + yc);

	lcd_drawLine(x1, y1, x2, y2, color);
	lcd_drawLine(x1, y1, x3, y3, color);
	lcd_drawLine(x2, y2, x4, y4, color);
	lcd_drawLine(x3, y3, x4, y4, color);
}

/**
 * @details A vertex's final position is calculated by rotating it
 *  around the center point of the primitive by the angle specified.
 * x1 = x * cos(angle) - y * sin(angle) + xc
 * y1 = x * sin(angle) + y * cos(angle) + yc
 */
void lcd_drawTriangleC(coord_t xc, coord_t yc, coord_t w, coord_t h, angle_t angle, color_t color)
{
	float xd, yd, rd;
	coord_t x1, y1;
	coord_t x2, y2;
	coord_t x3, y3;
	rd = -angle * M_PIf / 180.0f; // degrees to radians
	xd = 0.0f;
	yd = h/2;
	x1 = (coord_t)(xd * cosf(rd) - yd * sinf(rd) + xc);
	y1 = (coord_t)(xd * sinf(rd) + yd * cosf(rd) + yc);

	xd = w/2;
	yd = 0.0f - yd;
	x2 = (coord_t)(xd * cosf(rd) - yd * sinf(rd) + xc);
	y2 = (coord_t)(xd * sinf(rd) + yd * cosf(rd) + yc);

	xd = 0.0f - w/2;
	x3 = (coord_t)(xd * cosf(rd) - yd * sinf(rd) + xc);
	y3 = (coord_t)(xd * sinf(rd) + yd * cosf(rd) + yc);

	lcd_drawLine(x1, y1, x2, y2, color);
	lcd_drawLine(x1, y1, x3, y3, color);
	lcd_drawLine(x2, y2, x3, y3, color);
}

/**
 * @details A vertex's final position is calculated by rotating it
 *  around the center point of the primitive by the angle specified.
 * x1 = x * cos(angle) - y * sin(angle) + xc
 * y1 = x * sin(angle) + y * cos(angle) + yc
 */
void lcd_drawRegularPolygonC(coord_t xc, coord_t yc, coord_t n, coord_t r, angle_t angle, color_t color)
{
	float xd, yd, rd;
	coord_t x1, y1;
	coord_t x2, y2;
	coord_t i;

	rd = -angle * M_PIf / 180.0f; // degrees to radians
	for (i = 0; i < n; i++) {
		xd = r * cosf(2 * M_PIf * i / n);
		yd = r * sinf(2 * M_PIf * i / n);
		x1 = (coord_t)(xd * cosf(rd) - yd * sinf(rd) + xc);
		y1 = (coord_t)(xd * sinf(rd) + yd * cosf(rd) + yc);

		xd = r * cosf(2 * M_PIf * (i + 1) / n);
		yd = r * sinf(2 * M_PIf * (i + 1) / n);
		x2 = (coord_t)(xd * cosf(rd) - yd * sinf(rd) + xc);
		y2 = (coord_t)(xd * sinf(rd) + yd * cosf(rd) + yc);

		lcd_drawLine(x1, y1, x2, y2, color);
	}
}

//----------------------------------------------------------------------------//
// Draw characters and strings
//----------------------------------------------------------------------------//

coord_t lcd_drawChar(coord_t x, coord_t y, char ascii, color_t color)
{
#if 0
	if ((x >= dev->width) ||                        // off screen right
		(y >= dev->height) ||                       // off screen bottom
		((x + LCD_CHAR_W * dev->font_size) <= 0) || // off screen left
		((y + LCD_CHAR_H * dev->font_size) <= 0))   // off screen top
		return;
#endif

	if (dev->font_back_en) {
		lcd_fillRect(x, y,
			LCD_CHAR_W*dev->font_size,
			LCD_CHAR_H*dev->font_size,
			dev->font_back_color);
	}
	for (int8_t i = 0; i < LCD_CHAR_W; i++) {
		uint8_t line;
		if (i == LCD_CHAR_W-1)
			line = 0x0;
		else
			line = font[((uint8_t)ascii * (LCD_CHAR_W-1)) + i];
		for (int8_t j = 0; j < LCD_CHAR_H; j++) {
			if (line & 0x1) {
				if (dev->font_size == 1) // default size
					lcd_drawPixel(x + i, y + j, color);
				else { // big size
					coord_t x1 = x + (i * dev->font_size), y1 = y + (j * dev->font_size);
					lcd_fillRect(x1, y1, dev->font_size, dev->font_size, color);
				}
			}
#if 0
			else if (dev->font_back_en) {
				if (dev->font_size == 1) // default size
					lcd_drawPixel(x + i, y + j, dev->font_back_color);
				else { // big size
					coord_t x1 = x + (i * dev->font_size), y1 = y + (j * dev->font_size);
					lcd_fillRect(x1, y1, dev->font_size, dev->font_size, dev->font_back_color);
				}
			}
#endif
			line >>= 1;
		}
	}
	return x+LCD_CHAR_W*dev->font_size;
}

coord_t lcd_drawString(coord_t x, coord_t y, const char *ascii, color_t color)
{
	size_t length = strlen(ascii);
	for (size_t i=0; i<length; i++) {
		x = lcd_drawChar(x, y, ascii[i], color);
	}
	return x;
}

//----------------------------------------------------------------------------//
// Font parameters
//----------------------------------------------------------------------------//

void lcd_setFontDirection(direction_t dir)
{
	// TODO: implement, currently direction always 0
	dev->font_direction = dir;
}

void lcd_setFontSize(uint8_t size)
{
	if (size < 1) return;
	dev->font_size = size;
}

void lcd_setFontBackground(color_t color)
{
	dev->font_back_en = true;
	dev->font_back_color = color;
}

void lcd_noFontBackground(void)
{
	dev->font_back_en = false;
}

//----------------------------------------------------------------------------//
// Display configuration
//----------------------------------------------------------------------------//

void lcd_spiClockFreq(int32_t freq)
{
	ESP_LOGI(TAG, "SPI clock frequency=%d MHz", (int)freq/1000000);
	clock_freq_hz = freq;
}

void lcd_displayOff(void)
{
	spi_master_write_command(dev, 0x28); // Display OFF (28h), DISPOFF (28h): Display Off
}

void lcd_displayOn(void)
{
	spi_master_write_command(dev, 0x29); // Display ON (29h), DISPON (29h): Display On
}

void lcd_backlightOff(void)
{
	if (dev->bl >= 0) {
		gpio_set_level(dev->bl, 0);
	}
}

void lcd_backlightOn(void)
{
	if (dev->bl >= 0) {
		gpio_set_level(dev->bl, 1);
	}
}

void lcd_inversionOff(void)
{
	spi_master_write_command(dev, 0x20); // Display Inversion OFF (20h), INVOFF (20h): Display Inversion Off
}

void lcd_inversionOn(void)
{
	spi_master_write_command(dev, 0x21); // Display Inversion ON (21h), INVON (21h): Display Inversion On
}

//----------------------------------------------------------------------------//
// Frame management
//----------------------------------------------------------------------------//

void lcd_frameEnable(void)
{
	if (dev->use_frame_buffer == true) return;
	dev->frame_buffer = heap_caps_malloc(sizeof(color_t)*dev->width*dev->height, MALLOC_CAP_DMA);
	if (dev->frame_buffer == NULL) {
		ESP_LOGE(TAG, "frame buffer alloc fail");
	} else {
		ESP_LOGI(TAG, "frame buffer alloc success");
		dev->use_frame_buffer = true;
	}
}

void lcd_frameDisable(void)
{
	if (dev->frame_buffer != NULL) heap_caps_free(dev->frame_buffer);
	dev->frame_buffer = NULL;
	dev->use_frame_buffer = false;
}

color_t *lcd_getFrameBuffer(void)
{
	return dev->frame_buffer;
}

void lcd_wrapAround(scroll_t scroll, coord_t start, coord_t end)
{
	if (dev->use_frame_buffer == false) return;

	coord_t fb_w = dev->width;
	coord_t fb_h = dev->height;
	size_t index1;
	size_t index2;

	switch (scroll) {
	case SCROLL_RIGHT: {
		color_t wk[fb_w];
		for (size_t i=start;i<=end;i++) {
			index1 = i * fb_w;
			memcpy((char *)wk, (char*)&dev->frame_buffer[index1], fb_w*sizeof(color_t));
			index2 = index1 + fb_w - 1;
			dev->frame_buffer[index1] = dev->frame_buffer[index2];
			memcpy((char *)&dev->frame_buffer[index1+1], (char *)&wk[0], (fb_w-1)*sizeof(color_t));
		}
		break; }
	case SCROLL_LEFT: {
		color_t wk[fb_w];
		for (size_t i=start;i<=end;i++) {
			index1 = i * fb_w;
			memcpy((char *)wk, (char*)&dev->frame_buffer[index1], fb_w*sizeof(color_t));
			index2 = index1 + fb_w - 1;
			dev->frame_buffer[index2] = dev->frame_buffer[index1];
			memcpy((char *)&dev->frame_buffer[index1], (char *)&wk[1], (fb_w-1)*sizeof(color_t));
		}
		break; }
	case SCROLL_DOWN: {
		color_t wk;
		for (size_t i=start;i<=end;i++) {
			index2 = (size_t)(fb_h-1) * fb_w + i;
			wk = dev->frame_buffer[index2];
			for (ssize_t j=fb_h-2;j>=0;j--) {
				index1 = j * fb_w + i;
				index2 = (j+1) * fb_w + i;
				dev->frame_buffer[index2] = dev->frame_buffer[index1];
			}
			dev->frame_buffer[i] = wk;
		}
		break; }
	case SCROLL_UP: {
		color_t wk;
		for (size_t i=start;i<=end;i++) {
			wk = dev->frame_buffer[i];
			for (size_t j=0;j<fb_h-1;j++) {
				index1 = j * fb_w + i;
				index2 = (j+1) * fb_w + i;
				dev->frame_buffer[index1] = dev->frame_buffer[index2];
			}
			index2 = (size_t)(fb_h-1) * fb_w + i;
			dev->frame_buffer[index2] = wk;
		}
		break; }
	}
}

void lcd_writeFrame(void)
{
	if (dev->use_frame_buffer == false) return;

	spi_master_write_command(dev, 0x2A); // Column(x) Address Set
	spi_master_write_addr(dev, dev->offsetx, dev->offsetx+dev->width-1);
	spi_master_write_command(dev, 0x2B); // Page(y) Address Set
	spi_master_write_addr(dev, dev->offsety, dev->offsety+dev->height-1);
	spi_master_write_command(dev, 0x2C); // Memory Write
	spi_master_write_colors(dev, dev->frame_buffer, dev->width*dev->height);

#if 0
	size_t size = (size_t)dev->width*dev->height;
	color_t *image = dev->frame_buffer;
	while (size > 0) {
		// 1024 bytes per time.
		size_t bs = (size > 512) ? 512 : size;
		spi_master_write_colors(dev, image, bs);
		size -= bs;
		image += bs;
	}
#endif
	return;
}
