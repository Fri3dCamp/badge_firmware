#pragma once

#include "esp_err.h"
#include "hal/gpio_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief custom button configuration
 *
 */
typedef struct
{
    int32_t gpio_num;                /**< num of gpio */
    uint8_t active_level;            /**< gpio level when press down */
    gpio_pull_mode_t gpio_pull_mode; /**< gpio pull mode */
#if CONFIG_GPIO_BUTTON_SUPPORT_POWER_SAVE
    bool enable_power_save; /**< enable power save mode */
#endif
} bsp_button_custom_config_t;

/**
 * @brief Initialize custom button
 *
 * @param config pointer of configuration struct
 *
 * @return
 *      - ESP_OK on success
 *      - ESP_ERR_INVALID_ARG   Arguments is NULL.
 */
esp_err_t bsp_button_custom_init(void *param);

/**
 * @brief Get current level on button gpio
 *
 * @param config pointer of configuration struct
 *
 * @return Level on gpio
 */
uint8_t bsp_button_custom_get_key_level(void *param);

#ifdef __cplusplus
}
#endif
