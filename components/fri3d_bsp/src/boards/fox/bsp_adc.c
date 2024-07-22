#include "fri3d_bsp/bsp.h"

const adc_driver_config_t bsp_adc_config = {
    .unit = BSP_ADC_UNIT,
    .channel_count = BSP_ADC_CHANNEL_NUM,
    .channels =
        {
            {.gpio = BSP_JOYSTICK_AXIS_X_IO,
             .atten = BSP_JOYSTICK_AXIS_X_ATTEN,
             .bitwidth = ADC_BITWIDTH_DEFAULT,
             .calibration = ADC_DRIVER_CALIBRATION_CURVE},
            {.gpio = BSP_JOYSTICK_AXIS_Y_IO,
             .atten = BSP_JOYSTICK_AXIS_Y_ATTEN,
             .bitwidth = ADC_BITWIDTH_DEFAULT,
             .calibration = ADC_DRIVER_CALIBRATION_CURVE},
        },
};
