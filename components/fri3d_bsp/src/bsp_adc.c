#include "esp_log.h"

#include "fri3d_bsp/bsp_adc.h"

extern const adc_driver_config_t bsp_adc_config;

static const char *TAG = "fri3d_bsp_adc";

esp_err_t bsp_adc_create(adc_driver_handle_t *ret_adc)
{
    esp_log_level_set(TAG, LOG_LOCAL_LEVEL);

    esp_err_t ret = ESP_OK;

    ESP_LOGD(TAG, "Initialize ADC Driver");
    adc_driver_handle_t adc = adc_driver_create(&bsp_adc_config);

    if (adc == NULL)
    {
        ret = ESP_FAIL;
    }

    if (ret == ESP_OK)
    {
        *ret_adc = adc;
    }

    return ret;
}
