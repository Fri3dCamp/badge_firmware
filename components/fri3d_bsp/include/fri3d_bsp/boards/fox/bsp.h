#pragma once

#include "driver/gpio.h"

// Capabilities
#define BSP_CAPS_DISPLAY        0
#define BSP_CAPS_TOUCH          0
#define BSP_CAPS_BUTTONS        1
#define BSP_CAPS_AUDIO          0
#define BSP_CAPS_AUDIO_SPEAKER  0
#define BSP_CAPS_AUDIO_MIC      0
#define BSP_CAPS_LED            1
#define BSP_CAPS_SDCARD         0
#define BSP_CAPS_IMU            0

// Pinout

// Leds
#define BSP_LED_RGB_GPIO        (GPIO_NUM_12)
#define BSP_LED_NUM             (5)

/* Buttons */
#define BSP_BUTTON_BOOT_IO      (GPIO_NUM_0)
#define BSP_BUTTON_MENU_IO      (GPIO_NUM_45)
#define BSP_BUTTON_A_IO         (GPIO_NUM_39)
#define BSP_BUTTON_B_IO         (GPIO_NUM_40)
#define BSP_BUTTON_X_IO         (GPIO_NUM_38)
#define BSP_BUTTON_Y_IO         (GPIO_NUM_41)

/* Buttons */
typedef enum
{
    BSP_BUTTON_BOOT = 0,
    BSP_BUTTON_MENU,
    BSP_BUTTON_A,
    BSP_BUTTON_B,
    BSP_BUTTON_X,
    BSP_BUTTON_Y,
    BSP_BUTTON_NUM
} bsp_button_t;
