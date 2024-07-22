#pragma once

#include "driver/gpio.h"
#include "driver/spi_common.h"
#include "esp_lcd_types.h"

// Capabilities
#define BSP_CAPS_ADC           0
#define BSP_CAPS_DISPLAY       1
#define BSP_CAPS_TOUCH         0
#define BSP_CAPS_BUTTONS       1
#define BSP_CAPS_JOYSTICK      0
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
#define BSP_BUTTON_A_IO        (GPIO_NUM_13)
#define BSP_BUTTON_B_IO        (GPIO_NUM_12)
#define BSP_BUTTON_START_IO    (GPIO_NUM_32)
#define BSP_BUTTON_SELECT_IO   (GPIO_NUM_36)
#define BSP_BUTTON_P0_IO       (GPIO_NUM_27)
#define BSP_BUTTON_P1_IO       (GPIO_NUM_14)
#define BSP_BUTTON_UP_IO       (GPIO_NUM_39)
#define BSP_BUTTON_LEFT_IO     (GPIO_NUM_26)
#define BSP_BUTTON_DOWN_IO     (GPIO_NUM_15)
#define BSP_BUTTON_RIGHT_IO    (GPIO_NUM_0)
#define BSP_BUTTON_P3_IO       (GPIO_NUM_34)

/* Buttons */
typedef enum
{
    BSP_BUTTON_A = 0,
    BSP_BUTTON_B,
    BSP_BUTTON_START,
    BSP_BUTTON_SELECT,
    BSP_BUTTON_P0,
    BSP_BUTTON_P1,
    BSP_BUTTON_UP,
    BSP_BUTTON_LEFT,
    BSP_BUTTON_DOWN,
    BSP_BUTTON_RIGHT,
    BSP_BUTTON_P3,
    BSP_BUTTON_NUM
} bsp_button_t;

/* Button mappings */
#define BSP_KEY_ENTER      BSP_BUTTON_A
#define BSP_KEY_ESC        BSP_BUTTON_B
#define BSP_KEY_NEXT       BSP_BUTTON_P1
#define BSP_KEY_PREV       BSP_BUTTON_P0
#define BSP_KEY_HOME       BSP_BUTTON_SELECT
#define BSP_KEY_END        BSP_BUTTON_START
#define BSP_KEY_UP         BSP_BUTTON_UP
#define BSP_KEY_LEFT       BSP_BUTTON_LEFT
#define BSP_KEY_DOWN       BSP_BUTTON_DOWN
#define BSP_KEY_RIGHT      BSP_BUTTON_RIGHT

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
