#pragma once

#include "adc_driver/adc_driver.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void *joystick_axis_handle_t;

/**
 * @brief Joystick axis configuration
 *
 * The joystick axis has to be initialized with a minimum and maximum value but will automatically recalibrate the range
 * if it measures smaller or bigger values. It is important to initialize to a reasonable default as otherwise your
 * center might be off if the user only moved to one side
 */
typedef struct
{
    uint8_t adc_channel; /**< channel in the ADC driver that is mapped to this axis */
    uint16_t dead_val;   /**< ADC value relative to center still considered to be dead center */
    uint16_t min;        /**< minimum ADC value */
    uint16_t max;        /**< maximum ADC value */
} joystick_axis_config_t;

/**
 * @brief Create a joystick axis
 *
 * @param adc handle to an initialized ADC driver
 * @param config pointer of axis configuration
 *
 * @return A handle to the created axis, or NULL in case of error.
 */
joystick_axis_handle_t joystick_axis_create(adc_driver_handle_t adc, const joystick_axis_config_t *config);

/**
 * @brief Delete a joystick axis
 *
 * @param handle An axis handle to delete
 *
 * @return
 * - ESP_OK Success
 * - ESP_FAIL Failure
 */
esp_err_t joystick_axis_delete(joystick_axis_handle_t handle);

/**
 * @brief read the value from the joystick
 *
 * @param handle Axis to read the value from
 * @param[out] value Value of the axis, value between -100 and 100, untouched on error
 * @return
 * - ESP_OK Success
 * - ESP_FAIL Failure
 */
esp_err_t joystick_axis_read(joystick_axis_handle_t handle, int8_t *value);

#ifdef __cplusplus
}
#endif
