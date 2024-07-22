#include "fri3d_bsp/bsp.h"

const joystick_axis_config_t bsp_joystick_config[BSP_JOYSTICK_AXIS_NUM] = {
    {.adc_channel = BSP_ADC_CHANNEL_JOYSTICK_AXIS_X, .dead_val = 125, .min = 20, .max = 3060},
    {.adc_channel = BSP_ADC_CHANNEL_JOYSTICK_AXIS_Y, .dead_val = 125, .min = 20, .max = 3060}};
