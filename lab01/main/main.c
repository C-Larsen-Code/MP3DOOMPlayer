#include <stdio.h>
#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "lcd.h"
#include "pac.h"

static const char *TAG = "lab01";

#define DELAY_MS(ms) \
	vTaskDelay(((ms)+(portTICK_PERIOD_MS-1))/portTICK_PERIOD_MS)

//----------------------------------------------------------------------------//
// Car Implementation - Begin
//----------------------------------------------------------------------------//

// Car constants
#define CAR_CLR rgb565(220,30,0)
#define WINDOW_CLR rgb565(180,210,238)
#define TIRE_CLR BLACK
#define HUB_CLR GRAY

#define CAR_W 60
#define CAR_H 32

#define BODY_X0 0
#define BODY_Y0 12
#define BODY_X1 59
#define BODY_Y1 24

#define CABIN_X0 1
#define CABIN_Y0 0
#define CABIN_X1 39
#define CABIN_Y1 11

#define LEFT_WINDOW_X0 3
#define LEFT_WINDOW_Y0 1
#define LEFT_WINDOW_X1 18
#define LEFT_WINDOW_Y1 8
#define LEFT_WINDOW_R 2

#define RIGHT_WINDOW_X0 21
#define RIGHT_WINDOW_Y0 1
#define RIGHT_WINDOW_X1 37
#define RIGHT_WINDOW_Y1 8
#define RIGHT_WINDOW_R 2

#define HOOD_TOP_X 40
#define HOOD_TOP_Y 9
#define HOOD_BTM_X 40
#define HOOD_BTM_Y 11
#define HOOD_RGHT_X 59
#define HOOD_RGHT_Y 11

#define TIRE_L_X 11
#define TIRE_L_Y 24
#define TIRE_R_X 48
#define TIRE_R_Y 24
#define TIRE_INNER_R 4
#define TIRE_OUTER_R 7



// TODO: Finish car part constants

/**
 * @brief Draw a car at the specified location.
 * @param x      Top left corner X coordinate.
 * @param y      Top left corner Y coordinate.
 * @details Draw the car components relative to the anchor point (top, left).
 */
void drawCar(coord_t x, coord_t y)
{
	// TODO: Implement car procedurally with lcd geometric primitives.

	lcd_fillRect2(CABIN_X0+x, CABIN_Y0+y, CABIN_X1+x, CABIN_Y1+y, CAR_CLR); // Top Red Rectangle of Car
	lcd_fillRect2(BODY_X0+x, BODY_Y0+y, BODY_X1+x, BODY_Y1+y, CAR_CLR); // Bottom Red Rectangle of Car
	lcd_fillRoundRect2(LEFT_WINDOW_X0+x, LEFT_WINDOW_Y0+y, LEFT_WINDOW_X1+x, LEFT_WINDOW_Y1+y, LEFT_WINDOW_R, WINDOW_CLR); //Left Window of Car
	lcd_fillRoundRect2(RIGHT_WINDOW_X0+x, RIGHT_WINDOW_Y0+y, RIGHT_WINDOW_X1+x, RIGHT_WINDOW_Y1+y, RIGHT_WINDOW_R, WINDOW_CLR); //Right Window of Car
	lcd_fillTriangle(HOOD_TOP_X+x, HOOD_TOP_Y+y, HOOD_BTM_X+x, HOOD_BTM_Y+y, HOOD_RGHT_X+x, HOOD_RGHT_Y+y, CAR_CLR); //Hood of Car
	lcd_fillCircle(TIRE_L_X+x, TIRE_L_Y+y, TIRE_OUTER_R, TIRE_CLR); //Left Tire Rubber of Car
	lcd_fillCircle(TIRE_R_X+x, TIRE_R_Y+y, TIRE_OUTER_R, TIRE_CLR); //Right Tire Rubber of Car
	lcd_fillCircle(TIRE_L_X+x, TIRE_L_Y+y, TIRE_INNER_R, HUB_CLR); //Left Rim of Car
	lcd_fillCircle(TIRE_R_X+x, TIRE_R_Y+y, TIRE_INNER_R, HUB_CLR); //Right Rim of Car

}

//----------------------------------------------------------------------------//
// Car Implementation - End
//----------------------------------------------------------------------------//

// Main display constants
#define BACKGROUND_CLR rgb565(0,60,90)
#define TITLE_CLR GREEN
#define STATUS_CLR WHITE
#define STR_BUF_LEN 12 // string buffer length
#define FONT_SIZE 2
#define FONT_W (LCD_CHAR_W*FONT_SIZE)
#define FONT_H (LCD_CHAR_H*FONT_SIZE)
#define STATUS_W (FONT_W*3)

#define WAIT 2000 // milliseconds
#define DELAY_EX3 20 // milliseconds

// Object position and movement
#define OBJ_X 100
#define OBJ_Y 100
#define OBJ_MOVE 3 // pixels


void app_main(void)
{
	ESP_LOGI(TAG, "Start up");
	lcd_init();
	lcd_fillScreen(BACKGROUND_CLR);
	lcd_setFontSize(FONT_SIZE);
	lcd_drawString(0, 0, "Hello World! (lcd)", TITLE_CLR);
	printf("Hello World! (terminal)\n");
	DELAY_MS(WAIT);

	// TODO: Exercise 1 - Draw car in one location.

	lcd_fillScreen(BACKGROUND_CLR);
	lcd_drawString(0, 0, "Exercise 1", TITLE_CLR);
	drawCar(OBJ_X, OBJ_Y);
	DELAY_MS(WAIT);


	// TODO: Exercise 2 - Draw moving car (Method 1), one pass across display.

	for (coord_t x = -CAR_W; x <= LCD_W; x += OBJ_MOVE)
	{	
		lcd_fillScreen(BACKGROUND_CLR);
		lcd_drawString(0, 0, "Exercise 2", TITLE_CLR);
		drawCar(x, OBJ_Y);
		char str[STR_BUF_LEN];
		sprintf(str, "%3ld", x);
		lcd_drawString(0, LCD_H-FONT_H, str, STATUS_CLR);
		DELAY_MS(DELAY_EX3);
	}


	// TODO: Exercise 3 - Draw moving car (Method 2), one pass across display.

	lcd_fillScreen(BACKGROUND_CLR);
	lcd_drawString(0, 0, "Exercise 3", TITLE_CLR);

	for (coord_t x = -CAR_W; x <= LCD_W; x += OBJ_MOVE)
	{
		lcd_fillRect(x-OBJ_MOVE, OBJ_Y, CAR_W, CAR_H, BACKGROUND_CLR);
		drawCar(x, OBJ_Y);
		lcd_fillRect2(0, LCD_H-FONT_H, STATUS_W, LCD_H, BACKGROUND_CLR);
		char str[STR_BUF_LEN];
		sprintf(str, "%3ld", x);
		lcd_drawString(0, LCD_H-FONT_H, str, STATUS_CLR);
		DELAY_MS(DELAY_EX3);
	}

	// TODO: Exercise 4 - Draw moving car (Method 3), one pass across display.

	lcd_frameEnable();

	for (coord_t x = -CAR_W; x <= LCD_W; x += OBJ_MOVE)
	{
		lcd_fillScreen(BACKGROUND_CLR);
		lcd_drawString(0, 0, "Exercise 4", TITLE_CLR);
		drawCar(x, OBJ_Y);
		char str[STR_BUF_LEN];
		sprintf(str, "%3ld", x);
		lcd_drawString(0, LCD_H-FONT_H, str, STATUS_CLR);
		lcd_writeFrame();
		DELAY_MS(DELAY_EX3);
	}

	lcd_frameDisable();

	// TODO: Exercise 5 - Draw an animated Pac-Man moving across the display.
	// Cycle through the pac[] array to animate, repeats indefinitely.

	lcd_frameEnable();
	uint16_t i = 0;
	const uint8_t pidx[] = {0, 1, 2, 1};

	for(;;){
		for(coord_t x = -PAC_W; x <= LCD_W; x += OBJ_MOVE){
			lcd_fillScreen(BACKGROUND_CLR);
			lcd_drawString(0, 0, "Exercise 5", TITLE_CLR);
			lcd_drawBitmap(x, OBJ_Y, pac[pidx[i++ % sizeof(pidx)]], PAC_W, PAC_H, YELLOW);
			char str[STR_BUF_LEN];
			sprintf(str, "%3ld", x);
			lcd_drawString(0, LCD_H-FONT_H, str, STATUS_CLR);
			lcd_writeFrame();
		}

	}

	lcd_frameDisable();

}
