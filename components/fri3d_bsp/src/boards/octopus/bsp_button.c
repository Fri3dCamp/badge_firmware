#include "fri3d_bsp/bsp.h"
#include "iot_button.h"

const bsp_button_config_t bsp_button_config[BSP_BUTTON_NUM] = {
    {.gpio_num = BSP_BUTTON_BOOT_IO, .gpio_pull_mode = GPIO_FLOATING}};
