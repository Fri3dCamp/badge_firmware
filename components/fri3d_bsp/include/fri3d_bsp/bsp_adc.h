#pragma once

#include "adc_driver/adc_driver.h"
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Create new ADC driver
 *
 * @param ret_adc the created ADC driver on success, else NULL
 *
 * @return
 * - ESP_OK Success
 * - ESP_FAIL Failure
 */
esp_err_t bsp_adc_create(adc_driver_handle_t *ret_adc);

#ifdef __cplusplus
}
#endif
