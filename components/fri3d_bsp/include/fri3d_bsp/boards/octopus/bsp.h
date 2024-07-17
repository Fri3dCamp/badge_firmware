#pragma once

#include "driver/gpio.h"
#include "driver/spi_common.h"
#include "esp_lcd_types.h"

// Capabilities
#define BSP_CAPS_DISPLAY       1
#define BSP_CAPS_TOUCH         0
#define BSP_CAPS_BUTTONS       1
#define BSP_CAPS_BUZZER        1
#define BSP_CAPS_AUDIO         0
#define BSP_CAPS_AUDIO_SPEAKER 0
#define BSP_CAPS_AUDIO_MIC     0
#define BSP_CAPS_LED           1
#define BSP_CAPS_SDCARD        0
#define BSP_CAPS_IMU           0

// Pinout

// Leds
#define BSP_LED_RGB_GPIO       (GPIO_NUM_2)
#define BSP_LED_NUM            (5)

/* Buttons */
#define BSP_BUTTON_BOOT_IO     (GPIO_NUM_0)

/* Buttons */
typedef enum
{
    BSP_BUTTON_BOOT = 0,
    BSP_BUTTON_NUM
} bsp_button_t;

/* Buzzer */
#define BSP_BUZZER_GPIO    (GPIO_NUM_32)

/* SPI */
#define BSP_SPI_MOSI       (GPIO_NUM_23)
#define BSP_SPI_MISO       (GPIO_NUM_19)
#define BSP_SPI_SCK        (GPIO_NUM_18)

#define BSP_SPI_HOST       (SPI3_HOST)
#define BSP_SPI_BAUDRATE   (20 * 1000 * 1000)

/* Display */
#define BSP_LCD_RST        (GPIO_NUM_32)
#define BSP_LCD_DC         (GPIO_NUM_33)
#define BSP_LCD_CS         (GPIO_NUM_5)

#define BSP_LCD_WIDTH      (240)
#define BSP_LCD_HEIGHT     (240)
#define BSP_LCD_SWAP_XY    (false)
#define BSP_LCD_MIRROR_X   (false)
#define BSP_LCD_MIRROR_Y   (false)
#define BSP_LCD_INVERT     (true)
#define BSP_LCD_RGB_ORDER  (LCD_RGB_ELEMENT_ORDER_RGB)
#define BSP_LCD_BPP        (16)
#define BSP_LCD_CMD_BITS   (8)
#define BSP_LCD_PARAM_BITS (8)
