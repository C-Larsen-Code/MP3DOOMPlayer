#ifndef PIN_TEST_H_
#define PIN_TEST_H_

#include "pin.h"

// Get the value of the PIN register for the specified pin argument.
uint32_t pin_test_get_pin_reg(pin_num_t pin);

// Get the value of the output function register for the specified pin argument.
uint32_t pin_test_get_func_out_sel_cfg_reg(pin_num_t pin);

// Get the value of the IO MUX register for the specified pin argument.
uint32_t pin_test_get_io_mux_reg(pin_num_t pin);

#endif /* PIN_TEST_H_ */
