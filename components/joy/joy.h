#ifndef JOY_H_
#define JOY_H_

#include <stdint.h>

// Maximum joystick displacement in raw ADC values
// Dividing the values returned from joy_get_displacement()
// by this maximum, will give a proportion between 0 and +/- 1.
#define JOY_MAX_DISP 2048

// Initialize the joystick driver. Must be called before use.
// May be called multiple times. Return if already initialized.
// Return zero if successful, or non-zero otherwise.
int32_t joy_init(void);

// Free resources used by the joystick (ADC unit).
// Return zero if successful, or non-zero otherwise.
int32_t joy_deinit(void);

// Get the joystick displacement from center position.
// Displacement values range from 0 to +/- JOY_MAX_DISP.
// This function is not safe to call from an ISR context.
// Therefore, it must be called from a software task context.
// *dcx: pointer to displacement in x.
// *dcy: pointer to displacement in y.
void joy_get_displacement(int32_t *dcx, int32_t *dcy);

#endif // JOY_H_
