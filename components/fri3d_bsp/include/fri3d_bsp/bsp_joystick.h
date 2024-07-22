#pragma once

#include "joystick_axis/joystick_axis.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize the joystick
 *
 * When you clean up the resources created by this call, make sure you first delete the axis using joystick_axis_delete
 * and only then the ADC with adc_oneshot_del_unit
 *
 * @param[in] adc Handle to an initialized ADC driver the axis are using
 * @param[out] axis Array that will be initialized with the axis
 * @param[out] axis_count Will be set to the amount of axis that were initialized, can be NULL
 * @param[in] axis_size Size of axis array
 *
 * @return
 * - ESP_OK     Joystick is ready to use
 * - esp_err_t  Any other underlying return code
 */
esp_err_t bsp_joystick_create(adc_driver_handle_t adc, joystick_axis_handle_t axis[], int *axis_count, int axis_size);

#ifdef __cplusplus
}
#endif
