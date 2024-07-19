#include "fri3d_bsp/bsp.h"
#include "iot_button.h"

const bsp_button_config_t bsp_button_config[BSP_BUTTON_NUM] = {
    {.gpio_num = BSP_BUTTON_A_IO, .gpio_pull_mode = GPIO_PULLUP_ONLY},
    {.gpio_num = BSP_BUTTON_B_IO, .gpio_pull_mode = GPIO_PULLUP_ONLY},
    {.gpio_num = BSP_BUTTON_START_IO, .gpio_pull_mode = GPIO_PULLUP_ONLY},
    {.gpio_num = BSP_BUTTON_SELECT_IO, .gpio_pull_mode = GPIO_PULLUP_ONLY},
    {.gpio_num = BSP_BUTTON_P0_IO, .gpio_pull_mode = GPIO_PULLUP_ONLY},
    {.gpio_num = BSP_BUTTON_P1_IO, .gpio_pull_mode = GPIO_PULLUP_ONLY},
    {.gpio_num = BSP_BUTTON_UP_IO, .gpio_pull_mode = GPIO_PULLUP_ONLY},
    {.gpio_num = BSP_BUTTON_LEFT_IO, .gpio_pull_mode = GPIO_PULLUP_ONLY},
    {.gpio_num = BSP_BUTTON_DOWN_IO, .gpio_pull_mode = GPIO_PULLUP_ONLY},
    {.gpio_num = BSP_BUTTON_RIGHT_IO, .gpio_pull_mode = GPIO_FLOATING},
    {.gpio_num = BSP_BUTTON_P3_IO, .gpio_pull_mode = GPIO_PULLUP_ONLY}};
