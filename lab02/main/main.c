#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/gpio.h" // gpio_dump_io_configuration
#include "driver/rtc_io.h" // rtc_gpio_is_valid_gpio

#include "hw.h"
#include "lcd.h"
#include "pin.h"
#include "pin_test.h"

static const char *TAG = "lab02";

// #define USE_GPIO 1
// #define ERRORS 1

#define PIN_MAX  40
#define PIN_COLS 8
#define PIN_ROWS 3
#define PIN_W    (LCD_W/PIN_COLS)
#define PIN_H    (LCD_H/PIN_ROWS)
#define PIN_R    (PIN_W/4)

#define LGAP 2 // label gap

#define EXT_Y (PIN_H*0)
#define SND_Y (PIN_H*1)
#define BTN_Y (PIN_H*2)

#define IN_CLR GREEN
#define OUT_CLR rgb565(255,127,255)
#define ERR1_CLR rgb565(255,0,0) // show red if mismatch
#define ERR0_CLR rgb565(127,0,0)
#define GRP_CLR CYAN
#define LABEL_CLR YELLOW
#define PIN_CLR WHITE

#define SET_BIT(v,b) (v) |= (1U << (b))
#define CLR_BIT(v,b) (v) &= ~(1U << (b))
#define GET_BIT(v,b) ((int32_t)(((v) >> (b)) & 1U))

typedef struct {
	const char *label;
	pin_num_t pin;
	bool in;
	bool out;
} pin_conf_t;

const pin_conf_t external[] = {
	{    "EX8", HW_EX8,        false,  true },
	{    "EX7", HW_EX7,        false,  true },
	{    "EX6", HW_EX6,         true, false },
	{    "EX5", HW_EX5,         true, false },
	{    "EX4", HW_EX4,         true, false },
};
const pin_conf_t sound[] = {
	{     "SD", HW_SND_EN,      false, true },
	{     "A+", HW_SND_A,       false, true },
};
const pin_conf_t buttons[] = {
	{   "Menu", HW_BTN_MENU,    true, false },
	{ "Option", HW_BTN_OPTION,  true, false },
	{       "", -1,            false, false },
	{ "Select", HW_BTN_SELECT,  true, false },
	{  "Start", HW_BTN_START,   true, false },
	{       "", -1,            false, false },
	{      "B", HW_BTN_B,       true, false },
	{      "A", HW_BTN_A,       true, false },
};

typedef struct {
	coord_t x;
	coord_t y;
	const char *label;
	const pin_conf_t *ptr;
	pin_num_t len;
} group_t;

const group_t grp_list[] = {
	{0, PIN_H*0, "External I/O", external, sizeof(external)/sizeof(pin_conf_t)},
	// {0, PIN_H*1, "Sound",        sound,    sizeof(sound)/sizeof(pin_conf_t)},
	{0, PIN_H*2, "Buttons",      buttons,  sizeof(buttons)/sizeof(pin_conf_t)},
};

bool pin_state[PIN_MAX];


// Initialize I/O pins in a configuration group.
void init_grp(const pin_conf_t *grp, pin_num_t len)
{
	for (pin_num_t i = 0; i < len; i++) { // each pin in group
		pin_num_t pin = grp[i].pin;
		if (pin < 0) continue;
		bool out = grp[i].out;
#ifdef USE_GPIO
		gpio_reset_pin(pin);
		gpio_set_pull_mode(pin, out ? GPIO_FLOATING:GPIO_PULLUP_ONLY);
		// if output, set as input in addition so it can be read
		gpio_set_direction(pin, out ? GPIO_MODE_INPUT_OUTPUT:GPIO_MODE_INPUT);
		if (out) gpio_set_level(pin, 0);
#else
		pin_reset(pin);
		pin_pullup(pin, !out);
		// if output, set as input in addition so it can be read
		pin_input(pin, true);
		if (out) {
			pin_output(pin, true);
			pin_set_level(pin, 0);
		}
#endif /* USE_GPIO */
		// gpio_dump_io_configuration(stdout, 1LLU << pin);
		// printf("rtc_gpio(%02hhd)  : %d\n", pin, rtc_gpio_is_valid_gpio(pin));
	}
}

// Initialize all I/O pins in each configuration group
void init_all(void)
{
	for (int32_t i = 0; i < sizeof(grp_list)/sizeof(group_t); i++) {
		const group_t *g = grp_list + i;
		init_grp(g->ptr, g->len);
	}
}

// Test I/O pins in a configuration group.
void test_grp(const char *label, const pin_conf_t *grp, pin_num_t len)
{
	int32_t cps; // current pin state
	int32_t eps; // expected pin state
	uint64_t crs; // current reg state
	uint32_t errors; // error count

	// Test registers for proper initialization values.
	errors = 0;
	for (pin_num_t i = 0; i < len; i++) { // each pin in group
		pin_num_t pin = grp[i].pin;
		if (pin < 0) continue;
		bool out = grp[i].out;

		cps = pin_test_get_pin_reg(pin);
		eps = 0; // only non-zero if open drain, no open drain
#ifdef ERRORS
if (pin == HW_BTN_SELECT) cps |= 1;
#endif
		if (eps != cps) {
			ESP_LOGE(TAG, "IO%02hhd pin reg exp:0x%X cur:0x%X",
				pin, (int)eps, (int)cps);
			errors++;
		}

		cps = pin_test_get_func_out_sel_cfg_reg(pin);
		eps = 0x100; // GPIO_FUNCn_OUT_SEL=0x100
#ifdef ERRORS
if (pin == HW_EX8) cps |= 1;
#endif
		if (eps != cps) {
			ESP_LOGE(TAG, "IO%02hhd function out sel reg exp:0x%X cur:0x%X",
				pin, (int)eps, (int)cps);
			errors++;
		}

		cps = pin_test_get_io_mux_reg(pin);
		eps = 0x2900; // MCU_SEL=2, FUN_DRV=2, FUN_WPU=1
		if (!rtc_gpio_is_valid_gpio(pin)) {
			if (!out) SET_BIT(eps, 8); else CLR_BIT(eps, 8); // pull-up
			// no pull-down
		}
		SET_BIT(eps, 9); // all pins initialized as inputs
#ifdef ERRORS
if (pin == HW_BTN_MENU) cps |= 1;
#endif
#ifdef USE_GPIO
		cps &= ~0x0180; eps &= ~0x0180; // gpio can leave random pull-up/down
#endif
		if (eps != cps) {
			ESP_LOGE(TAG, "IO%02hhd IO MUX reg exp:0x%X cur:0x%X",
				pin, (int)eps, (int)cps);
			errors++;
		}
	}
	if (!errors) {
		ESP_LOGI(TAG, "PASS: %s configuration test", label);
	}

	// Test for consistency between pin_get_level() & pin_get_in_reg()
	// and between pin_set_level() & pin_get_out_reg().
	errors = 0;
	for (uint8_t j = 0; j < 3; j++) {
		uint32_t lvl = j & 1; // level: 0, 1, 0
		for (pin_num_t i = 0; i < len; i++) { // each pin in group
			pin_num_t pin = grp[i].pin;
			if (pin < 0) continue;
			if (grp[i].in) { // input pin
				cps = pin_get_level(pin);
				crs = pin_get_in_reg();
				eps = GET_BIT(crs, pin);
#ifdef ERRORS
if (pin == HW_BTN_SELECT) cps = !cps;
#endif
				if (eps != cps) {
					ESP_LOGE(TAG, "IO%02hhd in reg:%010llX exp:%d cur:%d",
						pin, crs, (int)eps, (int)cps);
					errors++;
				}
			}
			if (grp[i].out) { // output pin
				pin_set_level(pin, lvl);
				crs = pin_get_out_reg();
				cps = GET_BIT(crs, pin);
#ifdef ERRORS
if (pin == HW_EX7) cps = !cps;
#endif
				if (lvl != cps) {
					ESP_LOGE(TAG, "IO%02hhd out reg:%010llX exp:%d cur:%d",
						pin, crs, (int)lvl, (int)cps);
					errors++;
				}
			}
		}
	}
	if (!errors) {
		ESP_LOGI(TAG, "PASS: %s consistency test", label);
	}
}

// Test all I/O pins in each configuration group
void test_all(void)
{
	for (int32_t i = 0; i < sizeof(grp_list)/sizeof(group_t); i++) {
		const group_t *g = grp_list + i;
		test_grp(g->label, g->ptr, g->len);
	}
}

// Draw a group of I/O pins in a row.
void draw_grp(coord_t xa, coord_t ya, const pin_conf_t *grp, pin_num_t len, bool init)
{
	coord_t yc = ya+PIN_H/2;
	coord_t y1 = yc - (PIN_R+1+LGAP) - (LCD_CHAR_H-1);
	coord_t y2 = yc + (PIN_R+1+LGAP);
	for (pin_num_t i = 0; i < len; i++) { // each pin in group
		if (grp[i].pin < 0) continue;
		coord_t xc = xa+i*PIN_W+PIN_W/2;
		coord_t x1 = xc-(LCD_CHAR_W*strlen(grp[i].label))/2;
		coord_t x2 = xc-(LCD_CHAR_W*2);
		pin_num_t pin = grp[i].pin;
		color_t color1 = grp[i].out ? OUT_CLR : IN_CLR;
		color_t color0 = BLACK;
		if (init) {
			lcd_drawCircle(xc, yc, PIN_R, color1);
			lcd_drawString(x1, y1, grp[i].label, LABEL_CLR);
			char str[8];
			sprintf(str, "IO%02hhd", pin);
			lcd_drawString(x2, y2, str, PIN_CLR);
		}
		int32_t cps = pin_get_level(pin); // current pin state
		if (init || cps != pin_state[pin]) {
			// use gpio as reference
			int32_t eps = gpio_get_level(pin); // expected pin state
			uint64_t crs = pin_get_in_reg(); // current reg state
			if (!init) ESP_LOGI(TAG, "IO%02u=%u", (unsigned)pin, (unsigned)cps);
#ifdef ERRORS
if (pin == HW_BTN_B) eps = !eps;
#endif
			// if current pin state does not match expected pin state
			if (eps != cps) {
				ESP_LOGE(TAG, "IO%02hhd in reg:%010llX exp:%d cur:%d",
					pin, crs, (int)eps, (int)cps);
				color1 = ERR1_CLR; // mismatch
				color0 = ERR0_CLR;
			}
			lcd_fillCircle(xc, yc, PIN_R-1, cps ? color1 : color0);
			pin_state[pin] = cps;
		}
	}
}

// Draw a panel indicating the state of the I/O pins on the display.
void draw_all(bool init)
{
	for (int32_t i = 0; i < sizeof(grp_list)/sizeof(group_t); i++) {
		const group_t *g = grp_list + i;
		lcd_drawString(g->x, g->y, g->label, GRP_CLR);
		draw_grp(g->x, g->y, g->ptr, g->len, init);
	}
}

// Main application.
void app_main(void)
{

	ESP_LOGI(TAG, "Start up");
	lcd_init();
	lcd_fillScreen(BLACK);

	init_all();
	test_all();
	draw_all(true);
	for (;;) {
		draw_all(false);
		vTaskDelay(1);
	}
}
