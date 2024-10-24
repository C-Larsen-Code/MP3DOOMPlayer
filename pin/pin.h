#ifndef PIN_H_
#define PIN_H_

#include <stdbool.h>
#include <stdint.h>

typedef int8_t pin_num_t;

// Each of the functions below with a pin argument operate on a single pin.
// The exceptions are pin_get_in_reg() and pin_get_out_reg which return
// the state of all the I/O pins.

/***** I/O pin configuration *****/

// Reset the configuration of a pin to not be an input or an output.
// Pull-up is enabled so the pin does not float.
int32_t pin_reset(pin_num_t pin);

// Enable or disable a pull-up on the pin.
int32_t pin_pullup(pin_num_t pin, bool enable);

// Enable or disable a pull-down on the pin.
int32_t pin_pulldown(pin_num_t pin, bool enable);

// Enable or disable the pin as an input signal.
int32_t pin_input(pin_num_t pin, bool enable);

// Enable or disable the pin as an output signal.
int32_t pin_output(pin_num_t pin, bool enable);

// Enable or disable the pin as an open-drain signal.
int32_t pin_odrain(pin_num_t pin, bool enable);

/***** Set and get individual I/O pin signal levels *****/

// Sets the output signal level if the pin is configured as an output.
int32_t pin_set_level(pin_num_t pin, int32_t level);

// Gets the input signal level if the pin is configured as an input.
int32_t pin_get_level(pin_num_t pin);

/***** Get I/O register values, one pin per bit *****/

// Get the value of the input registers, one pin per bit.
// The two 32-bit input registers are concatenated into a uint64_t.
uint64_t pin_get_in_reg(void);

// Get the value of the output registers, one pin per bit.
// The two 32-bit output registers are concatenated into a uint64_t.
uint64_t pin_get_out_reg(void);

#endif // PIN_H_
