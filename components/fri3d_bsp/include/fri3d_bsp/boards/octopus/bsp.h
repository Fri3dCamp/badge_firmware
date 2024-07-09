#pragma once

#include "driver/gpio.h"

// Capabilities
#define BSP_CAPS_DISPLAY        0
#define BSP_CAPS_TOUCH          0
#define BSP_CAPS_BUTTONS        0
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
