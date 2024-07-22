#include "esp_log.h"

#include "fri3d_bsp/bsp.h"

static const char *TAG = "fri3d_bsp_joystick";

extern const joystick_axis_config_t bsp_joystick_config[BSP_JOYSTICK_AXIS_NUM];

esp_err_t bsp_joystick_create(adc_driver_handle_t adc, joystick_axis_handle_t axis[], int *axis_count, int axis_size)
{
    esp_log_level_set(TAG, LOG_LOCAL_LEVEL);

    esp_err_t ret = ESP_OK;
    if (axis_size < BSP_JOYSTICK_AXIS_NUM || axis == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }

    if (axis_count)
    {
        *axis_count = 0;
    }

    for (int i = 0; i < BSP_JOYSTICK_AXIS_NUM; i++)
    {
        axis[i] = joystick_axis_create(adc, &bsp_joystick_config[i]);

        if (axis[i] == NULL)
        {
            ret = ESP_FAIL;
            break;
        }

        if (axis_count)
        {
            ++*axis_count;
        }
    }

    return ret;
}
