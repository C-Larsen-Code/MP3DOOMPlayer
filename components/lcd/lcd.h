// Modified from: https://github.com/nopnop2002/esp-idf-st7789

#ifndef LCD_H_
#define LCD_H_
/**
 * @file
 * @brief Functions to draw primitive shapes on the LCD display.
 * @details Many functions are similar to the Adafruit GFX Graphics Library.
 * See this [link](https://learn.adafruit.com/adafruit-gfx-graphics-library/coordinate-system-and-units)
 * for more detail about the coordinate system and graphics primitives.
 */

#include <stdint.h>
#include <stdbool.h>
#include "hw.h"

/** @name Use to create a custom color. */
#define rgb565(r, g, b) ((((r) & 0xF8) << 8) | (((g) & 0xFC) << 3) | (((b) & 0xF8) >> 3))

/** @name Standard colors. */
/** @{ */

#define RED     rgb565(255,   0,   0) // 0xf800
#define GREEN   rgb565(  0, 255,   0) // 0x07e0
#define BLUE    rgb565(  0,   0, 255) // 0x001f
#define BLACK   rgb565(  0,   0,   0) // 0x0000
#define WHITE   rgb565(255, 255, 255) // 0xffff
#define GRAY    rgb565(128, 128, 128) // 0x8410
#define YELLOW  rgb565(255, 255,   0) // 0xFFE0
#define CYAN    rgb565(  0, 156, 209) // 0x04FA
#define MAGENTA rgb565(128,   0, 128) // 0x8010
#define PURPLE MAGENTA

/** @} */

/** @name Character width and height in pixels. */
/** @{ */
#define LCD_CHAR_W 6
#define LCD_CHAR_H 8

/** @} */

/** @name Screen width and height in pixels. */
/** @{ */
#define LCD_W HW_LCD_W
#define LCD_H HW_LCD_H

/** @} */

/** @brief Coordinate type for x,y screen positions. */
/** @note Needs to be signed to handle off screen positions. */
typedef int32_t coord_t;

/** @brief Color type for drawing pixels and objects.
 *  @details 16-bit RGB Color (5-6-5). */
typedef uint16_t color_t;

/** @brief Angle type, +/- [0, 360] degrees. */
typedef int16_t angle_t;

/** @brief Direction type for font orientation. */
typedef enum {
	DIRECTION0,
	DIRECTION90,
	DIRECTION180,
	DIRECTION270
} direction_t;

/** @brief Scroll type for movement of screen image. */
typedef enum {
	SCROLL_RIGHT = 1,
	SCROLL_LEFT = 2,
	SCROLL_DOWN = 3,
	SCROLL_UP = 4,
} scroll_t;

/**
 * @brief Initialize the LCD module.
 */
void lcd_init(void);

/** @name Draw (outline) and fill primitives. */
/** @{ */

/**
 * @brief Fill the screen with one color.
 * @param color Color value.
 */
void lcd_fillScreen(color_t color);

/**
 * @brief Draw a pixel.
 * @param x     X coordinate.
 * @param y     Y coordinate.
 * @param color Color value.
 */
void lcd_drawPixel(coord_t x, coord_t y, color_t color);

/**
 * @brief Draw multiple pixels in a horizontal line.
 * @param x      X coordinate.
 * @param y      Y coordinate.
 * @param w      Width (extent) of line.
 * @param colors Array of color values, one for each pixel, length = w.
 */
void lcd_drawHPixels(coord_t x, coord_t y, coord_t w, const color_t *colors);

/**
 * @brief Draw horizontal line (faster than arbitrary line).
 * @param x     X coordinate.
 * @param y     Y coordinate.
 * @param w     Width (extent) of line.
 * @param color Color value.
 */
void lcd_drawHLine(coord_t x, coord_t y, coord_t w, color_t color);

/**
 * @brief Draw vertical line (faster than arbitrary line).
 * @param x     X coordinate.
 * @param y     Y coordinate.
 * @param h     Height (extent) of line.
 * @param color Color value.
 */
void lcd_drawVLine(coord_t x, coord_t y, coord_t h, color_t color);

/**
 * @brief Draw a line between 2 arbitrary points.
 * @param x0    X coordinate for Point 0.
 * @param y0    Y coordinate for Point 0.
 * @param x1    X coordinate for Point 1.
 * @param y1    Y coordinate for Point 1.
 * @param color Color value.
 */
void lcd_drawLine(coord_t x0, coord_t y0, coord_t x1, coord_t y1, color_t color);

/**
 * @brief Draw a rectangle outline with the specified color.
 * @param x     Top left corner X coordinate.
 * @param y     Top left corner Y coordinate.
 * @param w     Width in pixels.
 * @param h     Height in pixels.
 * @param color Color value.
 */
void lcd_drawRect(coord_t x, coord_t y, coord_t w, coord_t h, color_t color);

/**
 * @brief Draw a filled rectangle with the specified color.
 * @param x     Top left corner X coordinate.
 * @param y     Top left corner Y coordinate.
 * @param w     Width in pixels.
 * @param h     Height in pixels.
 * @param color Color value.
 */
void lcd_fillRect(coord_t x, coord_t y, coord_t w, coord_t h, color_t color);

/**
 * @brief Draw a triangle outline using 3 arbitrary points.
 * @param x0    X coordinate for Vertex 0.
 * @param y0    Y coordinate for Vertex 0.
 * @param x1    X coordinate for Vertex 1.
 * @param y1    Y coordinate for Vertex 1.
 * @param x2    X coordinate for Vertex 2.
 * @param y2    Y coordinate for Vertex 2.
 * @param color Color value.
 */
void lcd_drawTriangle(coord_t x0, coord_t y0, coord_t x1, coord_t y1, coord_t x2, coord_t y2, color_t color);

/**
 * @brief Draw a filled triangle using 3 arbitrary points.
 * @param x0    X coordinate for Vertex 0.
 * @param y0    Y coordinate for Vertex 0.
 * @param x1    X coordinate for Vertex 1.
 * @param y1    Y coordinate for Vertex 1.
 * @param x2    X coordinate for Vertex 2.
 * @param y2    Y coordinate for Vertex 2.
 * @param color Color value.
 */
void lcd_fillTriangle(coord_t x0, coord_t y0, coord_t x1, coord_t y1, coord_t x2, coord_t y2, color_t color);

/**
 * @brief Draw a circle outline.
 * @param xc    Center-point X coordinate.
 * @param yc    Center-point Y coordinate.
 * @param r     Radius of circle.
 * @param color Color value.
 */
void lcd_drawCircle(coord_t xc, coord_t yc, coord_t r, color_t color);

/**
 * @brief Draw a circle with filled color.
 * @param xc    Center-point X coordinate.
 * @param yc    Center-point Y coordinate.
 * @param r     Radius of circle.
 * @param color Color value.
 */
void lcd_fillCircle(coord_t xc, coord_t yc, coord_t r, color_t color);

/**
 * @brief Draw a rounded rectangle with no fill color.
 * @param x     Top left corner X coordinate.
 * @param y     Top left corner Y coordinate.
 * @param w     Width in pixels.
 * @param h     Height in pixels.
 * @param r     Radius of corner rounding.
 * @param color Color value.
 */
void lcd_drawRoundRect(coord_t x, coord_t y, coord_t w, coord_t h, coord_t r, color_t color);

/**
 * @brief Draw a rounded rectangle with fill color.
 * @param x     Top left corner X coordinate.
 * @param y     Top left corner Y coordinate.
 * @param w     Width in pixels.
 * @param h     Height in pixels.
 * @param r     Radius of corner rounding.
 * @param color Color value.
 */
void lcd_fillRoundRect(coord_t x, coord_t y, coord_t w, coord_t h, coord_t r, color_t color);

/**
 * @brief Draw arrow outline.
 * @param x0    Begin X coordinate.
 * @param y0    Begin Y coordinate.
 * @param x1    End (arrow point) X coordinate.
 * @param y1    End (arrow point) Y coordinate.
 * @param w     Half width of the arrow head in pixels. Height is 3*w.
 * @param color Color value.
 */
void lcd_drawArrow(coord_t x0, coord_t y0, coord_t x1, coord_t y1, coord_t w, color_t color);

/**
 * @brief Draw filled arrow.
 * @param x0    Begin X coordinate.
 * @param y0    Begin Y coordinate.
 * @param x1    End (arrow point) X coordinate.
 * @param y1    End (arrow point) Y coordinate.
 * @param w     Half width of the arrow head in pixels. Height is 3*w.
 * @param color Color value.
 */
void lcd_fillArrow(coord_t x0, coord_t y0, coord_t x1, coord_t y1, coord_t w, color_t color);

/**
 * @brief Draw a 1-bit image at the specified location using the specified
 *  color for set bits. Unset bits are transparent (no change to destination).
 * @param x      Top left corner X coordinate.
 * @param y      Top left corner Y coordinate.
 * @param bitmap Byte array with monochrome bitmap, one bit for each pixel.
 * @param w      Width of bitmap in pixels.
 * @param h      Height of bitmap in pixels.
 * @param color  Color value.
 * @note  If the bitmap width is not a multiple of 8, The last byte on
 *  each row of the bitmap array needs to be padded with zeros.
 */
void lcd_drawBitmap(coord_t x, coord_t y, const uint8_t *bitmap, coord_t w, coord_t h, color_t color);

/**
 * @brief Draw an image at the specified location.
 * @param x      Top left corner X coordinate.
 * @param y      Top left corner Y coordinate.
 * @param bitmap Array of color values, one for each pixel, length = w * h.
 * @param w      Width of bitmap in pixels.
 * @param h      Height of bitmap in pixels.
 */
void lcd_drawRGBBitmap(coord_t x, coord_t y, const color_t *bitmap, coord_t w, coord_t h);

/** @} */

/** @name Rectangle variants that specify two diagonal corners. */
/** @{ */

/**
 * @brief Draw a rectangle based on two diagonal corners.
 * @param x0    X coordinate for diagonal Corner 0.
 * @param y0    Y coordinate for diagonal Corner 0.
 * @param x1    X coordinate for diagonal Corner 1.
 * @param y1    Y coordinate for diagonal Corner 1.
 * @param color Color value.
 */
void lcd_drawRect2(coord_t x0, coord_t y0, coord_t x1, coord_t y1, color_t color);

/**
 * @brief Draw a filled rectangle based on two diagonal corners.
 * @param x0    X coordinate for diagonal Corner 0.
 * @param y0    Y coordinate for diagonal Corner 0.
 * @param x1    X coordinate for diagonal Corner 1.
 * @param y1    Y coordinate for diagonal Corner 1.
 * @param color Color value.
 */
void lcd_fillRect2(coord_t x0, coord_t y0, coord_t x1, coord_t y1, color_t color);

/**
 * @brief Draw a rounded rectangle based on two diagonal corners.
 * @param x0    X coordinate for diagonal Corner 0.
 * @param y0    Y coordinate for diagonal Corner 0.
 * @param x1    X coordinate for diagonal Corner 1.
 * @param y1    Y coordinate for diagonal Corner 1.
 * @param r     Radius of rounded corners.
 * @param color Color value.
 */
void lcd_drawRoundRect2(coord_t x0, coord_t y0, coord_t x1, coord_t y1, coord_t r, color_t color);

/**
 * @brief Draw a filled, rounded rectangle based on two diagonal corners.
 * @param x0    X coordinate for diagonal Corner 0.
 * @param y0    Y coordinate for diagonal Corner 0.
 * @param x1    X coordinate for diagonal Corner 1.
 * @param y1    Y coordinate for diagonal Corner 1.
 * @param r     Radius of rounded corners.
 * @param color Color value.
 */
void lcd_fillRoundRect2(coord_t x0, coord_t y0, coord_t x1, coord_t y1, coord_t r, color_t color);

/** @} */

/** @name Specify center, size, and rotation angle of primitive shape. */
/** @{ */

/**
 * @brief Draw a rectangle outline based on a center point.
 * @param xc    Center X coordinate.
 * @param yc    Center Y coordinate.
 * @param w     Width of rectangle.
 * @param h     Height of rectangle.
 * @param angle Angle of rotation (degrees).
 * @param color Color value.
 */
void lcd_drawRectC(coord_t xc, coord_t yc, coord_t w, coord_t h, angle_t angle, color_t color);

/**
 * @brief Draw a triangle outline based on a center point.
 * @param xc    Center X coordinate.
 * @param yc    Center Y coordinate.
 * @param w     Width of triangle.
 * @param h     Height of triangle.
 * @param angle Angle of rotation (degrees).
 * @param color Color value.
 */
void lcd_drawTriangleC(coord_t xc, coord_t yc, coord_t w, coord_t h, angle_t angle, color_t color);

/**
 * @brief Draw a regular polygon outline based on a center point.
 * @param xc    Center X coordinate.
 * @param yc    Center Y coordinate.
 * @param n     Number of sides.
 * @param r     Radius of polygon.
 * @param angle Angle of rotation (degrees).
 * @param color Color value.
 */
void lcd_drawRegularPolygonC(coord_t xc, coord_t yc, coord_t n, coord_t r, angle_t angle, color_t color);

/** @} */

/** @name Draw characters and strings. */
/** @{ */

/**
 * @brief Draw a single character.
 * @param x     Top left corner X coordinate.
 * @param y     Top left corner Y coordinate.
 * @param ascii ASCII encoded character.
 * @param color Color value.
 * @returns The coordinate (in X or Y) of a potential following character.
 */
coord_t lcd_drawChar(coord_t x, coord_t y, char ascii, color_t color);

/**
 * @brief Draw a string.
 * @param x     Top left corner X coordinate.
 * @param y     Top left corner Y coordinate.
 * @param ascii ASCII encoded string, zero terminated.
 * @param color Color value.
 * @returns The coordinate (in X or Y) of a potential following character.
 */
coord_t lcd_drawString(coord_t x, coord_t y, const char *ascii, color_t color);

/** @} */

/** @name Font parameters. */
/** @{ */

/**
 * @brief Set font direction.
 * @param dir Font direction.
 * @note Not implemented. Direction is always 0.
 */
void lcd_setFontDirection(direction_t dir);

/**
 * @brief Set font size.
 * @param size Font size scale factor (1 or greater).
 */
void lcd_setFontSize(uint8_t size);

/**
 * @brief Set font background color.
 * @param color Color value.
 */
void lcd_setFontBackground(color_t color);

/**
 * @brief No font background. Pixels surrounding characters are unchanged.
 */
void lcd_noFontBackground(void);

/** @} */

/** @name Display configuration. */
/** @{ */

/**
 * @brief Set SPI clock frequency for SPI bus used by display.
 * @param freq Frequency in Hz.
 */
void lcd_spiClockFreq(int32_t freq);

/**
 * @brief Display off.
 */
void lcd_displayOff(void);

/**
 * @brief Display on.
 */
void lcd_displayOn(void);

/**
 * @brief Backlight off.
 */
void lcd_backlightOff(void);

/**
 * @brief Backlight on.
 */
void lcd_backlightOn(void);

/**
 * @brief Display inversion off.
 */
void lcd_inversionOff(void);

/**
 * @brief Display inversion on.
 */
void lcd_inversionOn(void);

/** @} */

/** @name Frame management. */
/** @{ */

/**
 * @brief Allocate the frame buffer and enable its use.
 */
void lcd_frameEnable(void);

/**
 * @brief Deallocate the frame buffer and disable its use.
 */
void lcd_frameDisable(void);

/**
 * @brief Get the frame buffer.
 * @returns A pointer to the frame buffer or NULL if not allocated.
 */
color_t *lcd_getFrameBuffer(void);

/**
 * @brief Scroll image by one pixel between the start and end coordinates.
 * @param scroll Scroll direction.
 * @param start  Start of range in X or Y (depends on scroll direction).
 * @param end    End of range in X or Y (depends on scroll direction).
 * @note  Requires frame buffer to be enabled.
 */
void lcd_wrapAround(scroll_t scroll, coord_t start, coord_t end);

/**
 * @brief Write frame buffer to display. Requires frame buffer to be enabled.
 */
void lcd_writeFrame(void);

/** @} */

#endif // LCD_H_
