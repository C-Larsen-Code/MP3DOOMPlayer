// https://mixbutton.com/mixing-articles/music-note-to-frequency-chart/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_check.h"

#include "hw.h"
#include "lcd.h"
#include "pin.h"
#include "joy.h"

static const char *TAG = "lab04";

#define UP_PER 30 // Update period in ms
#define TIME_OUT 500 // ms

#define PIN_GET_BIT(r,b) (((r) >> (b)) & 1)

// Cursor support
#define CUR_SZ 7 // Cursor size
#define CUR_CL WHITE // Cursor color
#define STB_CL GREEN // Status bar color
#define SBG_CL BLACK // Screen background color

#define OV 30 // Over scale by a margin
#define A2X(d) (((d)*(LCD_W/2+OV))/JOY_MAX_DISP+(LCD_W/2))
#define A2Y(d) (((d)*(LCD_H/2+OV))/JOY_MAX_DISP+(LCD_H/2))
#define CLIP(x,lo,hi) ((x) < (lo) ? (lo) : ((x) > (hi) ? (hi) : (x)))

// Sound support
#if MILESTONE == 2
#include "tone.h"
#include "powerUp.h"
#include "userSound.h"
#else
// If not Milestone 2, allow tone code to compile (do nothing)
typedef enum {SINE_T, SQUARE_T, TRIANGLE_T, SAW_T, LAST_T} tone_t;
#define MAX_VOL 100U
#define tone_init(hz)
#define tone_stop()
#define tone_set_volume(vol)
#define tone_start(tone,freq)
#define sound_start(audio,size,wait)
#endif // MILESTONE

#define VOL_INC 20 // %
#define SAMPLE_RATE POWERUP_SAMPLE_RATE // Hz
#define VOLUME_DEFAULT (VOL_INC*3) // %

// Musical note frequencies
#define A3 220
#define A4 440
#define A5 880

// Wave configuration
#define WAVE_X 0
#define WAVE_Y ((LCD_H-WAVE_H)/2)
#define WAVE_W LCD_W
#define WAVE_H (LCD_H*3/4)
#define WAVE_CL GREEN
#define WAVE_YC (WAVE_Y+WAVE_H/2-1)
#define WAVE_MARK_H 15
#define WAVE_MARK_CL rgb565(96, 96, 96)
// #define WAVE_MARK_CL WHITE

TimerHandle_t update_timer;
coord_t dcx, dcy;
uint32_t vol;
tone_t tone;

// Draw the cursor on the display
void cursor(coord_t x, coord_t y, color_t color)
{
	coord_t s2 = CUR_SZ >> 1; // size div 2
	lcd_drawHLine(x-s2, y,    CUR_SZ, color);
	lcd_drawVLine(x,    y-s2, CUR_SZ, color);
}

// Draw the joystick status bar at the top left of the display
void draw_joystick_status(void)
{
	char str[16];
	sprintf(str, "x:%5ld y:%5ld", dcx, dcy);
	lcd_drawString(0, 0, str, STB_CL);
}

// Draw the tone status bar at the top right of the display
void draw_tone_status(void)
{
#if MILESTONE == 2
	char str[12];
	static char *ttab[] = {"sin", "squ", "tri", "saw"};
	sprintf(str, "t:%s v:%3lu", ttab[tone], vol);
	lcd_drawString(LCD_W-(strlen(str)*LCD_CHAR_W), 0, str, STB_CL);
#endif // MILESTONE
}

// Draw the current waveform in the center of the display
void draw_waveform(void)
{
#if MILESTONE == 2
	extern const uint8_t *abase;
	extern volatile uint32_t asize;
	#define WAVE_MAX ((1 << (sizeof(abase[0])*8))-1) // Maximum value
	float scalex = (float)(WAVE_W/2)/(asize);
	float scaley = (float)(WAVE_H-1)/WAVE_MAX;
	#define W2X(val) ((coord_t)((val) * scalex + 0.5f) + WAVE_X)
	#define W2Y(val) ((WAVE_Y+WAVE_H-1) - (coord_t)((val) * scaley + 0.5f))

	coord_t x0 = W2X(-1);
	coord_t y0 = W2Y(abase[asize-1]);
	coord_t x2 = (coord_t)(asize * scalex + 0.5f);
	lcd_drawHLine(WAVE_X, WAVE_Y, WAVE_W, WAVE_MARK_CL);
	lcd_drawHLine(WAVE_X, WAVE_Y+WAVE_H-1, WAVE_W, WAVE_MARK_CL);
	lcd_drawVLine(WAVE_X, WAVE_YC-WAVE_MARK_H/2, WAVE_MARK_H, WAVE_MARK_CL);
	lcd_drawVLine(WAVE_X+x2, WAVE_YC-WAVE_MARK_H/2, WAVE_MARK_H, WAVE_MARK_CL);
	for (uint32_t i = 0; i < asize; i++) {
		coord_t x1 = W2X(i);
		coord_t y1 = W2Y(abase[i]);
		lcd_drawPixel(x1, WAVE_YC, WAVE_MARK_CL);
		lcd_drawPixel(x1+x2, WAVE_YC, WAVE_MARK_CL);
		lcd_drawLine(x0, y0, x1, y1, WAVE_CL);
		lcd_drawLine(x0+x2, y0, x1+x2, y1, WAVE_CL);
		x0 = x1; y0 = y1;
	}
#endif // MILESTONE
}

// Update the position of the cursor and play sound if button A or B is pressed
void update(TimerHandle_t pxTimer)
{
	static coord_t lx = -1, ly = -1;
	coord_t x, y;
	static bool pressed = false;
	uint64_t btns;

	joy_get_displacement(&dcx, &dcy);
	btns = ~pin_get_in_reg() & HW_BTN_MASK;
	if (!pressed && btns) { // On button press
		pressed = true;
		if (PIN_GET_BIT(btns, HW_BTN_A)) { // Play tone
			tone_start(tone, (dcy > 0) ?
				A4-(dcy*A3)/JOY_MAX_DISP :
				A4-(dcy*A4)/JOY_MAX_DISP);
			cursor(lx, ly, SBG_CL); // Erase cursor
			draw_waveform();
		} else if (PIN_GET_BIT(btns, HW_BTN_B)) { // Play user sound
			sound_start(userSound, sizeof(userSound), false);
		} else if (PIN_GET_BIT(btns, HW_BTN_MENU)) { // Select next tone
			tone = (tone+1)%LAST_T;
		} else if (PIN_GET_BIT(btns, HW_BTN_OPTION)) { // Select next volume level
			vol = (vol <= MAX_VOL-VOL_INC) ? vol+VOL_INC : 0;
			tone_set_volume(vol);
		}
		draw_tone_status();
	} else if (pressed && !btns) { // On button release, stop playing sound
		tone_stop();
		pressed = false;
		lcd_fillRect(WAVE_X, WAVE_Y, WAVE_W, WAVE_H, SBG_CL); // Erase waveform
		cursor(lx, ly, CUR_CL); // Redraw cursor
	}
	// Convert from joystick displacement to screen coordinates
	x = A2X(dcx); y = A2Y(dcy);
	x = CLIP(x, 0, LCD_W-1);
	y = CLIP(y, 0, LCD_H-1);
	if (x != lx || y != ly) { // Update cursor position
		if (!pressed || !PIN_GET_BIT(btns, HW_BTN_A)) {
			cursor( lx,  ly, SBG_CL);
			cursor(x, y, CUR_CL);
		}
		lx = x; ly = y;
		draw_joystick_status();
		if (y < LCD_CHAR_H && x > LCD_W/2)
			draw_tone_status(); // Redraw if cursor in tone status area
	}
}

// Main application
void app_main(void)
{
	ESP_LOGI(TAG, "Start up");

	// Configure I/O pins for buttons
	pin_reset(HW_BTN_A);
	pin_input(HW_BTN_A, true);
	pin_reset(HW_BTN_B);
	pin_input(HW_BTN_B, true);
	pin_reset(HW_BTN_MENU);
	pin_input(HW_BTN_MENU, true);
	pin_reset(HW_BTN_OPTION);
	pin_input(HW_BTN_OPTION, true);
	pin_reset(HW_BTN_SELECT);
	pin_input(HW_BTN_SELECT, true);
	pin_reset(HW_BTN_START);
	pin_input(HW_BTN_START, true);

	lcd_init(); // Initialize LCD display and device handle
	lcd_fillScreen(SBG_CL); // Clear the screen
	lcd_setFontBackground(SBG_CL); // Set font background

	vol = VOLUME_DEFAULT;
	tone = SINE_T;
	tone_init(SAMPLE_RATE); // Initialize tone and sample rate
	tone_set_volume(vol); // Set the volume
	draw_tone_status();

	joy_init(); // Initialize joystick driver
	draw_joystick_status();

	// Schedule the tick() function to run at the specified period.
	// The joy_get_displacement() function is not safe to call from an
	// ISR context. Therefore, the update() function must be called from
	// a software task context provided by the RTOS.
	update_timer = xTimerCreate(
		"update_timer",        // Text name for the timer.
		pdMS_TO_TICKS(UP_PER), // The timer period in ticks.
		pdTRUE,                // Auto-reload the timer when it expires.
		NULL,                  // No need for a timer ID.
		update                 // Tick function (callback) when timer expires.
	);
	if (update_timer == NULL) {
		ESP_LOGE(TAG, "Error creating timer");
		return;
	}
	if (xTimerStart(update_timer, pdMS_TO_TICKS(TIME_OUT)) != pdPASS) {
		ESP_LOGE(TAG, "Error starting timer");
		return;
	}

	sound_start(powerUp, sizeof(powerUp), true); // Play power-up sound
	for (;;) vTaskDelay(pdMS_TO_TICKS(1000));
}
