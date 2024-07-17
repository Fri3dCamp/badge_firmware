#pragma once

#include "button_custom.h"
#include "iot_button.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Button
 *
 * Special buttons on badges:
 *  - Reset:  Not programmable
 *  - Boot:   Controls boot mode during reset. Can be programmed after application starts
 */

/**
 * @brief Configuration structure to define buttons
 *
 */
typedef struct
{
    int32_t gpio_num;                /**< num of gpio */
    gpio_pull_mode_t gpio_pull_mode; /**< gpio pull mode, implemented for GPIO_FLOATING, GPIO_PULLUP_ONLY */
} bsp_button_config_t;

/**
 * @brief Initialize all buttons
 *
 * Returned button handlers must be used with espressif/button component API
 *
 * @param[out] btn_array      Output button array
 * @param[out] btn_cnt        Number of button handlers saved to btn_array, can be NULL
 * @param[in]  btn_array_size Size of output button array. Must be at least BSP_BUTTON_NUM
 * @return
 *     - ESP_OK               All buttons initialized
 *     - ESP_ERR_INVALID_ARG  btn_array is too small or NULL
 *     - ESP_FAIL             Underlaying iot_button_create failed
 */
esp_err_t bsp_iot_button_create(button_handle_t btn_array[], int *btn_cnt, int btn_array_size);

#ifdef __cplusplus
}
#endif
