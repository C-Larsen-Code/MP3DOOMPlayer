#include <stdint.h>
#include "soc/gpio_periph.h"
#include "pin_test.h"

// Get the value of the PIN register for the specified pin argument.
uint32_t pin_test_get_pin_reg(pin_num_t pin)
{
	return REG_READ(GPIO_PIN0_REG + (pin * sizeof(uint32_t)));
}

// Get the value of the output function register for the specified pin argument.
uint32_t pin_test_get_func_out_sel_cfg_reg(pin_num_t pin)
{
	return REG_READ(GPIO_FUNC0_OUT_SEL_CFG_REG + (pin * sizeof(uint32_t)));
}

// Get the value of the IO MUX register for the specified pin argument.
uint32_t pin_test_get_io_mux_reg(pin_num_t pin)
{
	return REG_READ(GPIO_PIN_MUX_REG[pin]);
}

/*
For details see:
C:\esp5\esp-idf\components\soc\include\soc\gpio_periph.h & .c
C:\esp5\esp-idf\components\soc\esp32\include\soc\gpio_reg.h
C:\esp5\esp-idf\components\soc\esp32\include\soc\soc.h
C:\esp5\esp-idf\components\soc\esp32\include\soc\reg_base.h
*/
