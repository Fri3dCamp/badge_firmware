#pragma once

#include "driver/gpio.h"

// Capabilities
#define BSP_CAPS_DISPLAY        0
#define BSP_CAPS_TOUCH          0
#define BSP_CAPS_BUTTONS        1
#define BSP_CAPS_BUZZER         1
#define BSP_CAPS_AUDIO          0
#define BSP_CAPS_AUDIO_SPEAKER  0
#define BSP_CAPS_AUDIO_MIC      0
#define BSP_CAPS_LED            1
#define BSP_CAPS_SDCARD         0
#define BSP_CAPS_IMU            0

// Pinout

// Leds
#define BSP_LED_RGB_GPIO        (GPIO_NUM_2)
#define BSP_LED_NUM             (5)

/* Buttons */
#define BSP_BUTTON_BOOT_IO      (GPIO_NUM_0)

/* Buttons */
typedef enum
{
    BSP_BUTTON_BOOT = 0,
    BSP_BUTTON_NUM
} bsp_button_t;

/* Buzzer */
#define BSP_BUZZER_GPIO         (GPIO_NUM_32)
