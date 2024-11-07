#ifndef CURSOR_H_
#define CURSOR_H_

#include <stdint.h>

#include "lcd.h" // coord_t

// This component depends on the joystick component to provide input for
// calculating the position of the cursor. If the joystick displacement
// is left, the cursor is moved incrementally left each time step
// through a call to cursor_tick(). If the joystick is moved in another
// direction, the cursor is incrementally moved in that direction.
// The cursor position is clipped to the minimum and maximum screen
// coordinates as defined in the lcd component.


// Initialize the cursor. Must be called before use.
// The initial position is the center of the screen.
// per: Specify the period in milliseconds that the cursor
// position is updated with a call to cursor_tick().
// Return zero if successful, or non-zero otherwise.
int32_t cursor_init(uint32_t per);

// Update the cursor position based on the joystick displacement.
// This function is not safe to call from an ISR context.
// Therefore, it must be called from a software task context.
void cursor_tick(void);

// Set the sensitivity (speed) of the cursor relative to joystick movement.
// The sensitivity is specified as a factor in units of screen widths/sec
// at full joystick displacement. The default is 1.25 screen widths per second.
// sens: joystick movement sensitivity in screen widths/sec.
void cursor_set_sensitivity(float sens);

// Set the threshold of joystick displacement needed before moving the cursor.
// The threshold is specified as a factor (0 to 1) of maximum displacement.
// If this value is too low, the cursor will drift when the joystick is untouched.
// The default is a displacement factor of 0.075 from center position.
// thr: threshold factor (0 to 1)
void cursor_set_threshold(float thr);

// Get the cursor position in screen coordinates.
// Coordinate values range from 0 to lcd maximum width and height minus 1.
// *x: pointer to x coordinate.
// *y: pointer to y coordinate.
void cursor_get_pos(coord_t *x, coord_t *y);

// Set the cursor position in screen coordinates.
// Coordinate values range from 0 to lcd maximum width and height minus 1.
// x: x coordinate.
// y: y coordinate.
void cursor_set_pos(coord_t x, coord_t y);

#endif // CURSOR_H_
