#pragma once

#include "esp_adc/adc_cali_scheme.h"
#include "esp_err.h"
#include "hal/adc_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void *adc_driver_handle_t;

typedef enum
{
#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
    ADC_DRIVER_CALIBRATION_CURVE,
#endif
#if ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
    ADC_DRIVER_CALIBRATION_LINE,
#endif
    ADC_DRIVER_CALIBRATION_NONE
} adc_driver_calibration_t;

typedef uint8_t adc_driver_channel_t;

typedef struct
{
    uint32_t gpio;
    adc_atten_t atten;
    adc_bitwidth_t bitwidth;
    adc_driver_calibration_t calibration;

} adc_driver_channel_config_t;

typedef struct
{
    adc_unit_t unit;
    adc_driver_channel_t channel_count;
    adc_driver_channel_config_t channels[];
} adc_driver_config_t;

/**
 * @brief Create an ADC driver
 *
 * @param config pointer of adc configuration
 *
 * @return A handle to the created ADC driver, NULL on error
 */
adc_driver_handle_t adc_driver_create(const adc_driver_config_t *config);

/**
 * @brief Delete the ADC driver
 *
 * @param handle handle to delete
 *
 * @return
 * - ESP_OK Success
 * - ESP_FAIL Failure
 */
esp_err_t adc_driver_delete(adc_driver_handle_t handle);

/**
 * @brief Read a value from the specified channel number
 *
 * @param channel number of the channel, corresponds to the channel in the config
 * @param value value of the channel, untouched on error
 *
 * @return
 * - ESP_OK Success
 * - ESP_FAIL Failure
 */
esp_err_t adc_driver_read_channel(adc_driver_handle_t handle, adc_driver_channel_t channel, int *value);

#ifdef __cplusplus
}
#endif
