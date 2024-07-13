#include "fri3d_bsp/bsp.h"
#include "iot_button.h"

const bsp_button_config_t bsp_button_config[BSP_BUTTON_NUM] = {
    {
        .gpio_num = BSP_BUTTON_BOOT_IO,
        .gpio_pull_mode = GPIO_FLOATING
    },
    {
        .gpio_num = BSP_BUTTON_MENU_IO,
        .gpio_pull_mode = GPIO_PULLUP_ONLY
    },
    {
        .gpio_num = BSP_BUTTON_A_IO,
        .gpio_pull_mode = GPIO_PULLUP_ONLY
    },
    {
        .gpio_num = BSP_BUTTON_B_IO,
        .gpio_pull_mode = GPIO_PULLUP_ONLY
    },
    {
        .gpio_num = BSP_BUTTON_X_IO,
        .gpio_pull_mode = GPIO_PULLUP_ONLY
    },
    {
        .gpio_num = BSP_BUTTON_Y_IO,
        .gpio_pull_mode = GPIO_PULLUP_ONLY
    }
};
