#include <stdint.h>
#include "joy.h"
#include "esp_adc/adc_oneshot.h"

#define JOY_CENTER_CALC 10
#define JOY_MAX_DISP 2048

//Global variable to hold the handle for the ADC unit
adc_oneshot_unit_handle_t adc1_handle;

//Global variables to hold the center position of the joystick
int32_t x_joystick_c = 0;
int32_t y_joystick_c = 0;

// Initialize the joystick driver. Must be called before use.
int32_t joy_init(void){

    //Initialize the ADC unit for the joystick and disables the ULP mode
    adc_oneshot_unit_init_cfg_t init_config1 = {
        .unit_id = ADC_UNIT_1,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &adc1_handle));

    //Sets the bitwidth and attenuation of the ADC channels
    adc_oneshot_chan_cfg_t config = {
    .bitwidth = ADC_BITWIDTH_DEFAULT,
    .atten = ADC_ATTEN_DB_12,
    };
//Configure the ADC channels for the x and y values of the joystick. 6 being the x value and 7 being the y value.
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_6, &config));
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_7, &config));

    //Holds the sum of the x and y values of the joystick when calculating the center
    int32_t x_joystick_sum = 0;
    int32_t y_joystick_sum = 0;

    //Find the center position of the joystick by averaging a few reads from the ADC. 
    for (int i = 0; i < JOY_CENTER_CALC; i++){
        int_fast32_t x_joystick = 0;
        int_fast32_t y_joystick = 0;
        adc_oneshot_read(adc1_handle, ADC_CHANNEL_6, &x_joystick); //Read the x value of the joystick
        adc_oneshot_read(adc1_handle, ADC_CHANNEL_7, &y_joystick); //Read the y value of the joystick
        //Add the x and y values to the sum.
        x_joystick_sum += x_joystick;
        y_joystick_sum += y_joystick;
    }
    
    //Calculate the average of the x and y values and set it to the global variables
    x_joystick_c = x_joystick_sum / JOY_CENTER_CALC;
    y_joystick_c = y_joystick_sum / JOY_CENTER_CALC;

    return 0;
}


//Within the joy_deinit() function,delete ADC Unit 1 if its handle is not NULL. 
//The handle is declared globally and configured in joy_init().
int32_t joy_deinit(void){

    //Delete the ADC unit if it is not NULL
    //adc1_handle is declared globally and configured in joy_init()
    if(adc1_handle != NULL){
        adc_oneshot_del_unit(adc1_handle);
        return 0;
    } else {
        //Return 1 if the ADC unit is NULL
        return 1;
    }

}

//Get the joystick displacement from the center position and store it in the dcx and dcy pointers.
void joy_get_displacement(int32_t *dcx, int32_t *dcy){

    //Read the x and y values of the joystick., int_fast32_t is used to ensure the variable is at least 32 bits and preserved appropriately.
    int_fast32_t x_joy = 0;
    int_fast32_t y_joy = 0;

    //Read the x and y values of the joystick using the ADC
    adc_oneshot_read(adc1_handle, ADC_CHANNEL_6, &x_joy); //Read the x value of the joystick
    adc_oneshot_read(adc1_handle, ADC_CHANNEL_7, &y_joy); //Read the y value of the joystick

    //Calculate the displacement of the joystick from the center position
    *dcx = x_joy - x_joystick_c;
    *dcy = y_joy - y_joystick_c;

    //Check if the displacement is within the maximum displacement range for the x value
    if (*dcx > JOY_MAX_DISP){
        *dcx = JOY_MAX_DISP;
    } else if (*dcx < -JOY_MAX_DISP){
        *dcx = -JOY_MAX_DISP;
    }
    //Check if the displacement is within the maximum displacement range for the y value
    if (*dcy > JOY_MAX_DISP){
        *dcy = JOY_MAX_DISP;
    } else if (*dcy < -JOY_MAX_DISP){
        *dcy = -JOY_MAX_DISP;
    }
}